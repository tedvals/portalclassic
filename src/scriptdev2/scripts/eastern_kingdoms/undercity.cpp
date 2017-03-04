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
SDName: Undercity
SD%Complete:
SDComment: Placeholder
SDCategory: Undercity
EndScriptData */

/* ContentData
EndContentData */

#include "precompiled.h"

#define SAY_LAMENT_END              -1000196
#define EMOTE_LAMENT_END            -1000197

#define SOUND_CREDIT                10896
#define ENTRY_HIGHBORNE_LAMENTER    21628
#define ENTRY_HIGHBORNE_BUNNY       21641

#define SPELL_HIGHBORNE_AURA        37090
#define SPELL_SYLVANAS_CAST         36568
#define SPELL_RIBBON_OF_SOULS       34432                   //the real one to use might be 37099

float HighborneLoc[4][3]=
{
    {1285.41f, 312.47f, 0.51f},
    {1286.96f, 310.40f, 1.00f},
    {1289.66f, 309.66f, 1.52f},
    {1292.51f, 310.50f, 1.99f}
};
#define HIGHBORNE_LOC_Y             -61.00f
#define HIGHBORNE_LOC_Y_NEW         -55.50f

struct npc_lady_sylvanas_windrunnerAI : public ScriptedAI
{
    npc_lady_sylvanas_windrunnerAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    uint32 LamentEvent_Timer;
    bool LamentEvent;
    uint64 targetGUID;

    float myX;
    float myY;
    float myZ;

    void Reset() override
    {
        myX = m_creature->GetPositionX();
        myY = m_creature->GetPositionY();
        myZ = m_creature->GetPositionZ();

        LamentEvent_Timer = 5000;
        LamentEvent = false;
        targetGUID = 0;
    }

    void JustSummoned(Creature *summoned) override
    {
        if (summoned->GetEntry() == ENTRY_HIGHBORNE_BUNNY)
        {
            if (Creature* pBunny = summoned->GetMap()->GetCreature(targetGUID))
            {
                pBunny->NearTeleportTo(pBunny->GetPositionX(), pBunny->GetPositionY(), myZ + 15.0f, 0.0f);
                summoned->CastSpell(pBunny, SPELL_RIBBON_OF_SOULS, false);
            }

            targetGUID = summoned->GetObjectGuid();
        }
    }

    void UpdateAI(const uint32 diff) override
    {
        if (LamentEvent)
        {
            if (LamentEvent_Timer < diff)
            {
                float raX = myX;
                float raY = myY;
                float raZ = myZ;

                m_creature->GetRandomPoint(myX, myY, myZ, 20.0f, raX, raY, raZ);
                m_creature->SummonCreature(ENTRY_HIGHBORNE_BUNNY, raX, raY, myZ, 0, TEMPSUMMON_TIMED_DESPAWN, 3000);

                LamentEvent_Timer = 2000;
                if (!m_creature->HasAura(SPELL_SYLVANAS_CAST, EFFECT_INDEX_0))
                {
                    DoScriptText(SAY_LAMENT_END, m_creature);
                    DoScriptText(EMOTE_LAMENT_END, m_creature);
                    LamentEvent = false;
                }
            }else LamentEvent_Timer -= diff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_npc_lady_sylvanas_windrunner(Creature* pCreature)
{
    return new npc_lady_sylvanas_windrunnerAI(pCreature);
}

bool QuestRewarded_npc_lady_sylvanas_windrunner(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == 9180)
    {
        ((npc_lady_sylvanas_windrunnerAI*)pCreature->AI())->LamentEvent = true;
        ((npc_lady_sylvanas_windrunnerAI*)pCreature->AI())->DoPlaySoundToSet(pCreature,SOUND_CREDIT);
        pCreature->CastSpell(pCreature,SPELL_SYLVANAS_CAST,false);

        for (uint8 i = 0; i < 4; ++i)
            pCreature->SummonCreature(ENTRY_HIGHBORNE_LAMENTER, HighborneLoc[i][0], HighborneLoc[i][1], HIGHBORNE_LOC_Y, HighborneLoc[i][2], TEMPSUMMON_TIMED_DESPAWN, 160000);
    }

    return true;
}

/*
TODO : TIMERS TO CHECK
*/

bool GossipHello_npc_estelle_gendry(Player* pPlayer, Creature* pCreature)
{
    if ((pPlayer->GetQuestStatus(1999) == QUEST_STATUS_INCOMPLETE))
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT,
                                 "I need the tools Mennet Carkad told me about.",
                                 GOSSIP_SENDER_MAIN,
                                 GOSSIP_ACTION_INFO_DEF + 10);
        pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature),
                                  pCreature->GetObjectGuid());
    }
    return true;
}

bool GossipSelect_npc_estelle_gendry(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction >= GOSSIP_ACTION_INFO_DEF + 10)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        if (!pPlayer->HasItemCount(5060, 1, true))
        {
            uint32 noSpaceForCount = 0;
            ItemPosCountVec dest;
            uint8 msg = pPlayer->CanStoreNewItem(NULL_BAG, NULL_SLOT,
                                                 dest, 5060, 1, &noSpaceForCount);

            if (msg == EQUIP_ERR_OK)
            {
                Item* pItem = pPlayer->StoreNewItem(dest, 5060,
                                                    true, Item::GenerateItemRandomPropertyId(5060));
                pPlayer->SendNewItem(pItem, 1, true, false);
            }
        }
    }
    return true;
}



/*######
## npc_parqual_fintallas
######*/

#define SPELL_MARK_OF_SHAME 6767

bool GossipHello_npc_parqual_fintallas(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->GetQuestStatus(6628) == QUEST_STATUS_INCOMPLETE && !pPlayer->HasAura(SPELL_MARK_OF_SHAME, EFFECT_INDEX_0))
    {
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Gul'dan", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Kel'Thuzad", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Ner'zhul", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        pPlayer->SEND_GOSSIP_MENU(5822, pCreature->GetGUID());
    }
    else
        pPlayer->SEND_GOSSIP_MENU(5821, pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_parqual_fintallas(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->CastSpell(pPlayer, SPELL_MARK_OF_SHAME, false);
    }
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 2)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pPlayer->AreaExploredOrEventHappens(6628);
    }
    return true;
}

/*#####
# npc_innkeeper_norman
######*/

enum
{
    QUEST_CHICKEN_CLUCKING_FOR_A_MINT  = 8354
};

struct npc_innkeeper_normanAI : public ScriptedAI
{
    npc_innkeeper_normanAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

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

CreatureAI* GetAI_npc_innkeeper_norman(Creature* pCreature)
{
    return new npc_innkeeper_normanAI(pCreature);
}

bool QuestAccept_npc_innkeeper_norman(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_CHICKEN_CLUCKING_FOR_A_MINT)
    {
        if (npc_innkeeper_normanAI* pShenAI = dynamic_cast<npc_innkeeper_normanAI*>(pCreature->AI()))
            pShenAI->DoStartQuestEvent(pPlayer);
    }

    return true;
}

/*######
## AddSC
######*/

void AddSC_undercity()
{
    Script *pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_innkeeper_norman";
    pNewScript->GetAI = &GetAI_npc_innkeeper_norman;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_innkeeper_norman;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_lady_sylvanas_windrunner";
    pNewScript->GetAI = &GetAI_npc_lady_sylvanas_windrunner;
    pNewScript->pQuestRewardedNPC  = &QuestRewarded_npc_lady_sylvanas_windrunner;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_parqual_fintallas";
    pNewScript->pGossipHello = &GossipHello_npc_parqual_fintallas;
    pNewScript->pGossipSelect = &GossipSelect_npc_parqual_fintallas;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_estelle_gendry";
    pNewScript->pGossipHello = &GossipHello_npc_estelle_gendry;
    pNewScript->pGossipSelect = &GossipSelect_npc_estelle_gendry;
    pNewScript->RegisterSelf();
}
