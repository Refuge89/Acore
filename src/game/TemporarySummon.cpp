/*
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 *
 * Copyright (C) 2008-2009 Trinity <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "TemporarySummon.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"

TempSummon::TempSummon( uint64 summoner ) :
Creature(), m_type(TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN), m_timer(0), m_lifetime(0), m_summoner(summoner)
, m_properties(NULL)
{
    m_isSummon = true;
}

void TempSummon::Update( uint32 diff )
{
    if (m_deathState == DEAD)
    {
        UnSummon();
        return;
    }
    switch(m_type)
    {
        case TEMPSUMMON_MANUAL_DESPAWN:
            break;
        case TEMPSUMMON_TIMED_DESPAWN:
        {
            if (m_timer <= diff)
            {
                UnSummon();
                return;
            }

            m_timer -= diff;
            break;
        }
        case TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT:
        {
            if (!isInCombat())
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;

            break;
        }

        case TEMPSUMMON_CORPSE_TIMED_DESPAWN:
        {
            if ( m_deathState == CORPSE)
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }

                m_timer -= diff;
            }
            break;
        }
        case TEMPSUMMON_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if ( m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            break;
        }
        case TEMPSUMMON_DEAD_DESPAWN:
        {
            if ( m_deathState == DEAD )
            {
                UnSummon();
                return;
            }
            break;
        }
        case TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if ( m_deathState == CORPSE || m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (!isInCombat())
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }
                else
                    m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        case TEMPSUMMON_TIMED_OR_DEAD_DESPAWN:
        {
            // if m_deathState is DEAD, CORPSE was skipped
            if (m_deathState == DEAD)
            {
                UnSummon();
                return;
            }

            if (!isInCombat() && isAlive() )
            {
                if (m_timer <= diff)
                {
                    UnSummon();
                    return;
                }
                else
                    m_timer -= diff;
            }
            else if (m_timer != m_lifetime)
                m_timer = m_lifetime;
            break;
        }
        default:
            UnSummon();
            sLog.outError("Temporary summoned creature (entry: %u) have unknown type %u of ",GetEntry(),m_type);
            break;
    }

    Creature::Update( diff );
}

void TempSummon::Summon(TempSummonType type, uint32 lifetime)
{
    m_type = type;
    m_timer = lifetime;
    m_lifetime = lifetime;

    GetMap()->Add((Creature*)this);

    AIM_Initialize();
}

void TempSummon::UnSummon()
{
    CleanupsBeforeDelete();
    AddObjectToRemoveList();

    Unit* owner = GetSummoner();
    if(owner)
    {
        if(owner->GetTypeId() == TYPEID_UNIT && ((Creature*)owner)->IsAIEnabled)
            ((Creature*)owner)->AI()->SummonedCreatureDespawn(this);

        if(!m_properties)
            return;

        if(uint32 slot = m_properties->Slot)
        {
            --slot;
            owner->m_TotemSlot[slot] = 0;
        }
    }
}

void TempSummon::SetSummonProperties(SummonPropertiesEntry const *properties)
{
    if(!properties)
        return;

    m_properties = properties;

    if(uint32 slot = m_properties->Slot)
    {
        --slot;
        Unit* owner = GetSummoner();
        if(owner)
        {
            if(owner->m_TotemSlot[slot] && owner->m_TotemSlot[slot] != GetGUID())
            {
                Creature *OldTotem = ObjectAccessor::GetCreature(*this, owner->m_TotemSlot[slot]);
                if(OldTotem && OldTotem->isSummon())
                    ((TempSummon*)OldTotem)->UnSummon();
            }
            owner->m_TotemSlot[slot] = GetGUID();
        }
    }

    AIM_Initialize();
}

void TempSummon::SaveToDB()
{
}

