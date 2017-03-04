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
SDName: Arathi Highlands
SD%Complete: 100
SDComment: Quest support: 660, 665
SDCategory: Arathi Highlands
EndScriptData */

/* ContentData
npc_professor_phizzlethorpe
npc_kinelory
EndContentData */

#include "precompiled.h"
#include "escort_ai.h"

/*######
## npc_professor_phizzlethorpe
######*/

enum
{
    SAY_PROGRESS_1          = -1000264,
    SAY_PROGRESS_2          = -1000265,
    SAY_PROGRESS_3          = -1000266,
    EMOTE_PROGRESS_4        = -1000267,
    SAY_AGGRO               = -1000268,
    SAY_PROGRESS_5          = -1000269,
    SAY_PROGRESS_6          = -1000270,
    SAY_PROGRESS_7          = -1000271,
    EMOTE_PROGRESS_8        = -1000272,
    SAY_PROGRESS_9          = -1000273,

    QUEST_SUNKEN_TREASURE   = 665,
    ENTRY_VENGEFUL_SURGE    = 2776
};

struct npc_professor_phizzlethorpeAI : public npc_escortAI
{
    npc_professor_phizzlethorpeAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    void Reset() override { }

    void WaypointReached(uint32 uiPointId) override
    {
        Player* pPlayer = GetPlayerForEscort();

        if (!pPlayer)
            return;

        switch (uiPointId)
        {
            case 4: DoScriptText(SAY_PROGRESS_2, m_creature, pPlayer); break;
            case 5: DoScriptText(SAY_PROGRESS_3, m_creature, pPlayer); break;
            case 8: DoScriptText(EMOTE_PROGRESS_4, m_creature); break;
            case 9:
                m_creature->SummonCreature(ENTRY_VENGEFUL_SURGE, -2056.41f, -2144.01f, 20.59f, 5.70f, TEMPSUMMON_TIMED_OOC_OR_CORPSE_DESPAWN, 600000);
                m_creature->SummonCreature(ENTRY_VENGEFUL_SURGE, -2050.17f, -2140.02f, 19.54f, 5.17f, TEMPSUMMON_TIMED_OOC_OR_CORPSE_DESPAWN, 600000);
                break;
            case 10: DoScriptText(SAY_PROGRESS_5, m_creature, pPlayer); break;
            case 11:
                DoScriptText(SAY_PROGRESS_6, m_creature, pPlayer);
                SetRun();
                break;
            case 19: DoScriptText(SAY_PROGRESS_7, m_creature, pPlayer); break;
            case 20:
                DoScriptText(EMOTE_PROGRESS_8, m_creature);
                DoScriptText(SAY_PROGRESS_9, m_creature, pPlayer);
                pPlayer->GroupEventHappens(QUEST_SUNKEN_TREASURE, m_creature);
                break;
        }
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        pSummoned->AI()->AttackStart(m_creature);
    }
};

bool QuestAccept_npc_professor_phizzlethorpe(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_SUNKEN_TREASURE)
    {
        pCreature->SetFactionTemporary(FACTION_ESCORT_N_NEUTRAL_PASSIVE, TEMPFACTION_RESTORE_RESPAWN);
        DoScriptText(SAY_PROGRESS_1, pCreature, pPlayer);

        if (npc_professor_phizzlethorpeAI* pEscortAI = dynamic_cast<npc_professor_phizzlethorpeAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest, true);
    }
    return true;
}

CreatureAI* GetAI_npc_professor_phizzlethorpe(Creature* pCreature)
{
    return new npc_professor_phizzlethorpeAI(pCreature);
}

/*######
## npc_kinelory
######*/

enum
{
    SAY_START               = -1000948,
    SAY_REACH_BOTTOM        = -1000949,
    SAY_AGGRO_KINELORY      = -1000950,
    SAY_AGGRO_JORELL        = -1000951,
    SAY_WATCH_BACK          = -1000952,
    EMOTE_BELONGINGS        = -1000953,
    SAY_DATA_FOUND          = -1000954,
    SAY_ESCAPE              = -1000955,
    SAY_FINISH              = -1000956,
    EMOTE_HAND_PACK         = -1000957,

    SPELL_REJUVENATION      = 3627,
    SPELL_BEAR_FORM         = 4948,

    NPC_JORELL              = 2733,
    NPC_QUAE                = 2712,

    QUEST_HINTS_NEW_PLAGUE  = 660
};

struct npc_kineloryAI : public npc_escortAI
{
    npc_kineloryAI(Creature* pCreature) : npc_escortAI(pCreature) { Reset(); }

    uint32 m_uiBearFormTimer;
    uint32 m_uiHealTimer;

    void Reset() override
    {
        m_uiBearFormTimer = Randomize(urand(5000, 7000));
        m_uiHealTimer     = Randomize(urand(2000, 5000));
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 9:
                DoScriptText(SAY_REACH_BOTTOM, m_creature);
                break;
            case 16:
                DoScriptText(SAY_WATCH_BACK, m_creature);
                DoScriptText(EMOTE_BELONGINGS, m_creature);
                break;
            case 17:
                DoScriptText(SAY_DATA_FOUND, m_creature);
                break;
            case 18:
                DoScriptText(SAY_ESCAPE, m_creature);
                if (Player* pPlayer = GetPlayerForEscort())
                    m_creature->SetFacingToObject(pPlayer);
                SetRun();
                break;
            case 33:
                DoScriptText(SAY_FINISH, m_creature);
                if (Creature* pQuae = GetClosestCreatureWithEntry(m_creature, NPC_QUAE, 10.0f))
                {
                    DoScriptText(EMOTE_HAND_PACK, m_creature, pQuae);
                    m_creature->SetFacingToObject(pQuae);
                }
                break;
            case 34:
                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_HINTS_NEW_PLAGUE, m_creature);
                break;
        }
    }

    void Aggro(Unit* pWho) override
    {
        if (pWho->GetEntry() == NPC_JORELL)
            DoScriptText(SAY_AGGRO_JORELL, pWho, m_creature);
        else if (roll_chance_i(10))
            DoScriptText(SAY_AGGRO_KINELORY, m_creature);
    }

    void ReceiveAIEvent(AIEventType eventType, Creature* /*pSender*/, Unit* pInvoker, uint32 uiMiscValue) override
    {
        if (eventType == AI_EVENT_START_ESCORT && pInvoker->GetTypeId() == TYPEID_PLAYER)
        {
            DoScriptText(SAY_START, m_creature);
            Start(false, (Player*)pInvoker, GetQuestTemplateStore(uiMiscValue), true);
        }
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiBearFormTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BEAR_FORM) == CAST_OK)
                m_uiBearFormTimer = Randomize(urand(25000, 30000));
        }
        else
            m_uiBearFormTimer -= uiDiff;

        if (m_uiHealTimer < uiDiff)
        {
            if (Unit* pTarget = DoSelectLowestHpFriendly(40.0f))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_REJUVENATION) == CAST_OK)
                    m_uiHealTimer = Randomize(urand(15000, 25000));
            }
        }
        else
            m_uiHealTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_kinelory(Creature* pCreature)
{
    return new npc_kineloryAI(pCreature);
}

bool QuestAccept_npc_kinelory(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_HINTS_NEW_PLAGUE)
        pCreature->AI()->SendAIEvent(AI_EVENT_START_ESCORT, pPlayer, pCreature, pQuest->GetQuestId());

    return true;
}

bool QuestAccept_npc_shakes_o_breen(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_DEATH_FROM_BELOW)
    {
        if (auto pEscortAI = dynamic_cast<npc_shakes_o_breenAI*>(pCreature->AI()))
        {
            pCreature->MonsterYell(BREEN_YELL_1);
            pEscortAI->Start(false, pPlayer->GetGUID(), pQuest);
            pEscortAI->SetEscortPaused(true);
            pCreature->SetOrientation(2.67f);
        }
    }

    return true;
}

CreatureAI* GetAI_npc_shakes_o_breen(Creature* pCreature)
{
    return new npc_shakes_o_breenAI(pCreature);
}

enum
{
    SPELL_BOLT_CHARGE_BRAMBLE       = 9056,

    GO_WITHERBARK_TOTEM_BUNDLE      = 174764
};

struct npc_items_of_power_watcherAI : public ScriptedAI
{
    npc_items_of_power_watcherAI(Creature* pCreature) : ScriptedAI(pCreature) { npc_items_of_power_watcherAI::Reset(); }

    void Reset() override
    {
        m_uiCastTimer = 8000;
        m_uiDeathTimer = 12000;

        m_uiStarted = false;
        m_uiCastDone = false;
        m_uiDeleted = false;
    }

    ObjectGuid playerGUID;

    uint32 m_uiCastTimer;
    uint32 m_uiDeathTimer;

    bool m_uiStarted;
    bool m_uiCastDone;
    bool m_uiDeleted;

    void StartEvent(Player* pPlayer)
    {
        m_uiStarted = true;
        if (pPlayer)
            playerGUID = pPlayer->GetObjectGuid();
    }
    
    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_uiStarted)
            return;

        if (!m_uiCastDone && m_uiCastTimer < uiDiff)
        {
            if (!m_uiDeleted)
                if (GameObject* pGo = GetClosestGameObjectWithEntry(m_creature, GO_WITHERBARK_TOTEM_BUNDLE, 10.0f))
                {
                    pGo->Delete();
                    m_uiDeleted = true;
                    m_uiCastTimer = 400;
                    return;
                }

            if (Player* player = m_creature->GetMap()->GetPlayer(playerGUID))
                player->CastSpell(player, SPELL_BOLT_CHARGE_BRAMBLE, true);
            else
                DoCastSpellIfCan(m_creature, SPELL_BOLT_CHARGE_BRAMBLE, CAST_TRIGGERED);
            m_uiCastDone = true;
        }
        else
            m_uiCastTimer -= uiDiff;

        if (m_uiDeathTimer < uiDiff)
            m_creature->ForcedDespawn();
        else
            m_uiDeathTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_items_of_power_watcher(Creature* pCreature)
{
    return new npc_items_of_power_watcherAI(pCreature);
}

enum
{
    NPC_ITEMS_OF_POWER_WATCHER      = 5002
};

bool item_witherbark_totem_stick(Player* pPlayer, Item* pItem, SpellCastTargets const& pCastTargets)
{
    if (pPlayer)
    {
        if (Creature* pWatcher = pPlayer->SummonCreature(NPC_ITEMS_OF_POWER_WATCHER, -1350.81f, -2740.07f, 59.1623f, 0, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 30*IN_MILLISECONDS))
            if (npc_items_of_power_watcherAI* pWatcherAI = static_cast<npc_items_of_power_watcherAI*>(pWatcher->AI()))
                pWatcherAI->StartEvent(pPlayer);
    }

    return false;
}


void AddSC_arathi_highlands()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_professor_phizzlethorpe";
    pNewScript->GetAI = &GetAI_npc_professor_phizzlethorpe;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_professor_phizzlethorpe;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_shakes_o_breen";
    pNewScript->GetAI = &GetAI_npc_shakes_o_breen;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_shakes_o_breen;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_kinelory";
    pNewScript->GetAI = &GetAI_npc_kinelory;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_kinelory;
    pNewScript->RegisterSelf();
    
    pNewScript = new Script;
    pNewScript->Name = "npc_items_of_power_watcher";
    pNewScript->GetAI = &GetAI_npc_items_of_power_watcher;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "item_witherbark_totem_stick";
    pNewScript->pItemUse = &item_witherbark_totem_stick;
    pNewScript->RegisterSelf();

}
