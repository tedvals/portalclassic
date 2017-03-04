/*
 * Copyright (C) 2010-2014 Anathema Script Engine project <http://valkyrie-wow.com/>
 * Copyright (C) 2006-2014 ScriptDev2 <http://www.scriptdev2.com/>
 */

/* ScriptData
SDName: Deeprun Tram
SD%Complete: 100
SDComment: Quest support: 6661
SDCategory: Deeprun Tram
EndScriptData */

/* ContentData
npc_deeprun_rat
EndContentData */

#include "scriptPCH.h"

/*####
# npc_deeprun_rat
####*/

enum
{
    SPELL_FLUTE         = 21050,
    SPELL_ENTHRALLED    = 21051,
    SPELL_BASH_RATS     = 21052,

    QUEST_ROUNDUP       = 6661,

    NPC_MONTY           = 12997,
    NPC_RAT             = 13016,
    NPC_RAT_F           = 13017
};

struct npc_deeprun_ratAI : public FollowerAI
{
    npc_deeprun_ratAI(Creature* pCreature) : FollowerAI(pCreature) { Reset(); }

    uint32 m_uiTimer;

    void Reset() override
    {
        m_uiTimer = 0;

        if (m_creature->GetEntry() == NPC_RAT_F)
        {
            if (!m_creature->m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE))
                m_creature->SetWalk(true);

            m_creature->UpdateEntry(NPC_RAT);
            m_creature->RemoveAurasDueToSpell(SPELL_ENTHRALLED);
        }
    }

    void MoveInLineOfSight(Unit *pWho) override
    {
        if (m_creature->getVictim())
            return;

        if (pWho->GetEntry() == NPC_MONTY && m_creature->GetEntry() == NPC_RAT_F)
        {
            if (m_creature->IsInRange(pWho, 0.0f, 10.0f) && !HasFollowState(STATE_FOLLOW_COMPLETE))
            {
                if (Player* pPlayer = GetLeaderForFollower())
                {
                    if (pPlayer->GetQuestStatus(QUEST_ROUNDUP) == QUEST_STATUS_INCOMPLETE)
                    {
                        m_uiTimer = 30000;
                        SetFollowComplete(true);
                        m_creature->GetMotionMaster()->MovePoint(0, -20.5f, 34.7f, -4.3f);
                        pPlayer->CastSpell(m_creature, SPELL_BASH_RATS, true);
                    }
                }
            }
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell) override
    {
        if (pSpell->Id == SPELL_FLUTE && pCaster->isPlayer() && m_creature->GetEntry() == NPC_RAT)
        {
            if (((Player*)pCaster)->GetQuestStatus(QUEST_ROUNDUP) == QUEST_STATUS_INCOMPLETE ||
                ((Player*)pCaster)->GetQuestStatus(QUEST_ROUNDUP) == QUEST_STATUS_COMPLETE)
            {
                m_creature->UpdateEntry(NPC_RAT_F);
                m_creature->CastSpell(m_creature, SPELL_ENTHRALLED, false);

                StartFollow((Player*)pCaster);

                if (m_creature->m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE))
                    m_creature->SetWalk(false);
            }

        }
        else if (pSpell->Id == SPELL_BASH_RATS && m_creature->GetEntry() == NPC_RAT_F)
            SetFollowComplete();
        else if (pSpell->Id == SPELL_ENTHRALLED && m_creature->getFaction() != FACTION_ESCORT_A_PASSIVE)
            m_creature->setFaction(FACTION_ESCORT_A_PASSIVE);
    }

    void UpdateFollowerAI(const uint32 uiDiff) override
    {
        if (m_uiTimer && HasFollowState(STATE_FOLLOW_COMPLETE))
        {
            if (m_uiTimer < uiDiff)
                m_creature->ForcedDespawn();
            else
                m_uiTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_npc_deeprun_rat(Creature* pCreature)
{
    return new npc_deeprun_ratAI(pCreature);
}

void AddSC_deeprun_tram()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_deeprun_rat";
    pNewScript->GetAI = &GetAI_npc_deeprun_rat;
    pNewScript->RegisterSelf();
}