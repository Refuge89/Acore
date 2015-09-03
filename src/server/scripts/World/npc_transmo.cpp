#include "ScriptMgr.h"

enum NpcTransmo
{
    LANG_REM_TRANSMOGRIFICATIONS_ITEMS  = 11100,
    LANG_ERR_NO_TRANSMOGRIFICATIONS     = 11101,
    LANG_REM_TRANSMOGRIFICATION_ITEM    = 11102,
    LANG_ERR_NO_TRANSMOGRIFICATION      = 11103,
    LANG_ITEM_TRANSMOGRIFIED            = 11104,
    LANG_ERR_NO_ITEM_SUITABLE           = 11105,
    LANG_ERR_NO_ITEM_EXISTS             = 11106,
    LANG_ERR_EQUIP_SLOT_EMPTY           = 11107,

    LANG_SLOT_NAME_HEAD                 = 11108,
    LANG_SLOT_NAME_SHOULDERS            = 11109,
    LANG_SLOT_NAME_BODY                 = 11110,
    LANG_SLOT_NAME_CHEST                = 11111,
    LANG_SLOT_NAME_WAIST                = 11112,
    LANG_SLOT_NAME_LEGS                 = 11113,
    LANG_SLOT_NAME_FEET                 = 11114,
    LANG_SLOT_NAME_WRISTS               = 11115,
    LANG_SLOT_NAME_HANDS                = 11116,
    LANG_SLOT_NAME_BACK                 = 11117,
    LANG_SLOT_NAME_MAINHAND             = 11118,
    LANG_SLOT_NAME_OFFHAND              = 11119,
    LANG_SLOT_NAME_RANGED               = 11120,
    LANG_SLOT_NAME_TABARD               = 11121,

    LANG_OPTION_BACK                    = 11122,
    LANG_OPTION_REMOVE_ALL              = 11123,
    LANG_POPUP_REMOVE_ALL               = 11124,
    LANG_OPTION_UPDATE_MENU             = 11125,
    LANG_OPTION_REMOVE_ONE              = 11126,
    LANG_POPUP_REMOVE_ONE               = 11127,
    LANG_POPUP_TRANSMOGRIFY             = 11128
};

class npc_transmo : public CreatureScript
{
public:
    npc_transmo() : CreatureScript("npc_transmo") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        WorldSession* session = player->GetSession();
        creature->HasQuestForPlayer(player);
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_TABARD; slot++) // EQUIPMENT_SLOT_END
        {
            if (Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                if (const char* slotName = GetSlotName(slot, session))
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, slotName, EQUIPMENT_SLOT_END, slot);
        }
        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, session->GetTrinityString(LANG_OPTION_REMOVE_ALL), EQUIPMENT_SLOT_END+2, 0, session->GetTrinityString(LANG_POPUP_REMOVE_ALL), 0, false);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, session->GetTrinityString(LANG_OPTION_UPDATE_MENU), EQUIPMENT_SLOT_END+1, 0);
        player->SEND_GOSSIP_MENU(player->GetDefaultGossipMenuForSource(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 uiAction)
    {
        WorldSession* session = player->GetSession();
        player->PlayerTalkClass->ClearMenus();
        switch(sender)
        {
        case EQUIPMENT_SLOT_END: {
            if (Item* oldItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, uiAction)) {
                uint32 lowGUID = player->GetGUIDLow();
                _items[lowGUID].clear();
                uint32 limit = 0;

                for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; i++) {
                    if (limit > 30) break;
                    if (Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i)) {
                        uint32 display = newItem->GetTemplate()->DisplayInfoID;
                        if (player->SuitableForTransmogrification(oldItem, newItem) == ERR_FAKE_OK) {
                            if (_items[lowGUID].find(display) == _items[lowGUID].end()) {
                                limit++;
                                _items[lowGUID][display] = newItem;
                                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, GetItemName(newItem, session), uiAction, display, session->GetTrinityString(LANG_POPUP_TRANSMOGRIFY)+GetItemName(newItem, session), 0, false);
                                } } } }

                for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++) {
                    if (Bag* bag = player->GetBagByPos(i)) {
                        for (uint32 j = 0; j < bag->GetBagSize(); j++) {
                            if (limit > 30) break;
                            if (Item* newItem = player->GetItemByPos(i, j)) {
                                uint32 display = newItem->GetTemplate()->DisplayInfoID;
                                if (player->SuitableForTransmogrification(oldItem, newItem) == ERR_FAKE_OK) {
                                    if (_items[lowGUID].find(display) == _items[lowGUID].end()) {
                                        limit++;
                                        _items[lowGUID][display] = newItem;
                                        player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, GetItemName(newItem, session), uiAction, display, session->GetTrinityString(LANG_POPUP_TRANSMOGRIFY)+GetItemName(newItem, session), 0, false);
                                        } } } } } }

                char popup[250];
                snprintf(popup, 250, session->GetTrinityString(LANG_POPUP_REMOVE_ONE), GetSlotName(uiAction, session));
                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_INTERACT_1, session->GetTrinityString(LANG_OPTION_REMOVE_ONE), EQUIPMENT_SLOT_END+3, uiAction, popup, 0, false);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, session->GetTrinityString(LANG_OPTION_BACK), EQUIPMENT_SLOT_END+1, 0);
                player->SEND_GOSSIP_MENU(player->GetDefaultGossipMenuForSource(creature), creature->GetGUID()); }
            else OnGossipHello(player, creature); }
            break;

        case EQUIPMENT_SLOT_END+1: { OnGossipHello(player, creature); } break;

        case EQUIPMENT_SLOT_END+2: { // Remove Transmogrifications
            bool removed = false;
            for (uint8 Slot = EQUIPMENT_SLOT_START; Slot < EQUIPMENT_SLOT_END; Slot++) {
                if (Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, Slot)) {
                    if (newItem->DeleteFakeEntry() && !removed)
                        removed = true; } }
            if (removed) {
                session->SendAreaTriggerMessage("%s", session->GetTrinityString(LANG_REM_TRANSMOGRIFICATIONS_ITEMS));
                player->PlayDirectSound(3337); }
            else session->SendNotification("%s", session->GetTrinityString(LANG_ERR_NO_TRANSMOGRIFICATIONS));
            OnGossipHello(player, creature); }
            break;

        case EQUIPMENT_SLOT_END+3: { // Remove Transmogrification from single item
            if (Item* newItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, uiAction)) {
                if (newItem->DeleteFakeEntry()) {
                    session->SendAreaTriggerMessage(session->GetTrinityString(LANG_REM_TRANSMOGRIFICATION_ITEM), GetSlotName(uiAction, session));
                    player->PlayDirectSound(3337); }
                else session->SendNotification(session->GetTrinityString(LANG_ERR_NO_TRANSMOGRIFICATION), GetSlotName(uiAction, session)); }
            OnGossipHello(player, creature); }
            break;

        default: { // Transmogrify
            uint32 lowGUID = player->GetGUIDLow();
            if (Item* oldItem = player->GetItemByPos(INVENTORY_SLOT_BAG_0, sender)) {
                if (_items[lowGUID].find(uiAction) != _items[lowGUID].end() && _items[lowGUID][uiAction]->IsInWorld()) {
                    Item* newItem = _items[lowGUID][uiAction];
                    if (newItem->GetOwnerGUID() == player->GetGUIDLow() && (newItem->IsInBag() || newItem->GetBagSlot() == INVENTORY_SLOT_BAG_0) && player->SuitableForTransmogrification(oldItem, newItem) == ERR_FAKE_OK) {
                        /*if (!player->HasEnoughMoney(0)) {
                            creature->MonsterWhisper(session->GetTrinityString(11129), player->GetGUID());
                            player->PlayerTalkClass->SendCloseGossip();
                            return false; }
                        //player->ModifyMoney(-10000); // take cost */
                        oldItem->SetFakeEntry(newItem->GetEntry());
//                        player->DestroyItemCount(newItem->GetTemplate()->ItemId, 1, true, false);
                        player->PlayDirectSound(3337);
                        session->SendAreaTriggerMessage(session->GetTrinityString(LANG_ITEM_TRANSMOGRIFIED), GetSlotName(sender, session)); }
                    else session->SendNotification("%s", session->GetTrinityString(LANG_ERR_NO_ITEM_SUITABLE)); }
//                else session->SendNotification("%s", session->GetTrinityString(LANG_ERR_NO_ITEM_EXISTS));
            } else session->SendNotification("%s", session->GetTrinityString(LANG_ERR_EQUIP_SLOT_EMPTY));
            _items[lowGUID].clear();
            OnGossipHello(player, creature); } break;
                }
        return true;
    }

private: std::map<uint64, std::map<uint32, Item*> > _items; // _items[lowGUID][DISPLAY] = item

    const char * GetSlotName(uint8 slot, WorldSession* session)
    {
        switch(slot) {
        case EQUIPMENT_SLOT_HEAD      : return session->GetTrinityString(LANG_SLOT_NAME_HEAD);
        case EQUIPMENT_SLOT_SHOULDERS : return session->GetTrinityString(LANG_SLOT_NAME_SHOULDERS);
        case EQUIPMENT_SLOT_BODY      : return session->GetTrinityString(LANG_SLOT_NAME_BODY);
        case EQUIPMENT_SLOT_CHEST     : return session->GetTrinityString(LANG_SLOT_NAME_CHEST);
        case EQUIPMENT_SLOT_WAIST     : return session->GetTrinityString(LANG_SLOT_NAME_WAIST);
        case EQUIPMENT_SLOT_LEGS      : return session->GetTrinityString(LANG_SLOT_NAME_LEGS);
        case EQUIPMENT_SLOT_FEET      : return session->GetTrinityString(LANG_SLOT_NAME_FEET);
        case EQUIPMENT_SLOT_WRISTS    : return session->GetTrinityString(LANG_SLOT_NAME_WRISTS);
        case EQUIPMENT_SLOT_HANDS     : return session->GetTrinityString(LANG_SLOT_NAME_HANDS);
        case EQUIPMENT_SLOT_BACK      : return session->GetTrinityString(LANG_SLOT_NAME_BACK);
        case EQUIPMENT_SLOT_MAINHAND  : return session->GetTrinityString(LANG_SLOT_NAME_MAINHAND);
        case EQUIPMENT_SLOT_OFFHAND   : return session->GetTrinityString(LANG_SLOT_NAME_OFFHAND);
        case EQUIPMENT_SLOT_RANGED    : return session->GetTrinityString(LANG_SLOT_NAME_RANGED);
        case EQUIPMENT_SLOT_TABARD    : return session->GetTrinityString(LANG_SLOT_NAME_TABARD);
        default: return NULL; }
    }

    std::string GetItemName(Item* item, WorldSession* session)
    {
        std::string name = item->GetTemplate()->Name1;
        int loc_idx = session->GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
            if (ItemLocale const* il = sObjectMgr->GetItemLocale(item->GetEntry()))
                ObjectMgr::GetLocaleString(il->Name, loc_idx, name);
        return name;
    }
};

void AddSC_npc_transmo()
{
    new npc_transmo();
}
