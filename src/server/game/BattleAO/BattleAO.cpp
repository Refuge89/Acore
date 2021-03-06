#include "Language.h"
#include "BattleAOMgr.h"
#include "BattlegroundMgr.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Map.h"
#include "MapManager.h"
#include "Group.h"
#include "WorldPacket.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "CreatureTextMgr.h"
#include "Chat.h"
#include "GroupMgr.h"

namespace Trinity
{
    class BattleAOChatBuilder
    {
        public:
            BattleAOChatBuilder(ChatMsg msgtype, uint32 textId, Player const* source, va_list* args = NULL)
                : _msgtype(msgtype), _textId(textId), _source(source), _args(args) { }

            void operator()(WorldPacket& data, LocaleConstant loc_idx)
            {
                char const* text = sObjectMgr->GetTrinityString(_textId, loc_idx);
                if (_args)
                {
                    // we need copy va_list before use or original va_list will corrupted
                    va_list ap;
                    va_copy(ap, *_args);

                    char str[2048];
                    vsnprintf(str, 2048, text, ap);
                    va_end(ap);

                    do_helper(data, &str[0]);
                }
                else
                    do_helper(data, text);
            }

        private:
            void do_helper(WorldPacket& data, char const* text)
            {
                ChatHandler::BuildChatPacket(data, _msgtype, LANG_UNIVERSAL, _source, _source, text);
            }

            ChatMsg _msgtype;
            uint32 _textId;
            Player const* _source;
            va_list* _args;
    };

    class BattleAO2ChatBuilder
    {
        public:
            BattleAO2ChatBuilder(ChatMsg msgtype, int32 textId, Player const* source, int32 arg1, int32 arg2)
                : _msgtype(msgtype), _textId(textId), _source(source), _arg1(arg1), _arg2(arg2) { }

            void operator()(WorldPacket& data, LocaleConstant loc_idx)
            {
                char const* text = sObjectMgr->GetTrinityString(_textId, loc_idx);
                char const* arg1str = _arg1 ? sObjectMgr->GetTrinityString(_arg1, loc_idx) : "";
                char const* arg2str = _arg2 ? sObjectMgr->GetTrinityString(_arg2, loc_idx) : "";

                char str[2048];
                snprintf(str, 2048, text, arg1str, arg2str);

                uint64 target_guid = _source  ? _source->GetGUID() : 0;

                data << uint8 (_msgtype);
                data << uint32(LANG_UNIVERSAL);
                data << uint64(target_guid);                // there 0 for BG messages
                data << uint32(0);                          // can be chat msg group or something
                data << uint64(target_guid);
                data << uint32(strlen(str) + 1);
                data << str;
                data << uint8 (_source ? _source->GetChatTag() : uint8(0));
            }

        private:
            ChatMsg _msgtype;
            int32 _textId;
            Player const* _source;
            int32 _arg1;
            int32 _arg2;
    };
}

template<class Do>
void BattleAO::BroadcastWorker(Do& _do)
{
    for (BattleAOPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = _GetPlayer(itr, "BroadcastWorker"))
            _do(player);
}

BattleAO::BattleAO() //ctor
{
    m_Map = sMapMgr->CreateBaseMap(BATTLEAO_MAP);
    m_PlayersCount[TEAM_ALLIANCE] = 0;
    m_PlayersCount[TEAM_HORDE]    = 0;
    m_PlayersCount[TEAM_NEUTRAL]  = 0;
    m_playerscantag = sWorld->getWorldState(BAO_WS_CANPLAYERSTAG) ? true : false;
    m_balancetag = false;
}
BattleAO::~BattleAO() { } //dtor

bool BattleAO::SetupBattleAO()
{
    AoObjects.resize(BAO_OBJECT_MAX);
    AoCreatures.resize(BAO_CREATURE_MAX);

    for (int i = 0; i < BAO_NODES_COUNT; ++i)
    {
        if (!AddObject(BAO_OBJECT_BANNER_NEUTRAL + 8*i,    BAO_OBJECTID_BANNER,        BAO_NodePositions[i][0], BAO_NodePositions[i][1], BAO_NodePositions[i][2], BAO_NodePositions[i][3], 0, 0, std::sin(BAO_NodePositions[i][3]/2), std::cos(BAO_NodePositions[i][3]/2), RESPAWN_ONE_DAY)
            || !AddObject(BAO_OBJECT_BANNER_CONT_A + 8*i,  BAO_OBJECTID_BANNER_CONT_A, BAO_NodePositions[i][0], BAO_NodePositions[i][1], BAO_NodePositions[i][2], BAO_NodePositions[i][3], 0, 0, std::sin(BAO_NodePositions[i][3]/2), std::cos(BAO_NodePositions[i][3]/2), RESPAWN_ONE_DAY)
            || !AddObject(BAO_OBJECT_BANNER_CONT_H + 8*i,  BAO_OBJECTID_BANNER_CONT_H, BAO_NodePositions[i][0], BAO_NodePositions[i][1], BAO_NodePositions[i][2], BAO_NodePositions[i][3], 0, 0, std::sin(BAO_NodePositions[i][3]/2), std::cos(BAO_NodePositions[i][3]/2), RESPAWN_ONE_DAY)
            || !AddObject(BAO_OBJECT_BANNER_ALLY + 8*i,    BAO_OBJECTID_BANNER_A,      BAO_NodePositions[i][0], BAO_NodePositions[i][1], BAO_NodePositions[i][2], BAO_NodePositions[i][3], 0, 0, std::sin(BAO_NodePositions[i][3]/2), std::cos(BAO_NodePositions[i][3]/2), RESPAWN_ONE_DAY)
            || !AddObject(BAO_OBJECT_BANNER_HORDE + 8*i,   BAO_OBJECTID_BANNER_H,      BAO_NodePositions[i][0], BAO_NodePositions[i][1], BAO_NodePositions[i][2], BAO_NodePositions[i][3], 0, 0, std::sin(BAO_NodePositions[i][3]/2), std::cos(BAO_NodePositions[i][3]/2), RESPAWN_ONE_DAY)
            || !AddObject(BAO_OBJECT_AURA_ALLY + 8*i,      BAO_OBJECTID_AURA_A,        BAO_NodePositions[i][0], BAO_NodePositions[i][1], BAO_NodePositions[i][2], BAO_NodePositions[i][3], 0, 0, std::sin(BAO_NodePositions[i][3]/2), std::cos(BAO_NodePositions[i][3]/2), RESPAWN_ONE_DAY)
            || !AddObject(BAO_OBJECT_AURA_HORDE + 8*i,     BAO_OBJECTID_AURA_H,        BAO_NodePositions[i][0], BAO_NodePositions[i][1], BAO_NodePositions[i][2], BAO_NodePositions[i][3], 0, 0, std::sin(BAO_NodePositions[i][3]/2), std::cos(BAO_NodePositions[i][3]/2), RESPAWN_ONE_DAY)
            || !AddObject(BAO_OBJECT_AURA_CONTESTED + 8*i, BAO_OBJECTID_AURA_C,        BAO_NodePositions[i][0], BAO_NodePositions[i][1], BAO_NodePositions[i][2], BAO_NodePositions[i][3], 0, 0, std::sin(BAO_NodePositions[i][3]/2), std::cos(BAO_NodePositions[i][3]/2), RESPAWN_ONE_DAY) )
        {
            TC_LOG_ERROR("sql.sql", "BAO: Failed to spawn some object BAO not created! %i", i);
            return false;
        }
    }

    for (uint8 i = 0; i < BAO_CREATURE_MAX; ++i)
        if (AoCreatures[i])
            DelCreature(i);

    for (uint8 node = 0; node < BAO_NODES_COUNT; ++node)
    {
        for (uint8 s = 0; s < 5; ++s)
            DelBanner(node, s);
        m_Nodes[node].timer = 0;
        m_Nodes[node].bannertimer.timer = 0;
        uint64 ws = sWorld->getWorldState(BAO_SAVENODEWORLDSTATE[node]);
        TeamId teamIndex = GetTeamIndexByTeamId(ws);
        if (ws == ALLIANCE || ws == HORDE)
        {
            m_Nodes[node].prev = teamIndex + BAO_NODE_TYPE_CONTESTED;
            m_Nodes[node].status = teamIndex + BAO_NODE_TYPE_OCCUPIED;
            CreateBanner(node, BAO_NODE_TYPE_OCCUPIED, teamIndex);
            PopulateNode(node, ws);
        }
        else
        {
            m_Nodes[node].prev = BAO_NODE_TYPE_NEUTRAL;
            m_Nodes[node].status = BAO_NODE_TYPE_NEUTRAL;
            CreateBanner(node, BAO_NODE_TYPE_NEUTRAL, TEAM_NEUTRAL);
        }
    }
    for (uint8 node = 0; node < BAO_NODES_COUNT; ++node)
        if (m_Nodes[node].status==BAO_NODE_STATUS_ALLY_OCCUPIED || m_Nodes[node].status==BAO_NODE_STATUS_HORDE_OCCUPIED)
            UpdateBannersFlag(node, m_Nodes[node].status==BAO_NODE_STATUS_ALLY_OCCUPIED?HORDE:ALLIANCE); // perte base autre team

    PopulateNode(BAO_SPIRIT_A2, ALLIANCE);
    PopulateNode(BAO_SPIRIT_H2, HORDE);

    return true;
}

bool BattleAO::Update(uint32 diff)
{
    for (int node = 0; node < BAO_NODES_COUNT; ++node)
    {
        if (m_Nodes[node].bannertimer.timer) // spawn banners after 2s (time for previous banner to burn)
        {
            if (m_Nodes[node].bannertimer.timer > diff)
                m_Nodes[node].bannertimer.timer -= diff;
            else
                CreateBanner(node, m_Nodes[node].bannertimer.type, m_Nodes[node].bannertimer.teamIndex);
        }

        if (m_Nodes[node].timer)
        {
            if (m_Nodes[node].timer > diff)
                m_Nodes[node].timer -= diff;
            else if (m_Nodes[node].status == m_Nodes[node].prev - 2)
                NeutralTaken(node);  // (cont V neutre)
            else                     // (cont V oqp)
                TeamTaken(node, (m_Nodes[node].status-1) ? HORDE : ALLIANCE);
        }
    }

    if (!m_OfflineQueue.empty()) // remove offline players from bg after 5 minutes
    {
        BattleAOPlayerMap::iterator itr = m_Players.find(*(m_OfflineQueue.begin()));
        if (itr != m_Players.end())
            if (itr->second.OfflineRemoveTime <= sWorld->GetGameTime())
            {
                TC_LOG_DEBUG("bao", "BAO kick offline player %s", itr->first.ToString().c_str());
                RemovePlayer(itr->first); // remove player from BG
                m_OfflineQueue.pop_front(); // remove from offline queue
            }
    }

    return true;
}

void BattleAO::AddPlayer(Player* player)
{
    TC_LOG_DEBUG("bao", "BAO AddPlayer %s", player->GetGUID().ToString().c_str());

    if (player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK))
        player->ToggleAFK();

    ObjectGuid guid = player->GetGUID();
    uint32 team = player->IsDeserter() ? TEAM_NEUTRAL : player->GetTeam(true);

    BattleAOPlayer bp;
    bp.OfflineRemoveTime = 0;
    bp.Team = team;

    m_Players[guid] = bp;
    if (m_Players.size() > BAO_MIN_PLAYERS_FOR_BALANCE)
        m_balancetag = true;

    WorldPacket data;
    sBattlegroundMgr->BuildPlayerJoinedBattlegroundPacket(&data, player);
    SendPacketToTeam(team, &data, player, false);
    sBattleAOMgr->BuildBattleAOStatusPacket(&data, player->GetBattlegroundQueueIndex(BATTLEGROUND_QUEUE_AO), STATUS_IN_PROGRESS, 0, 0);
    player->GetSession()->SendPacket(&data);
    sBattleAOMgr->BuildPvpLogDataPacket(&data);
    player->GetSession()->SendPacket(&data);
    SendAllNodeUpdate(player);

    player->RemoveAurasByType(SPELL_AURA_MOUNTED);

    if (!player->IsDeserter())
        AddOrSetPlayerToCorrectBAOGroup(player);
    RemoveAurasFromPlayer(player);
}

void BattleAO::RemovePlayer(ObjectGuid guid, bool teleport)
{
    // option quitter, 5 min de d�co, tp autre map :
    TC_LOG_DEBUG("bao", "BAO RemovePlayer %s", guid.ToString().c_str());
    uint32 team = 0;
    bool participant = false;
    BattleAOPlayerMap::iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
    {
        BattleAOPlayer bap = itr->second;
        team = bap.Team;
        UpdatePlayersCount(team, true);
        m_Players.erase(itr);
        participant = true;
    }
    if (m_Players.size() < BAO_MIN_PLAYERS_FOR_BALANCE)
        m_balancetag = false;

    Player* player = ObjectAccessor::FindPlayer(guid);
    if (player) // should remove spirit of redemption
    {
        if (player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            player->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
        if (!player->IsAlive())                              // resurrect on exit
        {
            player->ResurrectPlayer(1.0f);
            player->SpawnCorpseBones();
        }
    }

    if (participant) // if the player was a match participant, remove auras, calc rating, update queue
    {
        if (player)
        {
            player->ClearAfkReports();
            WorldPacket data;
            sBattleAOMgr->BuildBattleAOStatusPacket(&data, player->GetBattlegroundQueueIndex(BATTLEGROUND_QUEUE_AO), STATUS_NONE, 0, 0);
            player->GetSession()->SendPacket(&data);
            player->RemoveBattlegroundQueueId(BATTLEGROUND_QUEUE_AO);
        }
        sBattleAOMgr->ScheduleQueueUpdate();

        if (Group* group = player->GetGroup())
            if (group->isBAOGroup())
                group->RemoveMember(player->GetGUID(), GROUP_REMOVEMETHOD_LEAVE);
    }

    if (player && teleport)
    {
        player->SetBattlegroundId(0, BATTLEGROUND_TYPE_NONE);
        player->SetBGTeam(0);
        player->TeleportToBAOEntryPoint();
    }
}

// Called when a player enters the zone
void BattleAO::HandlePlayerEnterZone(Player* player, uint32 /*zoneid*/)
{
    if (player->GetBattlegroundQueueIndex(BATTLEGROUND_QUEUE_AO) == PLAYER_MAX_BATTLEGROUND_QUEUES)
    {
        player->AddBattlegroundQueueId(BATTLEGROUND_QUEUE_AO);
        UpdatePlayersCount(player->IsDeserter() ? TEAM_NEUTRAL : player->GetTeam(true), false);
        sBattleAOMgr->ScheduleQueueUpdate();
    }
    player->SetInviteForBattlegroundQueueType(BATTLEGROUND_QUEUE_AO, true);
    AddPlayer(player);
}

// Called when a player leave the zone
void BattleAO::HandlePlayerLeaveZone(Player* player, uint32 /*zoneid*/)
{
    RemovePlayer(player->GetGUID(), false);
}

void BattleAO::EventPlayerLoggedIn(Player* player)
{
    ObjectGuid guid = player->GetGUID();
    for (std::deque<ObjectGuid>::iterator itr = m_OfflineQueue.begin(); itr != m_OfflineQueue.end(); ++itr)
    {
        if (*itr == guid)
        {
            m_OfflineQueue.erase(itr);
            break;
        }
    }
    m_Players[guid].OfflineRemoveTime = 0;
    AddOrSetPlayerToCorrectBAOGroup(player);
    WorldPacket data;
    sBattleAOMgr->BuildBattleAOStatusPacket(&data, player->GetBattlegroundQueueIndex(BATTLEGROUND_QUEUE_AO), STATUS_IN_PROGRESS, 0, 0); // end time, start time
    player->GetSession()->SendPacket(&data);
    sBattleAOMgr->BuildPvpLogDataPacket(&data);
    player->GetSession()->SendPacket(&data);
    SendAllNodeUpdate(player);
}

void BattleAO::EventPlayerLoggedOut(Player* player)
{
    m_OfflineQueue.push_back(player->GetGUID());
    m_Players[player->GetGUID()].OfflineRemoveTime = sWorld->GetGameTime() + MAX_OFFLINE_TIME;
}

void BattleAO::SendPacketToAll(WorldPacket* packet)
{
    for (BattleAOPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = _GetPlayer(itr, "SendPacketToAll"))
            player->GetSession()->SendPacket(packet);
}

void BattleAO::SendPacketToTeam(uint32 TeamID, WorldPacket* packet, Player* sender, bool self)
{
    for (BattleAOPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = _GetPlayerForTeam(TeamID, itr, "SendPacketToTeam"))
            if (self || sender != player)
            {
                WorldSession* session = player->GetSession();
                TC_LOG_DEBUG("bao", "%s %s - SendPacketToTeam %u, Player: %s", GetOpcodeNameForLogging(packet->GetOpcode()).c_str(),
                    session->GetPlayerInfo().c_str(), TeamID, sender ? sender->GetName().c_str() : "null");
                session->SendPacket(packet);
            }
}

void BattleAO::PlaySoundToAll(uint32 SoundID)
{
    WorldPacket data;
    sBattlegroundMgr->BuildPlaySoundPacket(&data, SoundID);
    SendPacketToAll(&data);
}

void BattleAO::SendMessage2ToAll(int32 entry, ChatMsg type, Player const* source, int32 arg1, int32 arg2)
{
    Trinity::BattleAO2ChatBuilder bg_builder(type, entry, source, arg1, arg2);
    Trinity::LocalizedPacketDo<Trinity::BattleAO2ChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

void BattleAO::RemoveAurasFromPlayer(Player* player)
{
    //player->RemoveAurasDueToSpell();
}

void BattleAO::TeamCastSpell(TeamId team, int32 spellId)
{
    if (spellId > 0)
    {
        for (BattleAOPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            if (Player* player = sObjectAccessor->FindPlayer(itr->first))
                if (player->GetTeamId() == team)
                    player->CastSpell(player, uint32(spellId), true);
    }
    else
        for (BattleAOPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            if (Player* player = sObjectAccessor->FindPlayer(itr->first))
                if (player->GetTeamId() == team)
                    player->RemoveAuraFromStack(uint32(-spellId));
}

void BattleAO::SendUpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data;
    sBattlegroundMgr->BuildUpdateWorldStatePacket(&data, Field, Value);
    SendPacketToAll(&data);
}

void BattleAO::SendAllNodeUpdate(Player* player)
{
    uint8 ally = 0, horde = 0; // How many bases each team owns
    for (uint8 i = 0; i < BAO_NODES_COUNT; ++i)
        if (m_Nodes[i].status == BAO_NODE_STATUS_ALLY_OCCUPIED)
            ++ally;
        else if (m_Nodes[i].status == BAO_NODE_STATUS_HORDE_OCCUPIED)
            ++horde;

    const int8 plusArray[] = {-1, 2, 3, 0, 1}; // Send node owner state update to refresh map icons on client

    WorldPacket data;

    for (uint8 node = 0; node < BAO_NODES_COUNT; ++node)
    {
        if (m_Nodes[node].prev)
            sBattlegroundMgr->BuildUpdateWorldStatePacket(&data, BAO_OP_NODESTATES[node] + plusArray[m_Nodes[node].prev], 0);
        else
            sBattlegroundMgr->BuildUpdateWorldStatePacket(&data, BAO_OP_NODEICONS[node], 0);
        player->GetSession()->SendPacket(&data);

        sBattlegroundMgr->BuildUpdateWorldStatePacket(&data, BAO_OP_NODESTATES[node] + plusArray[m_Nodes[node].status], 1);
        player->GetSession()->SendPacket(&data);
    }

    sBattlegroundMgr->BuildUpdateWorldStatePacket(&data, BAO_WS_OCCUPIED_BASES_ALLY, ally);
    player->GetSession()->SendPacket(&data);
    sBattlegroundMgr->BuildUpdateWorldStatePacket(&data, BAO_WS_OCCUPIED_BASES_HORDE, horde);
    player->GetSession()->SendPacket(&data);
}

// ****************************************************
// ******************* Group System *******************
// ****************************************************
Group* BattleAO::GetFreeBAORaid(TeamId TeamId)
{
    for (GuidSet::const_iterator itr = m_Groups[TeamId].begin(); itr != m_Groups[TeamId].end(); ++itr)
        if (Group* group = sGroupMgr->GetGroupByGUID(*itr))
            if (!group->IsFull())
                return group;

    return NULL;
}

Group* BattleAO::GetGroupPlayer(ObjectGuid guid, TeamId TeamId)
{
    for (GuidSet::const_iterator itr = m_Groups[TeamId].begin(); itr != m_Groups[TeamId].end(); ++itr)
        if (Group* group = sGroupMgr->GetGroupByGUID(*itr))
            if (group->IsMember(guid))
                return group;
    return NULL;
}

bool BattleAO::AddOrSetPlayerToCorrectBAOGroup(Player* player)
{
    if (!player->IsInWorld())
        return false;

    if (Group* group = player->GetGroup())
    {
        if (group->isBAOGroup())
        {
            group->SendUpdateToPlayer(player->GetGUID());
            return false;
         }
        group->RemoveMember(player->GetGUID());
    }

    Group* group = GetFreeBAORaid(player->GetTeamId());
    if (!group)
    {
        group = new Group;
        group->SetBattleAOGroup(this);
        group->Create(player);
        sGroupMgr->AddGroup(group);
        m_Groups[player->GetTeamId()].insert(group->GetGUID());
    }
    else if (group->IsMember(player->GetGUID()))
    {
        uint8 subgroup = group->GetMemberGroup(player->GetGUID());
        player->SetBattlegroundOrBattlefieldRaid(group, subgroup);
    }
    else
        group->AddMember(player);

    return true;
}

WorldSafeLocsEntry const* BattleAO::GetClosestGraveYard(Player* player)
{
    //player->SetRezTime(20000);
    TeamId teamIndex = GetTeamIndexByTeamId(player->GetTeam(true));
    std::vector<uint8> graveyards;
    if (player->IsDeserter())
        graveyards.push_back(6); // neutral gy
    else
    {
        for (uint8 i = BAO_NODE_SPIRIT_FIRST; i < BAO_NODE_SPIRIT_MAX; ++i)
            if (m_Nodes[i].status == teamIndex + BAO_NODE_TYPE_OCCUPIED)
                graveyards.push_back(i-BAO_NODE_SPIRIT_FIRST);
        if (m_Nodes[BAO_NODE_NORD].status == teamIndex + BAO_NODE_TYPE_OCCUPIED && m_Nodes[BAO_NODE_RUINES].status == teamIndex + BAO_NODE_TYPE_OCCUPIED && m_Nodes[BAO_NODE_SUD].status == teamIndex + BAO_NODE_TYPE_OCCUPIED)
            graveyards.push_back(5); // syltania gy
    }

    WorldSafeLocsEntry const* good_entry = NULL;
    if (!graveyards.empty())
    {
        float plr_x = player->GetPositionX();
        float plr_y = player->GetPositionY();
        float mindist = 999999.0f;
        for (uint8 i = 0; i < graveyards.size(); ++i)
        {
            WorldSafeLocsEntry const*entry = sDBCMgr->GetWorldSafeLocsEntry(BAO_GraveyardIds[graveyards[i]]);
            if (!entry)
                continue;
            float dist = (entry->x - plr_x)*(entry->x - plr_x)+(entry->y - plr_y)*(entry->y - plr_y);
            if (mindist > dist)
            {
                mindist = dist;
                good_entry = entry;
            }
        }
        graveyards.clear();
    }
    if (!good_entry)
    {
        TC_LOG_ERROR("misc", "BAO aucun cimeti�re trouv�");
        good_entry = sDBCMgr->GetWorldSafeLocsEntry(BAO_GraveyardIds[BAO_NODE_A2+teamIndex]); // base gy
    }
    return good_entry;
}

void BattleAO::HandleKill(Player* killer, Player* victim)
{
    if (killer)
    {
        if (killer == victim)
            return;

        for (BattleAOPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player* creditedPlayer = ObjectAccessor::FindPlayer(itr->first);
            if (!creditedPlayer || creditedPlayer == killer)
                continue;
        }
    }
    if (victim)
        victim->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
}

GameObject* BattleAO::GetAOObject(uint32 type)
{
    GameObject* obj = m_Map->GetGameObject(AoObjects[type]);
    if (obj)
        return obj;
    TC_LOG_ERROR("bao", "BAO::GetAOObject: gameobject type:%u, guid %s not found", type, AoObjects[type].ToString().c_str());
    return NULL;
}

Creature* BattleAO::GetAOCreature(uint32 type)
{
    Creature* creature = m_Map->GetCreature(AoCreatures[type]);
    if (creature)
        return creature;
    TC_LOG_ERROR("bao", "BAO::GetAOCreature: creature type:%u, guid %s not found", type, AoCreatures[type].ToString().c_str());
    return NULL;
}

void BattleAO::SpawnAOObject(uint32 type, uint32 respawntime)
{
    if (m_Map)
        if (GameObject* obj = m_Map->GetGameObject(AoObjects[type]))
        {
            if (respawntime)
                obj->SetLootState(GO_JUST_DEACTIVATED);
            else
                if (obj->getLootState() == GO_JUST_DEACTIVATED)
                    obj->SetLootState(GO_READY);
            obj->SetRespawnTime(respawntime);
            obj->SetSpawnedByDefault(false);
            m_Map->AddToMap(obj);
        }
}

bool BattleAO::AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 /*respawnTime*/)
{
    ASSERT(type < AoObjects.size());
    Map* map = m_Map;
    if (!map)
        return false;
    GameObject* go = new GameObject;
    if (!go->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_GAMEOBJECT), entry, m_Map,
        PHASEMASK_NORMAL, x, y, z, o, rotation0, rotation1, rotation2, rotation3, 100, GO_STATE_READY))
    {
        TC_LOG_ERROR("sql.sql", "BAO::AddObject: cannot create gameobject entry:%u", entry);
        TC_LOG_ERROR("bao", "BAO::AddObject: cannot create gameobject entry:%u", entry);
        delete go;
        return false;
    }
    if (!map->AddToMap(go))
    {
        delete go;
        return false;
    }
    AoObjects[type] = go->GetGUID();
    return true;
}

Creature* BattleAO::AddCreature(uint32 entry, uint32 type, uint32 teamval, float x, float y, float z, float o, uint32 respawntime)
{
    ASSERT(type < AoCreatures.size());
    Map* map = m_Map;
    if (!map)
        return NULL;
    Creature* creature = new Creature;
    if (!creature->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_UNIT), map, PHASEMASK_NORMAL, entry, x, y, z, o))
    {
        TC_LOG_ERROR("bao", "BattleAO::AddCreature: cannot create creature entry %u", entry);
        delete creature;
        return NULL;
    }
    creature->SetHomePosition(x, y, z, o);
    CreatureTemplate const* cinfo = sObjectMgr->GetCreatureTemplate(entry);
    if (!cinfo)
    {
        TC_LOG_ERROR("bao", "BattleAO::AddCreature: no template for creature entry %u", entry);
        delete creature;
        return NULL;
    }
    if (!map->AddToMap(creature))
    {
        delete creature;
        return NULL;
    }
    AoCreatures[type] = creature->GetGUID();
    if (respawntime)
        creature->SetRespawnDelay(respawntime);

    return  creature;
}

bool BattleAO::DelCreature(uint32 type)
{
    if (!AoCreatures[type])
        return true;
    if (Creature* creature = m_Map->GetCreature(AoCreatures[type]))
    {
        creature->AddObjectToRemoveList();
        AoCreatures[type] = ObjectGuid::Empty;
        return true;
    }
    TC_LOG_ERROR("bao", "BAO::DelCreature: creature type:%u, GUID:%u not found", type, AoCreatures[type].ToString().c_str());
    AoCreatures[type] = ObjectGuid::Empty;
    return false;
}

bool BattleAO::DelObject(uint32 type)
{
    if (!AoObjects[type])
        return true;
    if (GameObject* obj = m_Map->GetGameObject(AoObjects[type]))
    {
        obj->SetRespawnTime(0);
        obj->Delete();
        AoObjects[type] = ObjectGuid::Empty;
        return true;
    }
    TC_LOG_ERROR("bao", "BAO::DelObject: gameobject type:%u GUID:%s not found", type, AoObjects[type].ToString().c_str());
    AoObjects[type] = ObjectGuid::Empty;
    return false;
}

bool BattleAO::AddSpiritGuide(uint32 type, float x, float y, float z, float o, uint32 team)
{
    uint32 entry = (team == ALLIANCE) ? BAO_CREATURE_ENTRY_A_SPIRITGUIDE : BAO_CREATURE_ENTRY_H_SPIRITGUIDE;
    if (Creature* creature = AddCreature(entry, type, team, x, y, z, o))
    {
        creature->setDeathState(DEAD);
        creature->SetUInt64Value(UNIT_FIELD_CHANNEL_OBJECT, creature->GetGUID());
        creature->SetUInt32Value(UNIT_CHANNEL_SPELL, SPELL_SPIRIT_HEAL_CHANNEL);
        creature->SetFloatValue(UNIT_MOD_CAST_SPEED, 1.0f);
        return true;
    }
    TC_LOG_ERROR("bao", "BAO::AddSpiritGuide: cannot create spirit guide type:%u, entry:%u", type, entry);
    return false;
}

void BattleAO::CreateBanner(uint8 node, uint8 type, uint8 teamIndex, bool delay)
{
    if (delay) // spawn in 2s
    {
        m_Nodes[node].bannertimer.timer = 2000;
        m_Nodes[node].bannertimer.type = type;
        m_Nodes[node].bannertimer.teamIndex = teamIndex;
        return;
    }

    m_Nodes[node].bannertimer.timer = 0;
    uint8 obj = node*8 + type + (teamIndex != TEAM_NEUTRAL) * teamIndex;
    m_Nodes[node].banner = obj; // type de la banni�re active

    if (GameObject* ob = GetAOObject(obj))
    {
        ob->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_CANTNEUTRAL|GO_FLAG_CANTA2|GO_FLAG_CANTH2);
        if (teamIndex==TEAM_NEUTRAL)
            ob->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_CANTNEUTRAL);
        else if (teamIndex)
            ob->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_CANTH2);
        else
            ob->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_CANTA2);
    }
    SpawnAOObject(obj, RESPAWN_IMMEDIATELY);

    obj = node * 8 + ((type == BAO_NODE_TYPE_OCCUPIED) ? (5 + teamIndex) : 7); // auras
    SpawnAOObject(obj, RESPAWN_IMMEDIATELY);
}

void BattleAO::DelBanner(uint8 node, uint8 status)
{
    uint8 obj = node*8 + status;
    SpawnAOObject(obj, RESPAWN_ONE_DAY);

    obj = node * 8 + (((2 < status) && (status < 5)) ? status + 2 : 7); // auras (oqp banner + 2 = oqp aura)
    SpawnAOObject(obj, RESPAWN_ONE_DAY);
}

void BattleAO::FillInitialWorldStates(WorldPacket& data)
{
    uint8 ally = 0, horde = 0; // How many bases each team owns
    for (uint8 node = 0; node < BAO_NODES_COUNT; ++node)
        if (m_Nodes[node].status == BAO_NODE_STATUS_ALLY_OCCUPIED)
            ++ally;
        else if (m_Nodes[node].status == BAO_NODE_STATUS_HORDE_OCCUPIED)
            ++horde;

    const int8 plusArray[] = {-1, 2, 3, 0, 1};


    for (uint8 node = 0; node < BAO_NODES_COUNT; ++node)
    {
        if (m_Nodes[node].prev)
            data << uint32(BAO_OP_NODESTATES[node] + plusArray[m_Nodes[node].prev]) << 0;
        else
            data << uint32(BAO_OP_NODEICONS[node]) << 0;
        data << uint32(BAO_OP_NODESTATES[node] + plusArray[m_Nodes[node].status]) << 1;
    }

    data << uint32(BAO_WS_OCCUPIED_BASES_ALLY)  << uint32(ally);
    data << uint32(BAO_WS_OCCUPIED_BASES_HORDE) << uint32(horde);

    // ressources
    data << uint32(BAO_WS_RESOURCES_ALLY)     << uint32(sWorld->getWorldState(BAO_WS_RESOURCES_ALLY));
    data << uint32(BAO_WS_RESOURCES_HORDE)    << uint32(sWorld->getWorldState(BAO_WS_RESOURCES_HORDE));
}

void BattleAO::FillInitialWorldStatesForKZ(WorldPacket& data)
{
    data << uint32(BAO_WS_RESOURCES_ALLY)     << uint32(sWorld->getWorldState(BAO_WS_RESOURCES_ALLY));
    data << uint32(BAO_WS_RESOURCES_HORDE)    << uint32(sWorld->getWorldState(BAO_WS_RESOURCES_HORDE));
}

void BattleAO::SendNodeUpdate(uint8 node)
{
    // Send node owner state update to refresh map icons on client
    const int8 plusArray[] = {-1, 2, 3, 0, 1};

    if (m_Nodes[node].prev)
        SendUpdateWorldState(BAO_OP_NODESTATES[node] + plusArray[m_Nodes[node].prev], 0);
    else
        SendUpdateWorldState(BAO_OP_NODEICONS[node], 0);

    SendUpdateWorldState(BAO_OP_NODESTATES[node] + plusArray[m_Nodes[node].status], 1);

    // How many bases each team owns
    uint8 ally = 0, horde = 0;
    for (uint8 i = 0; i < BAO_NODES_COUNT; ++i)
        if (m_Nodes[i].status == BAO_NODE_STATUS_ALLY_OCCUPIED)
            ++ally;
        else if (m_Nodes[i].status == BAO_NODE_STATUS_HORDE_OCCUPIED)
            ++horde;

    sWorld->setWorldState(BAO_WS_OCCUPIED_BASES_ALLY, ally);
    sWorld->setWorldState(BAO_WS_OCCUPIED_BASES_HORDE, horde);
    SendUpdateWorldState(BAO_WS_OCCUPIED_BASES_ALLY, ally);
    SendUpdateWorldState(BAO_WS_OCCUPIED_BASES_HORDE, horde);
    SendUpdateWorldState(BAO_WS_RESOURCES_ALLY, int32(sWorld->getWorldState(BAO_WS_RESOURCES_ALLY)));
    SendUpdateWorldState(BAO_WS_RESOURCES_HORDE,int32(sWorld->getWorldState(BAO_WS_RESOURCES_HORDE)));
}

void BattleAO::PopulateNode(uint8 node, uint32 team) // spawning creatures
{
    if (node < BAO_NODES_COUNT)
        sWorld->setWorldState(BAO_SAVENODEWORLDSTATE[node], uint64(team));

    if (node >= BAO_NODE_SPIRIT_FIRST && node < BAO_NODE_A2 && team != TEAM_OTHER)
        if (!AddSpiritGuide(node-BAO_NODE_SPIRIT_FIRST, BAO_SpiritGuidePos[node-BAO_NODE_SPIRIT_FIRST][0], BAO_SpiritGuidePos[node-BAO_NODE_SPIRIT_FIRST][1],
                                                        BAO_SpiritGuidePos[node-BAO_NODE_SPIRIT_FIRST][2], BAO_SpiritGuidePos[node-BAO_NODE_SPIRIT_FIRST][3], team))
            TC_LOG_ERROR("bao", "Failed to spawn spirit guide! point: %u, team: %u, ", node, team);

    uint8 capturedNodes = 0;
    for (uint8 i = 0; i < BAO_NODES_COUNT; ++i)
    {
        if (m_Nodes[node].status == GetTeamIndexByTeamId(team) + BAO_NODE_TYPE_OCCUPIED && !m_Nodes[i].timer)
            ++capturedNodes;
    }
}

void BattleAO::DepopulateNode(uint8 node)
{
    if (node < BAO_NODE_SPIRIT_FIRST || BAO_NODE_A2 <= node)
        return;

    std::vector<ObjectGuid> ghost_list = sWorld->GetShReviveQueue(GetAOCreature(node-BAO_NODE_SPIRIT_FIRST)->GetGUID());
    if (!ghost_list.empty())
    {
        WorldSafeLocsEntry const* ClosestGrave = NULL;
        for (std::vector<ObjectGuid>::const_iterator itr = ghost_list.begin(); itr != ghost_list.end(); ++itr)
        {
            Player* player = ObjectAccessor::FindPlayer(*itr);
            if (!player)
                continue;
            if (ClosestGrave = GetClosestGraveYard(player))
                player->TeleportTo(BATTLEAO_MAP, ClosestGrave->x, ClosestGrave->y, ClosestGrave->z, player->GetOrientation());
         }
    }
    if (GetAOCreature(node-BAO_NODE_SPIRIT_FIRST))
        DelCreature(node-BAO_NODE_SPIRIT_FIRST);
}

bool BattleAO::TeamHasDepForNode(uint32 team, uint8 node)
{
    for (uint8 i = 0; BAO_DepNodes[node][i] != -1 && i < 5; ++i)
        if (BAO_DepNodes[node][i] >= 6)
        {
            if (BAO_DepNodes[node][i] == GetTeamIndexByTeamId(team) + 6)
                return true;
        }
        else if (m_Nodes[BAO_DepNodes[node][i]].status == BAO_NODE_TYPE_OCCUPIED + GetTeamIndexByTeamId(team))
            return true;
    return false;
}

void BattleAO::UpdateBannersFlag(uint8 node, uint32 teambefore, uint32 teamafter)
{
    if (teambefore)
        for (uint8 i = 0; BAO_DepNodes[node][i] != -1 && i < 4; ++i)                       // Chaque d�pendance,
            if (!TeamHasDepForNode(teambefore, BAO_DepNodes[node][i]))                     // si elle n'a plus de d�pendances,
                if (GameObject* gob = GetAOObject(m_Nodes[BAO_DepNodes[node][i]].banner))  // et que sa banni�re existe,
                    gob->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);                // ne sera plus s�l�ctionnable.

    if (teamafter)
        for (uint8 i = 0; BAO_DepNodes[node][i] != -1 && i < 4; ++i)                                // Chaque d�pendance,
            if (BAO_DepNodes[node][i] < 6)                                                          // si n'est pas une base principale,
            {
                uint8 teamIndex = GetTeamIndexByTeamId(teamafter);                                  // et qu'elle n'est pas � la team,
                if (m_Nodes[BAO_DepNodes[node][i]].status != BAO_NODE_TYPE_CONTESTED + teamIndex
                    && m_Nodes[BAO_DepNodes[node][i]].status != BAO_NODE_TYPE_OCCUPIED + teamIndex)
                    if (GameObject* gob = GetAOObject(m_Nodes[BAO_DepNodes[node][i]].banner))       // et que sa banni�re existe,
                        gob->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NOT_SELECTABLE);                  // devient s�l�ctionnable.
            }
}

void BattleAO::EventPlayerClickedOnFlag(Player* source, GameObject* /*target_obj*/)
{
    uint8 node = 0;
    GameObject* obj = m_Map->GetGameObject(AoObjects[node*BAO_BANNER_MAX+7]);
    while ((node < BAO_NODES_COUNT) && ((!obj) || (!source->IsWithinDistInMap(obj, 10))))
    {
        ++node;
        obj = m_Map->GetGameObject(AoObjects[node*BAO_BANNER_MAX+7]);
    }

    if (node >= BAO_NODE_A2)
        return;
    
    source->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_ENTER_PVP_COMBAT);

    uint32 team = source->GetTeam();
    TeamId teamIndex = GetTeamIndexByTeamId(team);

    if (source->IsDeserter())
    {
        if ((m_Nodes[node].status == BAO_NODE_STATUS_ALLY_CONTESTED) || (m_Nodes[node].status == BAO_NODE_STATUS_HORDE_CONTESTED))
            NeutralTaken(node);  // prise neutre (cont 2 neutre)
        else
            NeutralAttack(node); // attaque neutre (oqp 2 cont V neutre)
    }
    else if (m_Nodes[node].prev == teamIndex + BAO_NODE_TYPE_OCCUPIED)
        TeamDefense(node, team); // d�fense faction (cont 2 oqp)
    else
        TeamAttack(node, team); // attaque faction (neutre/enemy 2 cont V oqp)
}

void BattleAO::TeamAttack(uint8 node, uint32 team)
{
    if (!TeamHasDepForNode(team, node))
        return;
    TeamId teamIndex =  GetTeamIndexByTeamId(team);
    m_Nodes[node].prev = m_Nodes[node].status;
    m_Nodes[node].status = teamIndex + BAO_NODE_TYPE_CONTESTED;
    DelBanner(node, m_Nodes[node].prev);
    CreateBanner(node, BAO_NODE_TYPE_CONTESTED, teamIndex, true);
    if ((m_Nodes[node].prev == BAO_NODE_STATUS_ALLY_OCCUPIED) || (m_Nodes[node].status == BAO_NODE_STATUS_HORDE_OCCUPIED))
        UpdateBannersFlag(node, m_Nodes[node].prev==BAO_NODE_STATUS_ALLY_OCCUPIED?ALLIANCE:(m_Nodes[node].prev==BAO_NODE_STATUS_HORDE_OCCUPIED?HORDE:0)); // perte base
    m_Nodes[node].timer = BAO_FLAG_CAPTURING_TIME;
    if (teamIndex == TEAM_ALLIANCE)
        SendMessage2ToAll(LANG_BAO_NODE_ASSAULTED, CHAT_MSG_BG_SYSTEM_ALLIANCE, NULL,LANG_BAO_ALLY, GetNodeNameId(node));
    else
         SendMessage2ToAll(LANG_BAO_NODE_ASSAULTED, CHAT_MSG_BG_SYSTEM_HORDE, NULL, LANG_BAO_HORDE, GetNodeNameId(node));
    SendNodeUpdate(node);
    PlaySoundToAll((teamIndex == TEAM_ALLIANCE) ? BAO_SOUND_NODE_ASSAULTED_ALLIANCE : BAO_SOUND_NODE_ASSAULTED_HORDE);
}

void BattleAO::TeamDefense(uint8 node, uint32 team)
{
    TeamId teamIndex =  GetTeamIndexByTeamId(team);
    m_Nodes[node].prev = m_Nodes[node].status;
    m_Nodes[node].status = teamIndex + BAO_NODE_TYPE_OCCUPIED;
    DelBanner(node, m_Nodes[node].prev);
    CreateBanner(node, BAO_NODE_TYPE_OCCUPIED, teamIndex, true);
    UpdateBannersFlag(node, 0, team); // gain base
    DepopulateNode(node);
    m_Nodes[node].timer = 0;
    if (node >= BAO_NODE_SPIRIT_FIRST)
        PopulateNode(node, (teamIndex == TEAM_ALLIANCE) ? ALLIANCE:HORDE);
    if (teamIndex == TEAM_ALLIANCE)
        SendMessage2ToAll(LANG_BAO_NODE_DEFENDED, CHAT_MSG_BG_SYSTEM_ALLIANCE, NULL, GetNodeNameId(node), 0);
    else
        SendMessage2ToAll(LANG_BAO_NODE_DEFENDED, CHAT_MSG_BG_SYSTEM_HORDE, NULL, GetNodeNameId(node), 0);
    SendNodeUpdate(node);
    PlaySoundToAll((teamIndex == TEAM_ALLIANCE) ? BAO_SOUND_NODE_CAPTURED_ALLIANCE : BAO_SOUND_NODE_CAPTURED_HORDE);
}

void BattleAO::TeamTaken(uint8 node, uint32 team)
{
    TeamId teamIndex =  GetTeamIndexByTeamId(team); // = m_Nodes[node].status-1 si (cont V taken)
    m_Nodes[node].timer = 0;
    m_Nodes[node].prev = m_Nodes[node].status;
    m_Nodes[node].status += 2;
    DelBanner(node, m_Nodes[node].prev);
    CreateBanner(node, BAO_NODE_TYPE_OCCUPIED, teamIndex, true);
    UpdateBannersFlag(node, 0, team); // gain base
    SendNodeUpdate(node);
    PopulateNode(node, team);
    if (teamIndex)
    {
        SendMessage2ToAll(LANG_BAO_NODE_TAKEN, CHAT_MSG_BG_SYSTEM_HORDE, NULL, LANG_BAO_ALLY, GetNodeNameId(node));
        PlaySoundToAll(BAO_SOUND_NODE_CAPTURED_HORDE);
    }
    else
    {
        SendMessage2ToAll(LANG_BAO_NODE_TAKEN, CHAT_MSG_BG_SYSTEM_ALLIANCE, NULL, LANG_BAO_ALLY, GetNodeNameId(node));
        PlaySoundToAll(BAO_SOUND_NODE_CAPTURED_ALLIANCE);
    }
}

void BattleAO::NeutralAttack(uint8 node)
{
    m_Nodes[node].prev = m_Nodes[node].status;
    m_Nodes[node].status -= 2; // occupied - 2 = contested
    DelBanner(node, m_Nodes[node].prev);
    CreateBanner(node, m_Nodes[node].status, TEAM_NEUTRAL, true); // ici le type est le status, pas de recup du teamindex
    UpdateBannersFlag(node, m_Nodes[node].prev==BAO_NODE_STATUS_ALLY_CONTESTED?ALLIANCE:HORDE); // perte base
    DepopulateNode(node);
    m_Nodes[node].timer = BAO_FLAG_CAPTURING_TIME;
    SendMessage2ToAll(LANG_BAO_NODE_NEUTRALASSAULTED, CHAT_MSG_BG_SYSTEM_NEUTRAL, NULL, GetNodeNameId(node, true), NULL);
    SendNodeUpdate(node);
    PlaySoundToAll(BAO_SOUND_NODE_CLAIMED);
}

void BattleAO::NeutralTaken(uint8 node)
{
    m_Nodes[node].timer = 0;
    m_Nodes[node].prev = m_Nodes[node].status;
    m_Nodes[node].status = BAO_NODE_TYPE_NEUTRAL;
    DelBanner(node, m_Nodes[node].prev);
    CreateBanner(node, BAO_NODE_TYPE_NEUTRAL, TEAM_NEUTRAL, true);
    SendNodeUpdate(node);
    SendMessage2ToAll(LANG_BAO_NODE_NEUTRALTAKEN, CHAT_MSG_BG_SYSTEM_NEUTRAL, NULL, GetNodeNameId(node, true), NULL);
    SendNodeUpdate(node);
    PlaySoundToAll(BAO_SOUND_NODE_CLAIMED);
}

void BattleAO::UpdatePlayerScore(Player* Source, uint32 type, uint32 value)
{
    // tofix no more score for bao
}

uint32 BattleAO::GetFreeSlotsForTeam(uint32 team) const
{
    if (!m_balancetag)
        return true;

    int32 otherTeam = GetPlayersCount(GetOtherTeam(team));

    uint32 diff = 0;
    if (otherTeam == GetPlayersCount(team))
        diff = 1;
    else if (otherTeam > GetPlayersCount(team))
        diff = otherTeam - GetPlayersCount(team);

    return diff;
}

void BattleAO::HandleQuestComplete(uint32 questid, Player* player)
{
    TC_LOG_DEBUG("bao", "BAO Quest %i completed", questid);
    switch (questid)
    {
        case 1://todo up ressources
        default:
            TC_LOG_DEBUG("bao", "BAO Quest %i completed but is not interesting at all", questid);
            return; //was no interesting quest at all
            break;
    }
}



void BattleAO::HandleStealth(Player* player)
{
    BattleAOPlayerMap::iterator itr = m_Players.find(player->GetGUID());
    if (itr != m_Players.end())
        itr->second.CantClickTime = sWorld->GetGameTime();    
}
