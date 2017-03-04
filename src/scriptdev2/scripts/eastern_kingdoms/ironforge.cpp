/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Ironforge
SD%Complete: 100
SDComment: Quest support: 3702, 8353, 8355
SDCategory: Ironforge
EndScriptData */

/* ContentData
npc_royal_historian_archesonus
npc_innkeeper_firebrew
npc_talvash_del_kissel
EndContentData */

/* ContentData
EndContentData */

#include "precompiled.h"

/*######
## npc_royal_historian_archesonus
######*/

#define GOSSIP_ITEM_ROYAL   "I am ready to listen"
#define GOSSIP_ITEM_ROYAL_1 "That is tragic. How did this happen?"
#define GOSSIP_ITEM_ROYAL_2 "Interesting, continue please."
#define GOSSIP_ITEM_ROYAL_3 "Unbelievable! How dare they??"
#define GOSSIP_ITEM_ROYAL_4 "Of course I will help!"

bool GossipHello_npc_royal_historian_archesonus(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestStatus(3702) == QUEST_STATUS_INCOMPLETE)
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ROYAL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        pPlayer->SEND_GOSSIP_MENU(2235, pCreature->GetObjectGuid());
    }
    else
        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_royal_historian_archesonus(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    pPlayer->PlayerTalkClass->ClearMenus();

    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ROYAL_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            pPlayer->SEND_GOSSIP_MENU(2236, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF + 1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ROYAL_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(2237, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ROYAL_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(2238, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ROYAL_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            pPlayer->SEND_GOSSIP_MENU(2239, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF + 4:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(3702);
            break;
    }
    return true;
}

/*#####
# npc_innkeeper_firebrew
######*/

enum
{
    QUEST_CHICKEN_CLUCKING_FOR_A_MINT  = 8353
};

struct npc_innkeeper_firebrewAI : public ScriptedAI
{
    npc_innkeeper_firebrewAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiChickenTimer;
    uint32 m_uiResetTimer;

    ObjectGuid m_playerGuid;

    void Reset() override
    {
        m_uiChickenTimer = 0;
        m_uiResetTimer = 0;

        m_playerGuid.Clear();
    }

    void ReceiveEmote(Player* pPlayer, uint32 uiTextEmote) override
    {
        if (m_uiResetTimer && uiTextEmote == TEXTEMOTE_CHICKEN && pPlayer->GetQuestStatus(QUEST_CHICKEN_CLUCKING_FOR_A_MINT) == QUEST_STATUS_INCOMPLETE)
        {
            pPlayer->AreaExploredOrEventHappens(QUEST_CHICKEN_CLUCKING_FOR_A_MINT);
            EnterEvadeMode();
        }
    }

    void DoStartQuestEvent(Player* pPlayer)
    {
        m_playerGuid = pPlayer->GetObjectGuid();
        m_uiChickenTimer = 2000;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiResetTimer)
        {
            if (m_uiResetTimer <= uiDiff)
            {
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                {
                    if (pPlayer->GetTypeId() == TYPEID_PLAYER && pPlayer->GetQuestStatus(QUEST_CHICKEN_CLUCKING_FOR_A_MINT) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->FailQuest(QUEST_CHICKEN_CLUCKING_FOR_A_MINT);
                }

                m_uiResetTimer = 0;
                EnterEvadeMode();
            }
            else
                m_uiResetTimer -= uiDiff;
        }

        if (m_uiChickenTimer)
        {
            if (m_uiChickenTimer <= uiDiff)
            {
                m_creature->HandleEmote(EMOTE_ONESHOT_CHICKEN);
                m_uiResetTimer = 60000;
                m_uiChickenTimer = 0;
            }
            else
                m_uiChickenTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_innkeeper_firebrew(Creature* pCreature)
{
    return new npc_innkeeper_firebrewAI(pCreature);
}

bool QuestAccept_npc_innkeeper_firebrew(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_CHICKEN_CLUCKING_FOR_A_MINT)
    {
        if (npc_innkeeper_firebrewAI* pShenAI = dynamic_cast<npc_innkeeper_firebrewAI*>(pCreature->AI()))
            pShenAI->DoStartQuestEvent(pPlayer);
    }

    return true;
}

/*#####
# npc_talvash_del_kissel
######*/

enum
{
    QUEST_INCOMING_GUMDROP  = 8355
};
enum
{
    TEXTEMOTE_TRAIN         = 264
};

struct npc_talvash_del_kisselAI : public ScriptedAI
{
    npc_talvash_del_kisselAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 m_uiTrainTimer;
    uint32 m_uiResetTimer;

    ObjectGuid m_playerGuid;

    void Reset() override
    {
        m_uiTrainTimer = 0;
        m_uiResetTimer = 0;

        m_playerGuid.Clear();
    }

    void ReceiveEmote(Player* pPlayer, uint32 uiTextEmote) override
    {
        if (m_uiResetTimer && uiTextEmote == TEXTEMOTE_TRAIN && pPlayer->GetQuestStatus(QUEST_INCOMING_GUMDROP) == QUEST_STATUS_INCOMPLETE)
        {
            pPlayer->AreaExploredOrEventHappens(QUEST_INCOMING_GUMDROP);
            EnterEvadeMode();
        }
    }

    void DoStartQuestEvent(Player* pPlayer)
    {
        m_playerGuid = pPlayer->GetObjectGuid();
        m_uiTrainTimer = 2000;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiResetTimer)
        {
            if (m_uiResetTimer <= uiDiff)
            {
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                {
                    if (pPlayer->GetTypeId() == TYPEID_PLAYER && pPlayer->GetQuestStatus(QUEST_INCOMING_GUMDROP) == QUEST_STATUS_INCOMPLETE)
                        pPlayer->FailQuest(QUEST_INCOMING_GUMDROP);
                }

                m_uiResetTimer = 0;
                EnterEvadeMode();
            }
            else
                m_uiResetTimer -= uiDiff;
        }

        if (m_uiTrainTimer)
        {
            if (m_uiTrainTimer <= uiDiff)
            {
                m_creature->HandleEmote(EMOTE_ONESHOT_TRAIN);
                m_uiResetTimer = 60000;
                m_uiTrainTimer = 0;
            }
            else
                m_uiTrainTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_talvash_del_kissel(Creature* pCreature)
{
    return new npc_talvash_del_kisselAI(pCreature);
}

bool QuestAccept_npc_talvash_del_kissel(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_INCOMING_GUMDROP)
    {
        if (npc_talvash_del_kisselAI* pShenAI = dynamic_cast<npc_talvash_del_kisselAI*>(pCreature->AI()))
            pShenAI->DoStartQuestEvent(pPlayer);
    }

    return true;
}

void AddSC_ironforge()
{
    Script *pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_royal_historian_archesonus";
    pNewScript->pGossipHello =  &GossipHello_npc_royal_historian_archesonus;
    pNewScript->pGossipSelect = &GossipSelect_npc_royal_historian_archesonus;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_innkeeper_firebrew";
    pNewScript->GetAI = &GetAI_npc_innkeeper_firebrew;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_innkeeper_firebrew;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_talvash_del_kissel";
    pNewScript->GetAI = &GetAI_npc_talvash_del_kissel;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_talvash_del_kissel;
    pNewScript->RegisterSelf();
}
