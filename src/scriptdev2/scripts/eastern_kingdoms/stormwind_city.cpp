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
SDName: Stormwind_City
SD%Complete: 100
SDComment: Quest support: 1640, 1447, 4185, 6402, 6403.
SDCategory: Stormwind City
EndScriptData */

/* ContentData
npc_bartleby
npc_dashel_stonefist
npc_lady_katrana_prestor
npc_squire_rowe
npc_reginald_windsor
EndContentData */

#include "precompiled.h"
#include "../world/world_map_scripts.h"
#include "escort_ai.h"

/*######
## npc_archmage_malin
######*/

#define GOSSIP_ITEM_MALIN "Can you send me to Theramore? I have an urgent message for Lady Jaina from Highlord Bolvar."

bool GossipHello_npc_archmage_malin(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetGUID());

    if (pPlayer->GetQuestStatus(11223) == QUEST_STATUS_COMPLETE && !pPlayer->GetQuestRewardStatus(11223))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_MALIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetGUID());

    return true;
}

bool GossipSelect_npc_archmage_malin(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->CastSpell(pPlayer, 42711, true);
    }

    return true;
}

/*######
## npc_bartleby
######*/

enum
{
    FACTION_ENEMY       = 168,
    QUEST_BEAT          = 1640
};

struct npc_bartlebyAI : public ScriptedAI
{
    npc_bartlebyAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    void Reset() override {}

    void AttackedBy(Unit* pAttacker) override
    {
        if (m_creature->getVictim())
            return;

        if (m_creature->IsFriendlyTo(pAttacker))
            return;

        AttackStart(pAttacker);
    }

    void DamageTaken(Unit* pDoneBy, uint32& uiDamage, DamageEffectType /*damagetype*/) override
    {
        if (uiDamage > m_creature->GetHealth() || ((m_creature->GetHealth() - uiDamage) * 100 / m_creature->GetMaxHealth() < 15))
        {
            uiDamage = 0;

            if (pDoneBy->GetTypeId() == TYPEID_PLAYER)
                ((Player*)pDoneBy)->AreaExploredOrEventHappens(QUEST_BEAT);

            EnterEvadeMode();
        }
    }
};

bool QuestAccept_npc_bartleby(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_BEAT)
    {
        pCreature->SetFactionTemporary(FACTION_ENEMY, TEMPFACTION_RESTORE_RESPAWN | TEMPFACTION_RESTORE_COMBAT_STOP);
        pCreature->AI()->AttackStart(pPlayer);
    }
    return true;
}

CreatureAI* GetAI_npc_bartleby(Creature* pCreature)
{
    return new npc_bartlebyAI(pCreature);
}

/*######
## npc_dashel_stonefist
######*/

enum
{
    QUEST_MISSING_DIPLO_PT8     = 1447,
    FACTION_HOSTILE             = 168
};

struct npc_dashel_stonefistAI : public ScriptedAI
{
    npc_dashel_stonefistAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    void Reset() override {}

    void AttackedBy(Unit* pAttacker) override
    {
        if (m_creature->getVictim())
            return;

        if (m_creature->IsFriendlyTo(pAttacker))
            return;

        AttackStart(pAttacker);
    }

    void DamageTaken(Unit* pDoneBy, uint32& uiDamage, DamageEffectType /*damagetype*/) override
    {
        if (uiDamage > m_creature->GetHealth() || ((m_creature->GetHealth() - uiDamage) * 100 / m_creature->GetMaxHealth() < 15))
        {
            uiDamage = 0;

            if (pDoneBy->GetTypeId() == TYPEID_PLAYER)
                ((Player*)pDoneBy)->AreaExploredOrEventHappens(QUEST_MISSING_DIPLO_PT8);

            EnterEvadeMode();
        }
    }
};

bool QuestAccept_npc_dashel_stonefist(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_MISSING_DIPLO_PT8)
    {
        pCreature->SetFactionTemporary(FACTION_HOSTILE, TEMPFACTION_RESTORE_COMBAT_STOP | TEMPFACTION_RESTORE_RESPAWN);
        pCreature->AI()->AttackStart(pPlayer);
    }
    return true;
}

CreatureAI* GetAI_npc_dashel_stonefist(Creature* pCreature)
{
    return new npc_dashel_stonefistAI(pCreature);
}

/*######
## npc_lady_katrana_prestor
######*/

#define GOSSIP_ITEM_KAT_1 "Pardon the intrusion, Lady Prestor, but Highlord Bolvar suggested that I seek your advice."
#define GOSSIP_ITEM_KAT_2 "My apologies, Lady Prestor."
#define GOSSIP_ITEM_KAT_3 "Begging your pardon, Lady Prestor. That was not my intent."
#define GOSSIP_ITEM_KAT_4 "Thank you for your time, Lady Prestor."

bool GossipHello_npc_lady_katrana_prestor(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestStatus(4185) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KAT_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    pPlayer->SEND_GOSSIP_MENU(2693, pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_lady_katrana_prestor(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KAT_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            pPlayer->SEND_GOSSIP_MENU(2694, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KAT_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(2695, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KAT_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(2696, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(4185);
            break;
    }
    return true;
}

/*######
## Quest #434 The Attack
######*/

enum QuestTheAttack
{
    QUEST_ITEMS_OF_SOME_CONSEQUENCE = 2746, // prequest for 434
    QUEST_THE_ATTACK                = 434,

    FACTION_ENEMYY                  = 14,
    FACTION_NORMAL_LESCOVAR         = 12,
    FACTION_NORMAL_MARZON           = 84,

    NPC_LORD_GREGOR_LESCOVAR        = 1754,
    NPC_MARZON_THE_SILENT_BLADE     = 1755,
    NPC_STORMWIND_ROYAL_GUARD       = 1756,
    NPC_PRIESTRESS                  = 7779,
    NPC_TYRION                      = 7766,
    NPC_TYRIONS_SPYBOT              = 8856,

    MODEL_TYRIANA                   = 6703,
    MODEL_SPYBOT                    = 1159,

    GOSSIP_TEXT_ID_EVENT_RUNNING    = 2394,

    SAY_SPYBOT                      = -1700000,
    SAY_TYRION_1                    = -1700001,
    SAY_TYRIONA_1                   = -1700002,
    SAY_ROYAL_GUARD_1               = -1700003,
    SAY_TYRIONA_2                   = -1700004,
    SAY_TYRIONA_3                   = -1700005,
    SAY_GREGOR_1                    = -1700006,
    SAY_TYRIONA_4                   = -1700007,
    SAY_GREGOR_2                    = -1700008,
    SAY_ROYAL_GUARD_2               = -1700009,
    SAY_GREGOR_3                    = -1700010,
    SAY_GREGOR_4                    = -1700011,
    SAY_MARZON_1                    = -1700012,
    SAY_GREGOR_5                    = -1700013,
    SAY_MARZON_2                    = -1700014,
    SAY_TYRION_2                    = -1700015,
    SAY_GREGOR_6                    = -1700016,
    SAY_MARZON_3                    = -1700017,

    SPELL_STEALTH                   = 8874, // for Marzon
};


/*######
## npc_tyrion
######*/
struct npc_tyrionAI : public ScriptedAI
{
    npc_tyrionAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }

    bool m_IsEventRunning;

    void GetAIInformation(ChatHandler& reader)
    {
        ScriptedAI::GetAIInformation(reader);
        reader.PSendSysMessage("TYRION: Event running: [%s]", m_IsEventRunning ? "YES" : "NO");
    }

    void Reset() override
    {
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        m_IsEventRunning = false;
    }

    bool AreCreaturesRequiredForQuestPresent(float fMaxSearchRange = 40.0f)
    {
        // m_guidTyrion Spybot
        if (!GetClosestCreatureWithEntry(m_creature, NPC_TYRIONS_SPYBOT, VISIBLE_RANGE))
            return false;
        return true;
    }

};

CreatureAI* GetAI_npc_tyrion(Creature* pCreature)
{
    return new npc_tyrionAI(pCreature);
}


/*######
## npc_lord_gregor_lescovar
######*/
struct npc_lord_gregor_lescovarAI : public npc_escortAI
{
    npc_lord_gregor_lescovarAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        Reset();
    }

    uint32 m_uiEventTimer;
    uint8 m_uiEventPhase;
    ObjectGuid m_guidGuard1;
    ObjectGuid m_guidGuard2;
    ObjectGuid m_guidMarzon;
    ObjectGuid m_guidTyrion;
    ObjectGuid m_guidPriestress;

    void Reset()
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
            return;

        m_uiEventTimer = 0;
        m_uiEventPhase = 0;
    }

    void JustDied(Unit* /*pKiller*/)
    {
        if (m_creature->getFaction() == FACTION_ENEMYY)
            m_creature->setFaction(FACTION_NORMAL_LESCOVAR);

        if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
            if (!pMarzon->isAlive())
            {
                if (Creature* pGuard1 = m_creature->GetMap()->GetCreature(m_guidGuard1))
                {
                    pGuard1->ForcedDespawn(0);
                    pGuard1->Respawn();
                }
                if (Creature* pGuard2 = m_creature->GetMap()->GetCreature(m_guidGuard2))
                {
                    pGuard2->ForcedDespawn(0);
                    pGuard2->Respawn();
                }
                if (Creature* pPriestress = m_creature->GetMap()->GetCreature(m_guidPriestress))
                    pPriestress->Respawn();

                if (Creature* pTyrion = m_creature->GetMap()->GetCreature(m_guidTyrion))
                {
                    pTyrion->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    if (npc_tyrionAI* ptyrionAI = dynamic_cast<npc_tyrionAI*>(pTyrion->AI()))
                        ptyrionAI->Reset();
                }
            }
    }

    void SummonedCreatureJustDied(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() != NPC_MARZON_THE_SILENT_BLADE)
            return;

        if (!m_creature->isAlive())
        {
            if (Creature* pGuard1 = m_creature->GetMap()->GetCreature(m_guidGuard1))
            {
                pGuard1->ForcedDespawn(0);
                pGuard1->Respawn();
            }
            if (Creature* pGuard2 = m_creature->GetMap()->GetCreature(m_guidGuard2))
            {
                pGuard2->ForcedDespawn(0);
                pGuard2->Respawn();
            }
            if (Creature* pPriestress = m_creature->GetMap()->GetCreature(m_guidPriestress))
                pPriestress->Respawn();

            if (Creature* pTyrion = m_creature->GetMap()->GetCreature(m_guidTyrion))
            {
                pTyrion->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                if (npc_tyrionAI* ptyrionAI = dynamic_cast<npc_tyrionAI*>(pTyrion->AI()))
                    ptyrionAI->m_IsEventRunning = false;
            }
        }
    }

    void Aggro(Unit* pAttacker)
    {
        if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
        {
            DoScriptText(SAY_MARZON_3, pMarzon);
            DoScriptText(SAY_GREGOR_6, m_creature);
            pMarzon->AI()->AttackStart(pAttacker);
        }
        m_uiEventPhase = 13;
    }

    void SummonedCreatureDespawn(Creature* pSummoned)
    {
        if (pSummoned->GetEntry() != NPC_MARZON_THE_SILENT_BLADE)
            return;

        if (Creature* pGuard1 = m_creature->GetMap()->GetCreature(m_guidGuard1))
        {
            pGuard1->ForcedDespawn(0);
            pGuard1->Respawn();
        }

        if (Creature* pGuard2 = m_creature->GetMap()->GetCreature(m_guidGuard2))
        {
            pGuard2->ForcedDespawn(0);
            pGuard2->Respawn();
        }

        if (Creature* pPriestress = m_creature->GetMap()->GetCreature(m_guidPriestress))
            pPriestress->Respawn();

        if (Creature* pTyrion = m_creature->GetMap()->GetCreature(m_guidTyrion))
        {
            pTyrion->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            if (npc_tyrionAI* ptyrionAI = dynamic_cast<npc_tyrionAI*>(pTyrion->AI()))
                ptyrionAI->m_IsEventRunning = false;
        }

    }

    void WaypointReached(uint32 uiPoint)
    {
        Player* pPlayer = GetPlayerForEscort();
        if (!pPlayer)
            return;

        switch (uiPoint)
        {
        case 13:
            SetEscortPaused(false);
            if (Creature* pGuard1 = m_creature->GetMap()->GetCreature(m_guidGuard1))
              if (pGuard1->isAlive())
                  pGuard1->SetFacingToObject(m_creature);
            if (Creature* pGuard2 = m_creature->GetMap()->GetCreature(m_guidGuard2))
              if (pGuard2->isAlive())
                  pGuard2->SetFacingToObject(m_creature);
            DoScriptText(SAY_GREGOR_2, m_creature);
            m_uiEventPhase = 1;
            m_uiEventTimer = 3500;
            SetEscortPaused(true);
            break;
        case 17:
            m_uiEventPhase = 4;
            m_uiEventTimer = 1500;
            SetEscortPaused(true);
            break;
        case 20:
            m_creature->setFaction(FACTION_NORMAL_LESCOVAR);
            if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
            {
                pMarzon->CastSpell(pMarzon, SPELL_STEALTH, true);
                pMarzon->ForcedDespawn(0);
            }
            break;
        case 21:
            if (Creature* pGuard1 = m_creature->GetMap()->GetCreature(m_guidGuard1))
            {
                pGuard1->ForcedDespawn(0);
                pGuard1->Respawn();
            }
            if (Creature* pGuard2 = m_creature->GetMap()->GetCreature(m_guidGuard2))
            {
                pGuard2->ForcedDespawn(0);
                pGuard2->Respawn();
            }
            if (Creature* pPriestress = m_creature->GetMap()->GetCreature(m_guidPriestress))
                pPriestress->Respawn();

            if (Creature* pTyrion = m_creature->GetMap()->GetCreature(m_guidTyrion))
            {
                pTyrion->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                if (npc_tyrionAI* ptyrionAI = dynamic_cast<npc_tyrionAI*>(pTyrion->AI()))
                    ptyrionAI->m_IsEventRunning = false;
            }

            if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
                if (!pMarzon->isInCombat())
                    pMarzon->ForcedDespawn(0);
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (m_uiEventTimer)
        {
            if (m_uiEventTimer <= uiDiff)
            {
                switch (m_uiEventPhase)
                {
                case 1:
                    if (Creature* pGuard1 = m_creature->GetMap()->GetCreature(m_guidGuard1))
                        if (pGuard1->isAlive() && !pGuard1->getVictim())
                        {
                            DoScriptText(SAY_ROYAL_GUARD_2, pGuard1);
                            pGuard1->GetMotionMaster()->MovePoint(0, -8364.07f, 406.775f, 122.274f, MOVE_PATHFINDING);
                        }
                    if (Creature* pGuard2 = m_creature->GetMap()->GetCreature(m_guidGuard2))
                        if (pGuard2->isAlive() && !pGuard2->getVictim())
                        {
                            DoScriptText(SAY_ROYAL_GUARD_2, pGuard2);
                            pGuard2->GetMotionMaster()->MovePoint(0, -8353.91f, 415.318f, 122.274f, MOVE_PATHFINDING);
                        }
                    ++m_uiEventPhase;
                    m_uiEventTimer = 3000;
                    break;
                case 2:
                    ++m_uiEventPhase;
                    SetEscortPaused(false);
                    break;
                case 4:
                    if (Creature* pMarzon = m_creature->SummonCreature(NPC_MARZON_THE_SILENT_BLADE, -8407.71f, 482.117f, 123.76f, 4.79f, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 1 * MINUTE*IN_MILLISECONDS))
                    {
                        pMarzon->CastSpell(pMarzon, SPELL_STEALTH, true);
                        pMarzon->GetMotionMaster()->MovePoint(0, -8406.00f, 470.00f, 123.76f, MOVE_PATHFINDING);
                        m_guidMarzon = pMarzon->GetObjectGuid();
                    }
                    m_uiEventTimer = 10000;
                    ++m_uiEventPhase;
                    SetEscortPaused(true);
                    break;
                case 5:
                    DoScriptText(SAY_GREGOR_3, m_creature);
                    m_uiEventTimer = 5000;
                    ++m_uiEventPhase;
                    break;
                case 6:

                    if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
                    {
                        pMarzon->RemoveAura(SPELL_STEALTH, EFFECT_INDEX_0);
                        m_creature->SetFacingToObject(pMarzon);
                    }
                    DoScriptText(SAY_GREGOR_4, m_creature);
                    m_uiEventTimer = 5000;
                    ++m_uiEventPhase;
                    break;
                case 7:
                    if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
                        DoScriptText(SAY_MARZON_1, pMarzon);
                    m_uiEventTimer = 5000;
                    ++m_uiEventPhase;
                    break;
                case 8:
                    DoScriptText(SAY_GREGOR_5, m_creature);
                    m_uiEventTimer = 4000;
                    ++m_uiEventPhase;
                    break;
                case 9:
                    if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
                        DoScriptText(SAY_MARZON_2, pMarzon);
                    m_uiEventTimer = 3000;
                    ++m_uiEventPhase;
                    break;
                case 10:
                    if (Player* pPlayer = GetPlayerForEscort())
                        pPlayer->GroupEventHappens(QUEST_THE_ATTACK, m_creature);
                    if (Creature* pTyrion = m_creature->GetMap()->GetCreature(m_guidTyrion))
                        DoScriptText(SAY_TYRION_2, pTyrion);
                    if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
                        pMarzon->setFaction(FACTION_ENEMYY);
                    m_creature->setFaction(FACTION_ENEMYY);
                    m_uiEventTimer = 5000;
                    ++m_uiEventPhase;
                    break;
                case 11:
                    if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
                    {
                        if (!m_creature->isInCombat())
                            pMarzon->GetMotionMaster()->MovePoint(0, -8419.00f, 486.26f, 123.75f, MOVE_PATHFINDING);
                        else if (m_creature->getVictim())
                            pMarzon->AI()->AttackStart(m_creature->getVictim());
                    }
                    ++m_uiEventPhase;
                    SetEscortPaused(false);
                    break;
                case 12:
                    if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
                      if (pMarzon->isInCombat() && pMarzon->getVictim() && !m_creature->isInCombat())
                          m_creature->AI()->AttackStart(pMarzon->getVictim());
                    break;
                case 13:
                    if (Player* pPlayer = GetPlayerForEscort())
                        if (pPlayer->isDead())
                        {
                            SetEscortPaused(false);
                            m_creature->setFaction(FACTION_NORMAL_LESCOVAR);
                            if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
                                if (!pMarzon->isDead() && pMarzon->getFaction() == FACTION_ENEMYY)
                                    pMarzon->setFaction(FACTION_NORMAL_MARZON);
                        }
                    if (!m_creature->isInCombat())
                        SetEscortPaused(false);
                    break;
                }
            }
            else
                m_uiEventTimer -= uiDiff;
        }

        if (Creature* pMarzon = m_creature->GetMap()->GetCreature(m_guidMarzon))
          if (pMarzon && pMarzon->isDead() && pMarzon->getFaction() == FACTION_ENEMYY)
              pMarzon->setFaction(FACTION_NORMAL_MARZON);

        npc_escortAI::UpdateAI(uiDiff);

        if (m_creature->SelectHostileTarget() || m_creature->getVictim())
            DoMeleeAttackIfReady();
    }
};


CreatureAI* GetAI_npc_lord_gregor_lescovar(Creature* pCreature)
{
    return new npc_lord_gregor_lescovarAI(pCreature);
}


/*######
## npc_tyrion_spybot
######*/

struct npc_tyrion_spybotAI : public npc_escortAI
{
    npc_tyrion_spybotAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        Reset();
    }

    ObjectGuid m_guidGuard1;
    ObjectGuid m_guidGuard2;
    ObjectGuid m_guidLordGregor;
    ObjectGuid m_guidTyrion;
    ObjectGuid m_guidMarzon;
    ObjectGuid m_guidPriestress;
    uint8 m_uiGardenGuardsCounter;

    uint32 m_uiEventTimer;
    uint8 m_uiEventPhase;
    float m_fDefaultScaleSize;

    void Reset()
    {

        if (HasEscortState(STATE_ESCORT_ESCORTING))
            return;

        m_fDefaultScaleSize = 0.30f;
        m_uiEventTimer = 0;
        m_uiEventPhase = 0;
        m_uiGardenGuardsCounter = 0;
        m_creature->SetDisplayId(MODEL_SPYBOT);

        if (Creature* tyrion = GetClosestCreatureWithEntry(m_creature, NPC_TYRION, VISIBLE_RANGE))
        {
            if (npc_tyrionAI* ptyrionAI = dynamic_cast<npc_tyrionAI*>(tyrion->AI()))
            {
                ptyrionAI->m_IsEventRunning = false;
                tyrion->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            }
        }
    }

    void Aggro(Unit* pAttacker) {}


    bool AreCreaturesRequiredForQuestPresent(float fMaxSearchRange = 40.0f)
    {
        if (m_guidTyrion && m_guidLordGregor && m_guidGuard1 && m_guidGuard2)
            return true;

        // m_guidPriestress
        if (!m_guidPriestress)
            if (Creature* pPriestress = GetClosestCreatureWithEntry(m_creature, NPC_PRIESTRESS, VISIBLE_RANGE))
            {
                if (!pPriestress)
                    return false;
                else if (pPriestress->GetEntry() != NPC_PRIESTRESS)
                    return false;
                else
                    m_guidPriestress = pPriestress->GetObjectGuid();
            }

        // m_guidTyrion
        if (!m_guidTyrion)
            if (Creature* pTyrion = GetClosestCreatureWithEntry(m_creature, NPC_TYRION, VISIBLE_RANGE))
            {
                if (!pTyrion)
                    return false;
                else if (pTyrion->GetEntry() != NPC_TYRION)
                    return false;
                else
                    m_guidTyrion = pTyrion->GetObjectGuid();
            }

        // Lord Gregor
        if (!m_guidLordGregor)
            if (Creature* pGregor = GetClosestCreatureWithEntry(m_creature, NPC_LORD_GREGOR_LESCOVAR, VISIBLE_RANGE))
            {
                if (!pGregor)
                    return false;
                else if (!pGregor->isAlive())
                    pGregor->Respawn();
                m_guidLordGregor = pGregor->GetObjectGuid();
            }
        // Stormwind Royal Guards (Garden)
        if (!m_guidGuard1 || !m_guidGuard2)
        {
            std::list<Creature*> lGardenStormwindRoyalGuards;
            GetCreatureListWithEntryInGrid(lGardenStormwindRoyalGuards, m_creature, NPC_STORMWIND_ROYAL_GUARD, fMaxSearchRange);

            if (lGardenStormwindRoyalGuards.empty())
                return true;

            for (std::list<Creature*>::iterator iter = lGardenStormwindRoyalGuards.begin(); iter != lGardenStormwindRoyalGuards.end(); ++iter)
            {
                Creature* GardenStormwindRoyalGuard = (*iter);
                if (GardenStormwindRoyalGuard->isDead() || !GardenStormwindRoyalGuard->isAlive())
                    GardenStormwindRoyalGuard->Respawn();

                if (m_uiGardenGuardsCounter == 0)
                {
                    m_guidGuard1 = GardenStormwindRoyalGuard->GetObjectGuid();
                    m_uiGardenGuardsCounter++;
                    continue;
                }
                else if (m_uiGardenGuardsCounter == 1)
                {
                    m_guidGuard2 = GardenStormwindRoyalGuard->GetObjectGuid();
                    m_uiGardenGuardsCounter++;
                    continue;
                }
            }
            return true;
        }
        return true;
    }

    void WaypointReached(uint32 uiPoint)
    {
        if (!AreCreaturesRequiredForQuestPresent())
            return;

        switch (uiPoint)
        {
        case 1:
            if (Creature* pTyrion = m_creature->GetMap()->GetCreature(m_guidTyrion))
            {
                pTyrion->SetFacingToObject(m_creature);
                pTyrion->HandleEmote(EMOTE_STATE_USESTANDING);
            }
            m_fDefaultScaleSize = m_creature->GetFloatValue(OBJECT_FIELD_SCALE_X);
            m_creature->SetDisplayId(MODEL_TYRIANA);
            m_creature->SetFloatValue(OBJECT_FIELD_SCALE_X, 1.00f);

            SetEscortPaused(true);
            m_uiEventTimer = 5000;
            m_uiEventPhase = 1;
            break;
        case 2:
            if (Creature* pPriestress = m_creature->GetMap()->GetCreature(m_guidPriestress))
                pPriestress->ForcedDespawn(0);
            if (Creature* pTyrion = m_creature->GetMap()->GetCreature(m_guidTyrion))
                DoScriptText(SAY_TYRION_1, pTyrion, GetPlayerForEscort());
            break;
        case 6:
            DoScriptText(SAY_TYRIONA_1, m_creature);
            if (Creature* pGuard1 = m_creature->GetMap()->GetCreature(m_guidGuard1))
            {
                if (pGuard1 && pGuard1->isAlive() && !pGuard1->getVictim())
                    pGuard1->SetFacingToObject(m_creature);
            }

            if (Creature* pGuard2 = m_creature->GetMap()->GetCreature(m_guidGuard2))
            {
                if (pGuard2 && pGuard2->isAlive() && !pGuard2->getVictim())
                    pGuard2->SetFacingToObject(m_creature);
            }

            m_uiEventPhase = 2;
            m_uiEventTimer = 5000;
            SetEscortPaused(true);
            break;
        case 18:
            DoScriptText(SAY_TYRIONA_3, m_creature);
            if (Creature* gregor = m_creature->GetMap()->GetCreature(m_guidLordGregor))
                gregor->SetFacingToObject(m_creature);
            m_uiEventPhase = 5;
            m_uiEventTimer = 5000;
            SetEscortPaused(true);
            break;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {

        if (m_uiEventTimer)
        {
            if (m_uiEventTimer <= uiDiff)
            {
                switch (m_uiEventPhase)
                {
                case 1:
                    if (Creature* pTyrion = m_creature->GetMap()->GetCreature(m_guidTyrion))
                        pTyrion->HandleEmote(EMOTE_ONESHOT_NONE);
                    SetEscortPaused(false);
                    break;
                case 2:
                    if (Creature* pGuard1 = m_creature->GetMap()->GetCreature(m_guidGuard1))
                    {
                        DoScriptText(SAY_ROYAL_GUARD_1, pGuard1);
                        m_creature->SetFacingToObject(pGuard1);
                    }
                    m_uiEventTimer = 5000;
                    ++m_uiEventPhase;
                    break;
                case 3:
                    DoScriptText(SAY_TYRIONA_2, m_creature);
                    if (Creature* pGuard1 = m_creature->GetMap()->GetCreature(m_guidGuard1))
                        pGuard1->HandleEmote(EMOTE_ONESHOT_KNEEL);
                    if (Creature* pGuard2 = m_creature->GetMap()->GetCreature(m_guidGuard2))
                        pGuard2->HandleEmote(EMOTE_ONESHOT_KNEEL);
                    ++m_uiEventPhase; // 4 = nothing (It's OK)
                    SetEscortPaused(false);
                    break;
                case 5:
                    if (Creature* gregor = m_creature->GetMap()->GetCreature(m_guidLordGregor))
                        DoScriptText(SAY_GREGOR_1, gregor);
                    m_uiEventTimer = 5000;
                    ++m_uiEventPhase;
                    break;
                case 6:
                    DoScriptText(SAY_TYRIONA_4, m_creature);
                    m_uiEventTimer = 5000;
                    ++m_uiEventPhase;
                    break;
                case 7:
                    if (Player* pPlayer = GetPlayerForEscort())
                        if (Creature* gregor = m_creature->GetMap()->GetCreature(m_guidLordGregor))
                            if (npc_lord_gregor_lescovarAI* pGregorEscortAI = dynamic_cast<npc_lord_gregor_lescovarAI*>(gregor->AI()))
                            {
                                pGregorEscortAI->SetMaxPlayerDistance(200.0f);
                                pGregorEscortAI->Start(false, pPlayer->GetGUID());
                                pGregorEscortAI->m_guidGuard1 = m_guidGuard1;
                                pGregorEscortAI->m_guidGuard2 = m_guidGuard2;
                                pGregorEscortAI->m_guidTyrion = m_guidTyrion;
                                pGregorEscortAI->m_guidPriestress = m_guidPriestress;
                            }
                    m_creature->SetDisplayId(MODEL_SPYBOT);
                    m_creature->SetFloatValue(OBJECT_FIELD_SCALE_X, m_fDefaultScaleSize);
                    ++m_uiEventPhase;
                    m_uiEventTimer = 1000;
                    break;
                case 8:
                    SetEscortPaused(false);
                    break;
                }
            }
            else m_uiEventTimer -= uiDiff;
        }

        npc_escortAI::UpdateAI(uiDiff);
    }
};

CreatureAI* GetAI_npc_tyrion_spybot(Creature* pCreature)
{
    return new npc_tyrion_spybotAI(pCreature);
}

/*######
## npc_tyrion
######*/

bool GossipHello_npc_tyrion(Player* pPlayer, Creature* pCreature)
{
    bool bIsEventReady = false;

    if (npc_tyrionAI* pTyrion = dynamic_cast<npc_tyrionAI*>(pCreature->AI()))
        bIsEventReady =  (!pTyrion->m_IsEventRunning && pTyrion->AreCreaturesRequiredForQuestPresent());

    // Check if event is possible and also check the status of the quests
    if (!bIsEventReady)
    {
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_EVENT_RUNNING, pCreature->GetObjectGuid());
    }
    else
    {
        if (pCreature->isQuestGiver())
            pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

        pPlayer->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
    }

    return true;
}


bool QuestAccept_npc_tyrion(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (npc_tyrionAI* ptyrionAI = dynamic_cast<npc_tyrionAI*>(pCreature->AI()))
        if (ptyrionAI->m_IsEventRunning)
            return false;

    if (pQuest->GetQuestId() == QUEST_THE_ATTACK)
    {
        if (Creature* TyrionSpyBot = GetClosestCreatureWithEntry(pCreature, NPC_TYRIONS_SPYBOT, VISIBLE_RANGE))
        {
            if (npc_tyrion_spybotAI* pSpybotEscortAI = dynamic_cast<npc_tyrion_spybotAI*>(TyrionSpyBot->AI()))
            {
                DoScriptText(SAY_SPYBOT, TyrionSpyBot);
                pSpybotEscortAI->SetMaxPlayerDistance(200.0f);
                pSpybotEscortAI->Start(false, pPlayer->GetGUID(), nullptr, false, false);
                pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                if (npc_tyrionAI* ptyrionAI = dynamic_cast<npc_tyrionAI*>(pCreature->AI()))
                    ptyrionAI->m_IsEventRunning = true;
            }
        }
    }
    return true;
}

/*
 *
 */


/*######
## npc_squire_rowe
######*/

enum
{
    SAY_SIGNAL_SENT             = -1000822,
    SAY_DISMOUNT                = -1000823,
    SAY_WELCOME                 = -1000824,

    GOSSIP_ITEM_WINDSOR         = -3000106,

    GOSSIP_TEXT_ID_DEFAULT      = 9063,
    GOSSIP_TEXT_ID_PROGRESS     = 9064,
    GOSSIP_TEXT_ID_START        = 9065,

    NPC_WINDSOR_MOUNT           = 12581,

    QUEST_STORMWIND_RENDEZVOUS  = 6402,
    QUEST_THE_GREAT_MASQUERADE  = 6403,
};

static const DialogueEntry aIntroDialogue[] =
{
    {NPC_WINDSOR,                0,           3000},        // wait
    {NPC_WINDSOR_MOUNT,          0,           1000},        // summon horse
    {SAY_DISMOUNT,               NPC_WINDSOR, 2000},
    {QUEST_STORMWIND_RENDEZVOUS, 0,           2000},        // face player
    {QUEST_THE_GREAT_MASQUERADE, 0,           0},           // say intro to player
    {0, 0, 0},
};

static const float aWindsorSpawnLoc[3] = { -9145.68f, 373.79f, 90.64f};
static const float aWindsorMoveLoc[3] = { -9050.39f, 443.55f, 93.05f};

struct npc_squire_roweAI : public npc_escortAI, private DialogueHelper
{
    npc_squire_roweAI(Creature* m_creature) : npc_escortAI(m_creature),
        DialogueHelper(aIntroDialogue)
    {
        m_bIsEventInProgress = false;
        Reset();
    }

    bool m_bIsEventInProgress;

    ObjectGuid m_windsorGuid;
    ObjectGuid m_horseGuid;

    void Reset() override { }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_WINDSOR)
        {
            pSummoned->SetWalk(false);
            pSummoned->GetMotionMaster()->MovePoint(1, aWindsorMoveLoc[0], aWindsorMoveLoc[1], aWindsorMoveLoc[2]);

            m_windsorGuid = pSummoned->GetObjectGuid();
            m_bIsEventInProgress = true;
        }
        else if (pSummoned->GetEntry() == NPC_WINDSOR_MOUNT)
            m_horseGuid = pSummoned->GetObjectGuid();
    }

    void SummonedCreatureDespawn(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_WINDSOR)
        {
            m_windsorGuid.Clear();
            m_bIsEventInProgress = false;
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiMotionType, uint32 uiPointId) override
    {
        if (uiMotionType != POINT_MOTION_TYPE || !uiPointId || pSummoned->GetEntry() != NPC_WINDSOR)
            return;

        // Summoned npc has escort and this can trigger twice if escort state is not checked
        if (uiPointId && HasEscortState(STATE_ESCORT_PAUSED))
            StartNextDialogueText(NPC_WINDSOR);
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 2:
                m_creature->SetStandState(UNIT_STAND_STATE_KNEEL);
                break;
            case 3:
                m_creature->SetStandState(UNIT_STAND_STATE_STAND);
                m_creature->SummonCreature(NPC_WINDSOR, aWindsorSpawnLoc[0], aWindsorSpawnLoc[1], aWindsorSpawnLoc[2], 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
                break;
            case 6:
                DoScriptText(SAY_SIGNAL_SENT, m_creature);
                m_creature->SetFacingTo(2.15f);
                SetEscortPaused(true);
                break;
        }
    }

    void JustDidDialogueStep(int32 iEntry) override
    {
        switch (iEntry)
        {
            case NPC_WINDSOR_MOUNT:
            {
                if (Creature* pWindsor = m_creature->GetMap()->GetCreature(m_windsorGuid))
                {
                    pWindsor->Unmount();
                    m_creature->SummonCreature(NPC_WINDSOR_MOUNT, pWindsor->GetPositionX() - 1.0f, pWindsor->GetPositionY() + 1.0f, pWindsor->GetPositionZ(), pWindsor->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 30000);
                }
                break;
            }
            case SAY_DISMOUNT:
            {
                if (Creature* pHorse = m_creature->GetMap()->GetCreature(m_horseGuid))
                {
                    pHorse->SetWalk(false);
                    pHorse->GetMotionMaster()->MovePoint(1, aWindsorSpawnLoc[0], aWindsorSpawnLoc[1], aWindsorSpawnLoc[2]);
                }
                break;
            }
            case QUEST_STORMWIND_RENDEZVOUS:
            {
                Creature* pWindsor = m_creature->GetMap()->GetCreature(m_windsorGuid);
                Player* pPlayer = GetPlayerForEscort();
                if (!pWindsor || !pPlayer)
                    break;

                pWindsor->SetFacingToObject(pPlayer);
                break;
            }
            case QUEST_THE_GREAT_MASQUERADE:
            {
                Creature* pWindsor = m_creature->GetMap()->GetCreature(m_windsorGuid);
                Player* pPlayer = GetPlayerForEscort();
                if (!pWindsor || !pPlayer)
                    break;

                DoScriptText(SAY_WELCOME, pWindsor, pPlayer);
                // Allow players to finish quest and also finish the escort
                pWindsor->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                SetEscortPaused(false);
                break;
            }
        }
    }

    Creature* GetSpeakerByEntry(uint32 uiEntry) override
    {
        if (uiEntry == NPC_WINDSOR)
            return m_creature->GetMap()->GetCreature(m_windsorGuid);

        return nullptr;
    }

    // Check if the event is already running
    bool IsStormwindQuestActive() { return m_bIsEventInProgress; }

    void UpdateEscortAI(const uint32 uiDiff) { DialogueUpdate(uiDiff); }
};

CreatureAI* GetAI_npc_squire_rowe(Creature* pCreature)
{
    return new npc_squire_roweAI(pCreature);
}

bool GossipHello_npc_squire_rowe(Player* pPlayer, Creature* pCreature)
{
    // Allow gossip if quest 6402 is completed but not yet rewarded or 6402 is rewarded but 6403 isn't yet completed
    if ((pPlayer->GetQuestStatus(QUEST_STORMWIND_RENDEZVOUS) == QUEST_STATUS_COMPLETE && !pPlayer->GetQuestRewardStatus(QUEST_STORMWIND_RENDEZVOUS)) ||
            (pPlayer->GetQuestRewardStatus(QUEST_STORMWIND_RENDEZVOUS) && pPlayer->GetQuestStatus(QUEST_THE_GREAT_MASQUERADE) != QUEST_STATUS_COMPLETE))
    {
        bool bIsEventInProgress = true;

        // Check if event is already in progress
        if (npc_squire_roweAI* pRoweAI = dynamic_cast<npc_squire_roweAI*>(pCreature->AI()))
            bIsEventInProgress = pRoweAI->IsStormwindQuestActive();

        // If event is already in progress, then inform the player to wait
        if (bIsEventInProgress)
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_PROGRESS, pCreature->GetObjectGuid());
        else
        {
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_WINDSOR, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_START, pCreature->GetObjectGuid());
        }
    }
    else
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_DEFAULT, pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_squire_rowe(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        if (npc_squire_roweAI* pRoweAI = dynamic_cast<npc_squire_roweAI*>(pCreature->AI()))
            pRoweAI->Start(true, pPlayer, 0, true, false);

        pPlayer->CLOSE_GOSSIP_MENU();
    }

    return true;
}

/*######
## npc_reginald_windsor
######*/

enum
{
    SAY_WINDSOR_QUEST_ACCEPT    = -1000825,
    SAY_WINDSOR_GET_READY       = -1000826,
    SAY_PRESTOR_SIEZE           = -1000827,

    SAY_JON_DIALOGUE_1          = -1000828,
    SAY_WINDSOR_DIALOGUE_2      = -1000829,
    SAY_WINDSOR_DIALOGUE_3      = -1000830,
    SAY_JON_DIALOGUE_4          = -1000832,
    SAY_JON_DIALOGUE_5          = -1000833,
    SAY_WINDSOR_DIALOGUE_6      = -1000834,
    SAY_WINDSOR_DIALOGUE_7      = -1000835,
    SAY_JON_DIALOGUE_8          = -1000836,
    SAY_JON_DIALOGUE_9          = -1000837,
    SAY_JON_DIALOGUE_10         = -1000838,
    SAY_JON_DIALOGUE_11         = -1000839,
    SAY_WINDSOR_DIALOGUE_12     = -1000840,
    SAY_WINDSOR_DIALOGUE_13     = -1000841,

    SAY_WINDSOR_BEFORE_KEEP     = -1000849,
    SAY_WINDSOR_TO_KEEP         = -1000850,

    SAY_WINDSOR_KEEP_1          = -1000851,
    SAY_BOLVAR_KEEP_2           = -1000852,
    SAY_WINDSOR_KEEP_3          = -1000853,
    SAY_PRESTOR_KEEP_4          = -1000855,
    SAY_PRESTOR_KEEP_5          = -1000856,
    SAY_WINDSOR_KEEP_6          = -1000857,
    SAY_WINDSOR_KEEP_7          = -1000859,
    SAY_WINDSOR_KEEP_8          = -1000860,
    SAY_PRESTOR_KEEP_9          = -1000863,
    SAY_BOLVAR_KEEP_10          = -1000864,
    SAY_PRESTOR_KEEP_11         = -1000865,
    SAY_WINDSOR_KEEP_12         = -1000866,
    SAY_PRESTOR_KEEP_13         = -1000867,
    SAY_PRESTOR_KEEP_14         = -1000868,
    SAY_BOLVAR_KEEP_15          = -1000869,
    SAY_WINDSOR_KEEP_16         = -1000870,

    EMOTE_CONTEMPLATION         = -1000831,
    EMOTE_PRESTOR_LAUGH         = -1000854,
    EMOTE_WINDSOR_TABLETS       = -1000858,
    EMOTE_WINDSOR_READ          = -1000861,
    EMOTE_BOLVAR_GASP           = -1000862,
    EMOTE_WINDSOR_DIE           = -1000871,
    EMOTE_GUARD_TRANSFORM       = -1000872,

    GOSSIP_ITEM_REGINALD        = -3000107,

    GOSSIP_TEXT_ID_MASQUERADE   = 5633,

    // SPELL_ONYXIA_TRANSFORM    = 20409,            // removed from DBC
    SPELL_WINDSOR_READ          = 20358,
    SPELL_WINDSOR_DEATH         = 20465,
    SPELL_ONYXIA_DESPAWN        = 20466,

    // combat spells
    SPELL_HAMMER_OF_JUSTICE     = 10308,
    SPELL_SHIELD_WALL           = 871,
    SPELL_STRONG_CLEAVE         = 8255,

    NPC_GUARD_ROYAL             = 1756,
    NPC_GUARD_CITY              = 68,
    NPC_GUARD_PATROLLER         = 1976,
    NPC_GUARD_ONYXIA            = 12739,
    NPC_LADY_ONYXIA             = 12756,

    MAX_ROYAL_GUARDS            = 6,
    MAX_GUARD_SALUTES           = 7,
};

static const float aGuardLocations[MAX_ROYAL_GUARDS][4] =
{
    { -8968.510f, 512.556f, 96.352f, 3.849f},               // guard right - left
    { -8969.780f, 515.012f, 96.593f, 3.955f},               // guard right - middle
    { -8972.410f, 518.228f, 96.594f, 4.281f},               // guard right - right
    { -8965.170f, 508.565f, 96.352f, 3.825f},               // guard left - right
    { -8962.960f, 506.583f, 96.593f, 3.802f},               // guard left - middle
    { -8961.080f, 503.828f, 96.593f, 3.465f},               // guard left - left
};

static const float aMoveLocations[10][3] =
{
    { -8967.960f, 510.008f, 96.351f},                       // Jonathan move
    { -8959.440f, 505.424f, 96.595f},                       // Guard Left - Middle kneel
    { -8957.670f, 507.056f, 96.595f},                       // Guard Left - Right kneel
    { -8970.680f, 519.252f, 96.595f},                       // Guard Right - Middle kneel
    { -8969.100f, 520.395f, 96.595f},                       // Guard Right - Left kneel
    { -8974.590f, 516.213f, 96.590f},                       // Jonathan kneel
    { -8505.770f, 338.312f, 120.886f},                      // Wrynn safe
    { -8448.690f, 337.074f, 121.330f},                      // Bolvar help
    { -8448.279f, 338.398f, 121.329f}                       // Bolvar kneel
};

static const DialogueEntry aMasqueradeDialogue[] =
{
    {SAY_WINDSOR_QUEST_ACCEPT,  NPC_WINDSOR,    7000},
    {SAY_WINDSOR_GET_READY,     NPC_WINDSOR,    6000},
    {SAY_PRESTOR_SIEZE,         NPC_PRESTOR,    0},

    {SAY_JON_DIALOGUE_1,        NPC_JONATHAN,   5000},
    {SAY_WINDSOR_DIALOGUE_2,    NPC_WINDSOR,    6000},
    {SAY_WINDSOR_DIALOGUE_3,    NPC_WINDSOR,    5000},
    {EMOTE_CONTEMPLATION,       NPC_JONATHAN,   3000},
    {SAY_JON_DIALOGUE_4,        NPC_JONATHAN,   6000},
    {SAY_JON_DIALOGUE_5,        NPC_JONATHAN,   7000},
    {SAY_WINDSOR_DIALOGUE_6,    NPC_WINDSOR,    8000},
    {SAY_WINDSOR_DIALOGUE_7,    NPC_WINDSOR,    6000},
    {SAY_JON_DIALOGUE_8,        NPC_JONATHAN,   7000},
    {SAY_JON_DIALOGUE_9,        NPC_JONATHAN,   6000},
    {SAY_JON_DIALOGUE_10,       NPC_JONATHAN,   5000},
    {EMOTE_ONESHOT_SALUTE,      0,              4000},
    {SAY_JON_DIALOGUE_11,       NPC_JONATHAN,   3000},
    {NPC_JONATHAN,              0,              2000},
    {EMOTE_ONESHOT_KNEEL,       0,              3000},
    {SAY_WINDSOR_DIALOGUE_12,   NPC_WINDSOR,    5000},
    {SAY_WINDSOR_DIALOGUE_13,   NPC_WINDSOR,    3000},
    {EMOTE_ONESHOT_POINT,       0,              3000},
    {NPC_WINDSOR,               0,              0},

    {NPC_GUARD_ROYAL,           0,              3000},
    {SAY_WINDSOR_BEFORE_KEEP,   NPC_WINDSOR,    0},
    {SAY_WINDSOR_TO_KEEP,       NPC_WINDSOR,    4000},
    {NPC_GUARD_CITY,            0,              0},

    {NPC_WRYNN,                 0,              3000},
    {SAY_WINDSOR_KEEP_1,        NPC_WINDSOR,    3000},
    {SAY_BOLVAR_KEEP_2,         NPC_BOLVAR,     2000},
    {SAY_WINDSOR_KEEP_3,        NPC_WINDSOR,    4000},
    {EMOTE_PRESTOR_LAUGH,       NPC_PRESTOR,    4000},
    {SAY_PRESTOR_KEEP_4,        NPC_PRESTOR,    9000},
    {SAY_PRESTOR_KEEP_5,        NPC_PRESTOR,    7000},
    {SAY_WINDSOR_KEEP_6,        NPC_WINDSOR,    6000},
    {EMOTE_WINDSOR_TABLETS,     NPC_WINDSOR,    6000},
    {SAY_WINDSOR_KEEP_7,        NPC_WINDSOR,    4000},
    {SAY_WINDSOR_KEEP_8,        NPC_WINDSOR,    5000},
    {EMOTE_WINDSOR_READ,        NPC_WINDSOR,    3000},
    {SPELL_WINDSOR_READ,        0,              10000},
    {EMOTE_BOLVAR_GASP,         NPC_BOLVAR,     3000},
    {SAY_PRESTOR_KEEP_9,        NPC_PRESTOR,    4000},
    {SAY_BOLVAR_KEEP_10,        NPC_BOLVAR,     3000},
    {SAY_PRESTOR_KEEP_11,       NPC_PRESTOR,    2000},
    {SPELL_WINDSOR_DEATH,       0,              1500},
    {SAY_WINDSOR_KEEP_12,       NPC_WINDSOR,    4000},
    {SAY_PRESTOR_KEEP_14,       NPC_PRESTOR,    0},

    {NPC_GUARD_ONYXIA,          0,              14000},
    {NPC_BOLVAR,                0,              2000},
    {SAY_BOLVAR_KEEP_15,        NPC_BOLVAR,     8000},
    {NPC_GUARD_PATROLLER,       0,              0},
    {0, 0, 0},
};

static const int32 aGuardSalute[MAX_GUARD_SALUTES] = { -1000842, -1000843, -1000844, -1000845, -1000846, -1000847, -1000848};

struct npc_reginald_windsorAI : public npc_escortAI, private DialogueHelper
{
    npc_reginald_windsorAI(Creature* m_creature) : npc_escortAI(m_creature),
        DialogueHelper(aMasqueradeDialogue)
    {
        m_pScriptedMap = (ScriptedMap*)m_creature->GetInstanceData();
        // Npc flag is controlled by script
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        InitializeDialogueHelper(m_pScriptedMap);
        Reset();
    }

    ScriptedMap* m_pScriptedMap;

    uint32 m_uiGuardCheckTimer;
    uint8 m_uiOnyxiaGuardCount;

    uint32 m_uiHammerTimer;
    uint32 m_uiCleaveTimer;

    bool m_bIsKeepReady;
    bool m_bCanGuardSalute;

    ObjectGuid m_playerGuid;
    ObjectGuid m_guardsGuid[MAX_ROYAL_GUARDS];

    GuidList m_lRoyalGuardsGuidList;
    GuidSet m_sGuardsSalutedGuidSet;

    void Reset() override
    {
        m_uiGuardCheckTimer  = 0;
        m_bIsKeepReady       = false;
        m_bCanGuardSalute    = false;

        m_uiHammerTimer      = urand(0, 1000);
        m_uiCleaveTimer      = urand(1000, 3000);
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_SHIELD_WALL);
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        // Note: this implementation is not the best; It should be better handled by the guard script
        if (m_bCanGuardSalute && (pWho->GetEntry() == NPC_GUARD_CITY || pWho->GetEntry() == NPC_GUARD_ROYAL ||
                                  pWho->GetEntry() == NPC_GUARD_PATROLLER) && pWho->IsWithinDistInMap(m_creature, 15.0f) &&
                m_sGuardsSalutedGuidSet.find(pWho->GetObjectGuid()) == m_sGuardsSalutedGuidSet.end() && pWho->IsWithinLOSInMap(m_creature))
        {
            DoScriptText(aGuardSalute[urand(0, MAX_GUARD_SALUTES - 1)], pWho);
            m_sGuardsSalutedGuidSet.insert(pWho->GetObjectGuid());
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 0:
                if (!m_pScriptedMap)
                    break;
                // Prepare Jonathan for the first event
                if (Creature* pJonathan = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_JONATHAN))
                {
                    // Summon 3 guards on each side and move Jonathan in the middle
                    for (uint8 i = 0; i < MAX_ROYAL_GUARDS; ++i)
                    {
                        if (Creature* pTemp = m_creature->SummonCreature(NPC_GUARD_ROYAL, aGuardLocations[i][0], aGuardLocations[i][1], aGuardLocations[i][2], aGuardLocations[i][3], TEMPSUMMON_TIMED_DESPAWN, 180000))
                            m_guardsGuid[i] = pTemp->GetObjectGuid();
                    }

                    pJonathan->SetWalk(false);
                    pJonathan->Unmount();
                    pJonathan->GetMotionMaster()->MovePoint(0, aMoveLocations[0][0], aMoveLocations[0][1], aMoveLocations[0][2]);
                }
                break;
            case 1:
                StartNextDialogueText(SAY_JON_DIALOGUE_1);
                SetEscortPaused(true);
                break;
            case 3:
                m_bCanGuardSalute = true;
                break;
            case 11:
                if (!m_pScriptedMap)
                    break;
                // We can reset Jonathan now
                if (Creature* pJonathan = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_JONATHAN))
                {
                    pJonathan->SetWalk(true);
                    pJonathan->SetStandState(UNIT_STAND_STATE_STAND);
                    pJonathan->GetMotionMaster()->MoveTargetedHome();
                }
                break;
            case 22:
                SetEscortPaused(true);
                m_creature->SetFacingTo(5.41f);
                StartNextDialogueText(NPC_GUARD_ROYAL);
                break;
            case 24:
                m_bCanGuardSalute = false;
                break;
            case 25:
                StartNextDialogueText(NPC_WRYNN);
                SetEscortPaused(true);
                m_bCanGuardSalute = false;
                break;
        }
    }

    void SummonedMovementInform(Creature* pSummoned, uint32 uiMotionType, uint32 uiPointId) override
    {
        if (uiMotionType != POINT_MOTION_TYPE || !uiPointId || pSummoned->GetEntry() != NPC_GUARD_ROYAL)
            return;

        // Handle city gates royal guards
        switch (uiPointId)
        {
            case 1:
            case 2:
                pSummoned->SetFacingTo(2.234f);
                pSummoned->SetStandState(UNIT_STAND_STATE_KNEEL);
                break;
            case 3:
            case 4:
                pSummoned->SetFacingTo(5.375f);
                pSummoned->SetStandState(UNIT_STAND_STATE_KNEEL);
                break;
        }
    }

    void JustDidDialogueStep(int32 iEntry) override
    {
        if (!m_pScriptedMap)
            return;

        switch (iEntry)
        {
                // Set orientation and prepare the npcs for the next event
            case SAY_WINDSOR_GET_READY:
                m_creature->SetFacingTo(0.6f);
                break;
            case SAY_PRESTOR_SIEZE:
                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                    Start(false, pPlayer);
                break;
            case SAY_JON_DIALOGUE_8:
                // Turn left and move the guards
                if (Creature* pJonathan = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_JONATHAN))
                    pJonathan->SetFacingTo(5.375f);
                if (Creature* pGuard = m_creature->GetMap()->GetCreature(m_guardsGuid[5]))
                {
                    pGuard->SetFacingTo(2.234f);
                    pGuard->SetStandState(UNIT_STAND_STATE_KNEEL);
                }
                if (Creature* pGuard = m_creature->GetMap()->GetCreature(m_guardsGuid[4]))
                    pGuard->GetMotionMaster()->MovePoint(1, aMoveLocations[1][0], aMoveLocations[1][1], aMoveLocations[1][2]);
                if (Creature* pGuard = m_creature->GetMap()->GetCreature(m_guardsGuid[3]))
                    pGuard->GetMotionMaster()->MovePoint(2, aMoveLocations[2][0], aMoveLocations[2][1], aMoveLocations[2][2]);
                break;
            case SAY_JON_DIALOGUE_9:
                // Turn right and move the guards
                if (Creature* pJonathan = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_JONATHAN))
                    pJonathan->SetFacingTo(2.234f);
                if (Creature* pGuard = m_creature->GetMap()->GetCreature(m_guardsGuid[2]))
                {
                    pGuard->SetFacingTo(5.375f);
                    pGuard->SetStandState(UNIT_STAND_STATE_KNEEL);
                }
                if (Creature* pGuard = m_creature->GetMap()->GetCreature(m_guardsGuid[1]))
                    pGuard->GetMotionMaster()->MovePoint(3, aMoveLocations[3][0], aMoveLocations[3][1], aMoveLocations[3][2]);
                if (Creature* pGuard = m_creature->GetMap()->GetCreature(m_guardsGuid[0]))
                    pGuard->GetMotionMaster()->MovePoint(4, aMoveLocations[4][0], aMoveLocations[4][1], aMoveLocations[4][2]);
                break;
            case SAY_JON_DIALOGUE_10:
                if (Creature* pJonathan = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_JONATHAN))
                    pJonathan->SetFacingToObject(m_creature);
                break;
            case EMOTE_ONESHOT_SALUTE:
                if (Creature* pJonathan = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_JONATHAN))
                    pJonathan->HandleEmote(EMOTE_ONESHOT_SALUTE);
                break;
            case NPC_JONATHAN:
                if (Creature* pJonathan = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_JONATHAN))
                {
                    pJonathan->SetWalk(true);
                    pJonathan->GetMotionMaster()->MovePoint(0, aMoveLocations[5][0], aMoveLocations[5][1], aMoveLocations[5][2]);
                }
                break;
            case EMOTE_ONESHOT_KNEEL:
                if (Creature* pJonathan = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_JONATHAN))
                {
                    pJonathan->SetFacingToObject(m_creature);
                    pJonathan->SetStandState(UNIT_STAND_STATE_KNEEL);
                }
                break;
            case SAY_WINDSOR_DIALOGUE_12:
                if (Creature* pJonathan = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_JONATHAN))
                    m_creature->SetFacingToObject(pJonathan);
                break;
            case SAY_WINDSOR_DIALOGUE_13:
                m_creature->SetFacingTo(0.6f);
                break;
            case EMOTE_ONESHOT_POINT:
                m_creature->HandleEmote(EMOTE_ONESHOT_POINT);
                break;
            case NPC_WINDSOR:
                SetEscortPaused(false);
                break;
            case SAY_WINDSOR_BEFORE_KEEP:
                m_bIsKeepReady = true;
                m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                break;
            case NPC_GUARD_CITY:
                SetEscortPaused(false);
                break;
            case NPC_WRYNN:
                // Remove npc flags during the event
                if (Creature* pOnyxia = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_PRESTOR))
                    pOnyxia->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                if (Creature* pWrynn = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_WRYNN))
                    pWrynn->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                if (Creature* pBolvar = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_BOLVAR))
                    pBolvar->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                break;
            case SAY_BOLVAR_KEEP_2:
                if (Creature* pWrynn = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_WRYNN))
                {
                    pWrynn->SetWalk(false);
                    pWrynn->ForcedDespawn(15000);
                    pWrynn->GetMotionMaster()->MovePoint(0, aMoveLocations[6][0], aMoveLocations[6][1], aMoveLocations[6][2]);

                    // Store all the nearby guards, in order to transform them into Onyxia guards
                    std::list<Creature*> lGuardsList;
                    GetCreatureListWithEntryInGrid(lGuardsList, pWrynn, NPC_GUARD_ROYAL, 25.0f);

                    for (std::list<Creature*>::const_iterator itr = lGuardsList.begin(); itr != lGuardsList.end(); ++itr)
                        m_lRoyalGuardsGuidList.push_back((*itr)->GetObjectGuid());
                }
                break;
            case SPELL_WINDSOR_READ:
                DoCastSpellIfCan(m_creature, SPELL_WINDSOR_READ);
                break;
            case EMOTE_BOLVAR_GASP:
                if (Creature* pOnyxia = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_PRESTOR))
                {
                    pOnyxia->UpdateEntry(NPC_LADY_ONYXIA);

                    if (Creature* pBolvar = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_BOLVAR))
                        pBolvar->SetFacingToObject(pOnyxia);
                }
                break;
            case SAY_PRESTOR_KEEP_9:
                if (Creature* pBolvar = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_BOLVAR))
                {
                    pBolvar->SetWalk(false);
                    pBolvar->GetMotionMaster()->MovePoint(0, aMoveLocations[7][0], aMoveLocations[7][1], aMoveLocations[7][2]);
                }
                break;
            case SAY_BOLVAR_KEEP_10:
                if (Creature* pBolvar = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_BOLVAR))
                {
                    if (Creature* pOnyxia = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_PRESTOR))
                    {
                        pBolvar->SetFacingToObject(pOnyxia);
                        DoScriptText(EMOTE_PRESTOR_LAUGH, pOnyxia);
                    }
                }
                break;
            case SAY_PRESTOR_KEEP_11:
                for (GuidList::const_iterator itr = m_lRoyalGuardsGuidList.begin(); itr != m_lRoyalGuardsGuidList.end(); ++itr)
                {
                    if (Creature* pGuard = m_creature->GetMap()->GetCreature(*itr))
                    {
                        if (!pGuard->isAlive())
                            continue;

                        pGuard->UpdateEntry(NPC_GUARD_ONYXIA);
                        DoScriptText(EMOTE_GUARD_TRANSFORM, pGuard);

                        if (Creature* pBolvar = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_BOLVAR))
                            pGuard->AI()->AttackStart(pBolvar);
                    }
                }
                m_uiGuardCheckTimer = 1000;
                break;
            case SPELL_WINDSOR_DEATH:
                if (Creature* pOnyxia = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_PRESTOR))
                    pOnyxia->CastSpell(m_creature, SPELL_WINDSOR_DEATH, TRIGGERED_NONE);
                break;
            case SAY_WINDSOR_KEEP_12:
                if (Creature* pOnyxia = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_PRESTOR))
                    DoScriptText(SAY_PRESTOR_KEEP_13, pOnyxia);

                // Fake death
                m_creature->InterruptNonMeleeSpells(true);
                m_creature->SetHealth(0);
                m_creature->StopMoving();
                m_creature->ClearComboPointHolders();
                m_creature->RemoveAllAurasOnDeath();
                m_creature->ModifyAuraState(AURA_STATE_HEALTHLESS_20_PERCENT, false);
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                m_creature->ClearAllReactives();
                m_creature->GetMotionMaster()->Clear();
                m_creature->GetMotionMaster()->MoveIdle();
                m_creature->SetStandState(UNIT_STAND_STATE_DEAD);
                break;
            case SAY_PRESTOR_KEEP_14:
                if (Creature* pOnyxia = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_PRESTOR))
                {
                    pOnyxia->ForcedDespawn(1000);
                    pOnyxia->HandleEmote(EMOTE_ONESHOT_LIFTOFF);
                    pOnyxia->CastSpell(pOnyxia, SPELL_ONYXIA_DESPAWN, TRIGGERED_NONE);
                }
                break;
            case NPC_GUARD_ONYXIA:
                if (Creature* pBolvar = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_BOLVAR))
                    pBolvar->GetMotionMaster()->MovePoint(0, aMoveLocations[7][0], aMoveLocations[7][1], aMoveLocations[7][2]);
                break;
            case NPC_BOLVAR:
                if (Creature* pBolvar = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_BOLVAR))
                {
                    pBolvar->SetWalk(true);
                    pBolvar->GetMotionMaster()->MovePoint(0, aMoveLocations[8][0], aMoveLocations[8][1], aMoveLocations[8][2]);
                }
                break;
            case SAY_BOLVAR_KEEP_15:
                if (Creature* pBolvar = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_BOLVAR))
                    pBolvar->SetStandState(UNIT_STAND_STATE_KNEEL);

                DoScriptText(SAY_WINDSOR_KEEP_16, m_creature);
                DoScriptText(EMOTE_WINDSOR_DIE, m_creature);

                if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                    pPlayer->GroupEventHappens(QUEST_THE_GREAT_MASQUERADE, m_creature);
                break;
            case NPC_GUARD_PATROLLER:
                // Reset Bolvar and Wrynn
                if (Creature* pBolvar = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_BOLVAR))
                {
                    pBolvar->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    pBolvar->SetStandState(UNIT_STAND_STATE_STAND);
                    pBolvar->GetMotionMaster()->MoveTargetedHome();
                }
                if (Creature* pWrynn = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_WRYNN))
                {
                    pWrynn->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    pWrynn->Respawn();
                    pWrynn->SetWalk(true);
                    pWrynn->GetMotionMaster()->MoveTargetedHome();
                }
                // Onyxia will respawn by herself in about 30 min, so just reset flags
                if (Creature* pOnyxia = m_pScriptedMap->GetSingleCreatureFromStorage(NPC_PRESTOR))
                    pOnyxia->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
                // Allow creature to despawn
                SetEscortPaused(false);
                break;
        }
    }

    void DoStartKeepEvent()
    {
        StartNextDialogueText(SAY_WINDSOR_TO_KEEP);
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    }

    void DoStartEscort(Player* pPlayer)
    {
        StartNextDialogueText(SAY_WINDSOR_QUEST_ACCEPT);
        m_playerGuid = pPlayer->GetObjectGuid();
    }

    bool IsKeepEventReady() { return m_bIsKeepReady; }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        DialogueUpdate(uiDiff);

        // Check if all Onyxia guards are dead
        if (m_uiGuardCheckTimer)
        {
            if (m_uiGuardCheckTimer <= uiDiff)
            {
                uint8 uiDeadGuardsCount = 0;
                for (GuidList::const_iterator itr = m_lRoyalGuardsGuidList.begin(); itr != m_lRoyalGuardsGuidList.end(); ++itr)
                {
                    if (Creature* pGuard = m_creature->GetMap()->GetCreature(*itr))
                    {
                        if (!pGuard->isAlive() && pGuard->GetEntry() == NPC_GUARD_ONYXIA)
                            ++uiDeadGuardsCount;
                    }
                }
                if (uiDeadGuardsCount == m_lRoyalGuardsGuidList.size())
                {
                    StartNextDialogueText(NPC_GUARD_ONYXIA);
                    m_uiGuardCheckTimer = 0;
                }
                else
                    m_uiGuardCheckTimer = 1000;
            }
            else
                m_uiGuardCheckTimer -= uiDiff;
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiHammerTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_HAMMER_OF_JUSTICE) == CAST_OK)
                m_uiHammerTimer = 60000;
        }
        else
            m_uiHammerTimer -= uiDiff;

        if (m_uiCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_STRONG_CLEAVE) == CAST_OK)
                m_uiCleaveTimer = urand(1000, 5000);
        }
        else
            m_uiCleaveTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_reginald_windsor(Creature* pCreature)
{
    return new npc_reginald_windsorAI(pCreature);
}

bool QuestAccept_npc_reginald_windsor(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_THE_GREAT_MASQUERADE)
    {
        if (npc_reginald_windsorAI* pReginaldAI = dynamic_cast<npc_reginald_windsorAI*>(pCreature->AI()))
            pReginaldAI->DoStartEscort(pPlayer);
    }

    return true;
}

bool GossipHello_npc_reginald_windsor(Player* pPlayer, Creature* pCreature)
{
    bool bIsEventReady = false;

    if (npc_reginald_windsorAI* pReginaldAI = dynamic_cast<npc_reginald_windsorAI*>(pCreature->AI()))
        bIsEventReady = pReginaldAI->IsKeepEventReady();

    // Check if event is possible and also check the status of the quests
    if (bIsEventReady && pPlayer->GetQuestStatus(QUEST_THE_GREAT_MASQUERADE) != QUEST_STATUS_COMPLETE && pPlayer->GetQuestRewardStatus(QUEST_STORMWIND_RENDEZVOUS))
    {
        pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_REGINALD, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_MASQUERADE, pCreature->GetObjectGuid());
    }
    else
    {
        if (pCreature->isQuestGiver())
            pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

        pPlayer->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, pCreature->GetObjectGuid());
    }

    return true;
}

bool GossipSelect_npc_reginald_windsor(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        if (npc_reginald_windsorAI* pReginaldAI = dynamic_cast<npc_reginald_windsorAI*>(pCreature->AI()))
            pReginaldAI->DoStartKeepEvent();

        pPlayer->CLOSE_GOSSIP_MENU();
    }

    return true;
}

struct SpawnLocation
{
    float m_fX, m_fY, m_fZ;
};

// NOTE: Stormwind Keep is explicitly not supposed to receive the buff
static const SpawnLocation aRallyGeneratorLocs[7] =
{
    { -8800.6f, 643.17f, 94.29f },   // Trade District
    { -8643.5f, 758.10f, 97.28f },   // Cathedral Square
    { -8523.0f, 848.78f, 106.8f },   // Cathedral of Light
    { -8951.7f, 865.88f, 104.9f },   // Mage District
    { -8424.0f, 619.94f, 95.55f },   // Blacksmith District
    { -8705.6f, 418.51f, 99.35f },   // Old Town
    { -8734.6f, 1043.2f, 92.15f }    // The Park
};

enum
{
    SPELL_RALLYING_CRY_DRAGONSLAYER = 22888,
    NPC_RALLY_CRY_GENERATOR_ALLY    = 21002,

    MAX_RALLY_GENERATORS            = 7
};

/*######
## npc_major_mattingly
######*/

enum
{
    QUEST_CELEBRATING_GOOD_TIMES        = 7496,

    YELL_ONY_ALLY_REWARD_1              = -143941,
    YELL_ONY_ALLY_REWARD_2              = -143942,

    GO_ONYXIAS_HEAD_ALLY                = 179558,

};

struct npc_major_mattinglyAI : public ScriptedAI
{
    npc_major_mattinglyAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }
    uint32 m_uiTick;
    uint32 m_uiDialogueTimer;
    bool m_bRallyingCryEvent;
    ObjectGuid m_playerGuid;

    void Reset()
    {
        m_uiTick = 0;
        m_uiDialogueTimer = 2000;
        m_bRallyingCryEvent = false;
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void StartRallyEvent(ObjectGuid playerGuid)
    {
        m_playerGuid = playerGuid;
        m_bRallyingCryEvent = true;
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void UpdateAI(uint32 const uiDiff)
    {
        if (m_bRallyingCryEvent)
        {
            if (m_uiDialogueTimer <= uiDiff)
            {
                switch (m_uiTick)
                {
                    case 0:
                        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                        {
                            m_creature->HandleEmote(EMOTE_ONESHOT_SHOUT);
                            m_creature->MonsterYellToZone(YELL_ONY_ALLY_REWARD_1, 0, pPlayer);
                        }
                        m_uiDialogueTimer = 10000;
                        break;
                    case 1:
                        m_creature->HandleEmote(EMOTE_ONESHOT_SHOUT);
                        m_creature->MonsterYellToZone(YELL_ONY_ALLY_REWARD_2);
                        if (GameObject* pGo = m_creature->FindNearestGameObject(GO_ONYXIAS_HEAD_ALLY, 150.0f))
                        {
                            if (!pGo->isSpawned())
                            {
                                pGo->SetRespawnTime(7200);
                                pGo->Refresh();
                            }
                        }
                        m_uiDialogueTimer = 7000;
                        break;
                    case 2:
                        m_creature->CastSpell(m_creature, SPELL_RALLYING_CRY_DRAGONSLAYER, true);
                        for (uint8 i = 0; i < MAX_RALLY_GENERATORS; ++i)
                        {
                            if (Creature* pGenerator = m_creature->SummonCreature(NPC_RALLY_CRY_GENERATOR_ALLY, aRallyGeneratorLocs[i].m_fX, aRallyGeneratorLocs[i].m_fY, aRallyGeneratorLocs[i].m_fZ, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 1000))
                                pGenerator->CastSpell(pGenerator, SPELL_RALLYING_CRY_DRAGONSLAYER, true);
                        }
                        Reset();
                        return;
                }
                m_uiTick++;
            }
            else m_uiDialogueTimer -= uiDiff;
        } 

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_major_mattingly(Creature* pCreature)
{
    return new npc_major_mattinglyAI(pCreature);
}

bool QuestRewarded_npc_major_mattingly(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_CELEBRATING_GOOD_TIMES)
    {
        if (npc_major_mattinglyAI* pMattingly = dynamic_cast<npc_major_mattinglyAI*>(pCreature->AI()))
            pMattingly->StartRallyEvent(pPlayer->GetObjectGuid());
    }
    return true;
}

/*######
## npc_field_marshal_afrasiabi
######*/

enum
{
    QUEST_LORD_OF_BLACKROCK_ALLY          = 7782,

    YELL_NEF_REWARD_1_MALE_ALLY          = -147211,
    YELL_NEF_REWARD_2_MALE_ALLY          = -147212,
    YELL_NEF_REWARD_1_FEMALE_ALLY        = -147213,
    YELL_NEF_REWARD_2_FEMALE_ALLY        = -147214,

    GO_NEFARIANS_HEAD_ALLY               = 179882,
};

struct npc_field_marshal_afrasiabiAI : public ScriptedAI
{
    npc_field_marshal_afrasiabiAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        Reset();
    }

    uint32 m_uiTick;
    uint32 m_uiDialogueTimer;
    bool m_bRallyingCryEvent;
    ObjectGuid m_playerGuid;

    void Reset()
    {
        m_uiTick = 0;
        m_uiDialogueTimer = 2000;
        m_bRallyingCryEvent = false;
        m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void StartRallyEvent(ObjectGuid playerGuid)
    {
        m_playerGuid = playerGuid;
        m_bRallyingCryEvent = true;
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void UpdateAI(uint32 const uiDiff)
    {
        if (m_bRallyingCryEvent)
        {
            if (m_uiDialogueTimer <= uiDiff)
            {
                switch (m_uiTick)
                {
                    case 0:
                        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                        {
                            m_creature->HandleEmote(EMOTE_ONESHOT_SHOUT);
                            m_creature->MonsterYellToZone(pPlayer->getGender() ? YELL_NEF_REWARD_1_FEMALE_ALLY : YELL_NEF_REWARD_1_MALE_ALLY, 0, pPlayer);
                        }
                        m_uiDialogueTimer = 10000;
                        break;
                    case 1:
                        if (Player* pPlayer = m_creature->GetMap()->GetPlayer(m_playerGuid))
                        {
                            m_creature->HandleEmote(EMOTE_ONESHOT_SHOUT);
                            m_creature->MonsterYellToZone(pPlayer->getGender() ? YELL_NEF_REWARD_2_FEMALE_ALLY : YELL_NEF_REWARD_2_MALE_ALLY);
                        }
                        if (GameObject* pGo = m_creature->FindNearestGameObject(GO_NEFARIANS_HEAD_ALLY, 150.0f))
                        {
                            if (!pGo->isSpawned())
                            {
                                pGo->SetRespawnTime(7200);
                                pGo->Refresh();
                            }
                        }
                        m_uiDialogueTimer = 7000;
                        break;
                    case 2:
                        m_creature->CastSpell(m_creature, SPELL_RALLYING_CRY_DRAGONSLAYER, true);
                        for (uint8 i = 0; i < MAX_RALLY_GENERATORS; ++i)
                        {
                            if (Creature* pGenerator = m_creature->SummonCreature(NPC_RALLY_CRY_GENERATOR_ALLY, aRallyGeneratorLocs[i].m_fX, aRallyGeneratorLocs[i].m_fY, aRallyGeneratorLocs[i].m_fZ, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 1000))
                                pGenerator->CastSpell(pGenerator, SPELL_RALLYING_CRY_DRAGONSLAYER, true);
                        }
                        Reset();
                        return;
                }
                m_uiTick++;
            }
            else m_uiDialogueTimer -= uiDiff;
        } 

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_field_marshal_afrasiabi(Creature* pCreature)
{
    return new npc_field_marshal_afrasiabiAI(pCreature);
}

bool QuestRewarded_npc_field_marshal_afrasiabi(Player* pPlayer, Creature* pCreature, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_LORD_OF_BLACKROCK_ALLY)
    {
        if (npc_field_marshal_afrasiabiAI* pAfrasiabi = dynamic_cast<npc_field_marshal_afrasiabiAI*>(pCreature->AI()))
            pAfrasiabi->StartRallyEvent(pPlayer->GetObjectGuid());
    }
    return true;
}

void AddSC_stormwind_city()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_archmage_malin";
    pNewScript->pGossipHello = &GossipHello_npc_archmage_malin;
    pNewScript->pGossipSelect = &GossipSelect_npc_archmage_malin;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_bartleby";
    pNewScript->GetAI = &GetAI_npc_bartleby;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_bartleby;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_dashel_stonefist";
    pNewScript->GetAI = &GetAI_npc_dashel_stonefist;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_dashel_stonefist;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_lady_katrana_prestor";
    pNewScript->pGossipHello = &GossipHello_npc_lady_katrana_prestor;
    pNewScript->pGossipSelect = &GossipSelect_npc_lady_katrana_prestor;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_squire_rowe";
    pNewScript->GetAI = &GetAI_npc_squire_rowe;
    pNewScript->pGossipHello = &GossipHello_npc_squire_rowe;
    pNewScript->pGossipSelect = &GossipSelect_npc_squire_rowe;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_reginald_windsor";
    pNewScript->GetAI = &GetAI_npc_reginald_windsor;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_reginald_windsor;
    pNewScript->pGossipHello = &GossipHello_npc_reginald_windsor;
    pNewScript->pGossipSelect = &GossipSelect_npc_reginald_windsor;
    pNewScript->RegisterSelf();

 // The Attack quest
    pNewScript = new Script;
    pNewScript->Name = "npc_lord_gregor_lescovar";
    pNewScript->GetAI = &GetAI_npc_lord_gregor_lescovar;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_tyrion_spybot";
    pNewScript->GetAI = &GetAI_npc_tyrion_spybot;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_tyrion";
    pNewScript->GetAI = &GetAI_npc_tyrion;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_tyrion;
    pNewScript->pGossipHello = &GossipHello_npc_tyrion;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_major_mattingly";
    pNewScript->GetAI = &GetAI_npc_major_mattingly;
    pNewScript->pQuestRewardedNPC = &QuestRewarded_npc_major_mattingly;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_field_marshal_afrasiabi";
    pNewScript->GetAI = &GetAI_npc_field_marshal_afrasiabi;
    pNewScript->pQuestRewardedNPC = &QuestRewarded_npc_field_marshal_afrasiabi;
    pNewScript->RegisterSelf();
}
