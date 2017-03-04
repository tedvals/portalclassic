/*
 * Auteur        : Chakor
 * All rights reserved */

/* ScriptData
SDName: Duskwood
SD%Complete: 100
SDComment: Quest support: 252
SDCategory: Duskwood
EndScriptData */

/* ContentData
npc_lord_ello_ebonlocke
EndContentData */

#include "precompiled.h"

/* ######
## go_elizas_grave_dirt
###### */

enum
{
    NPC_ELIZA                       = 314,

    QUEST_BRIDE_OF_THE_EMBALMER     = 253,
    QUEST_DIGGING_THROUGH_THE_DIRT  = 254
};

bool QuestRewarded_go_elizas_grave_dirt(Player* pPlayer, GameObject* pGo, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_DIGGING_THROUGH_THE_DIRT && pPlayer->GetQuestStatus(QUEST_BRIDE_OF_THE_EMBALMER) != QUEST_STATUS_COMPLETE)
    {
        Creature* pEliza = GetClosestCreatureWithEntry(pGo, NPC_ELIZA, 30.0f, true);
        if (!pEliza)
        {
            pGo->SummonCreature(NPC_ELIZA, -10267.0f, 52.52f, 42.54f, 2.5f, TEMPSUMMON_DEAD_DESPAWN, 15 * MINUTE * IN_MILLISECONDS);
            return true;
        }
    }

    return false;
}

enum
{
    SPELL_SOUL_CORRUPTION = 25805,
    SPELL_CREATURE_OF_NIGHTMARE = 25806,
    SPELL_SWELL_OF_SOULS = 21307,

    NPC_TWILIGHT_CORRUPTER = 15625,

    QUEST_NIGHTMARE_CORRUPTION = 8735,
};

bool AreaTrigger_at_twilight_grove(Player* pPlayer, const AreaTriggerEntry* pAt)
{
    if (pPlayer->isDead())
        return false;

    char wMessage[200];
    sprintf(wMessage, "Come, %s. See what the Nightmare brings...", pPlayer->GetName());

    if (pPlayer->GetQuestStatus(QUEST_NIGHTMARE_CORRUPTION) == QUEST_STATUS_INCOMPLETE)
        if (!pPlayer->FindNearestCreature(NPC_TWILIGHT_CORRUPTER, 350.0f))
            if (Creature* corrupter = pPlayer->SummonCreature(NPC_TWILIGHT_CORRUPTER, -10335.9f, -489.051f, 50.6233f, 2.59373f, TEMPSUMMON_DEAD_DESPAWN, 0))
                corrupter->MonsterWhisper(wMessage, (Unit*)pPlayer);
    return false;
}

struct npc_twilight_corrupterAI : ScriptedAI
{
    explicit npc_twilight_corrupterAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        npc_twilight_corrupterAI::Reset();
    }

    uint32 m_uiSoulCorruptionTimer;
    uint32 m_uiCreatureOfNightmareTimer;
    uint32 m_uiCheckTimer;
    bool bEngaged;
    uint64 CoNPlayerGuid;
    float CoNPlayerAggro;
    uint64 GUIDs[40];

    void Reset() override
    {
        m_uiSoulCorruptionTimer         = urand(6000, 18000);
        m_uiCreatureOfNightmareTimer    = urand(10000, 20000);
        m_uiCheckTimer  = 1000;
        CoNPlayerGuid   = 0;
        CoNPlayerAggro  = 0;
        bEngaged        = false;

        for (int i = 0; i < 40; i++)
            GUIDs[i] = 0;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (!bEngaged)
        {
            m_creature->MonsterSay("The Nightmare cannot be stopped!");
            bEngaged = true;
        }
    }

    void FillPlayerList()
    {
        for (int i = 0; i < 40; i++)
            GUIDs[i] = 0;

        ThreatList const& tList = m_creature->getThreatManager().getThreatList();
        for (ThreatList::const_iterator i = tList.begin(); i != tList.end(); ++i)
        {
            Unit* pUnit = m_creature->GetMap()->GetUnit((*i)->getUnitGuid());

            if (pUnit && pUnit->IsPlayer())
                for (int i = 0; i < 40; i++)
                    if (GUIDs[i] == 0)
                        GUIDs[i] = pUnit->GetGUID();
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiSoulCorruptionTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SOUL_CORRUPTION) == CAST_OK)
                m_uiSoulCorruptionTimer = urand(20000, 30000);
        }
        else
            m_uiSoulCorruptionTimer -= uiDiff;

        if (CoNPlayerGuid)
        {
            if (Unit* pTarget = m_creature->GetMap()->GetPlayer(CoNPlayerGuid))
            {
                if (!pTarget->HasAura(SPELL_CREATURE_OF_NIGHTMARE))
                {
                    m_creature->getThreatManager().modifyThreatPercent(pTarget, -100);
                    m_creature->getThreatManager().addThreatDirectly(pTarget, CoNPlayerAggro);
                    CoNPlayerGuid = 0;
                    CoNPlayerAggro = 0;
                }
            }
            else
            {
                CoNPlayerGuid = 0;
                CoNPlayerAggro = 0;
            }
        }

        if (m_uiCreatureOfNightmareTimer < uiDiff)
        {
            Unit* pTarg = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0);
            if (pTarg && pTarg->IsPlayer())
            {
                CoNPlayerGuid = pTarg->GetGUID();
                CoNPlayerAggro = m_creature->getThreatManager().getThreat(pTarg);
                if (DoCastSpellIfCan(pTarg, SPELL_CREATURE_OF_NIGHTMARE) == CAST_OK)
                    m_uiCreatureOfNightmareTimer = urand(35000, 40000);
            }
        }
        else
            m_uiCreatureOfNightmareTimer -= uiDiff;

        if (m_uiCheckTimer < uiDiff)
        {
            FillPlayerList();
            for (int i = 0; i < 40; i++)
            {
                if (Player* player = m_creature->GetMap()->GetPlayer(GUIDs[i]))
                {
                    if (player->isDead())
                    {
                        char eMessage[200];
                        sprintf(eMessage, "Twilight Corrupter squeezes the last bit of life out of %s and swallows their soul.", player->GetName());
                        m_creature->MonsterTextEmote(eMessage, nullptr, false);
                        m_creature->CastSpell(m_creature, SPELL_SWELL_OF_SOULS, true);
                        GUIDs[i] = 0;
                    }
                }
            }
            m_uiCheckTimer = 1000;
        }
        else
            m_uiCheckTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_twilight_corrupter(Creature* pCreature)
{
    return new npc_twilight_corrupterAI(pCreature);
}

/*
 * Watcher Blomberg (Stitches event support)
 */

enum
{
    NPC_WATCHER_DODDS   = 888,
    NPC_WATCHER_PAIGE   = 499
};

struct npc_watcher_blombergAI : ScriptedAI
{
    explicit npc_watcher_blombergAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        npc_watcher_blombergAI::Reset();
        npc_watcher_blombergAI::ResetCreature();
    }

    bool m_bIsEngaged;
    uint32 m_uiSayTimer;
    ObjectGuid m_DoddsGuid;
    ObjectGuid m_PaigeGuid;

    void Reset() override
    {
        m_creature->SetWalk(false);
    }

    void ResetCreature() override
    {
        m_bIsEngaged = false;
        m_uiSayTimer = 3000;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_bIsEngaged)
        {
            if (m_uiSayTimer < uiDiff)
            {
                m_creature->MonsterSay("The abomination is coming! Dodds! Paige! Come here and help us!");
                m_bIsEngaged = true;

                if (auto pDodds = m_creature->FindNearestCreature(NPC_WATCHER_DODDS, 200.0f))
                {
                    m_DoddsGuid = pDodds->GetObjectGuid();
                    pDodds->GetMotionMaster()->MovePoint(0, -10903.043945f, -377.539124f, 40.065773f, MOVE_PATHFINDING, 0, 1.19f);                    
                }


                if (auto pPaige = m_creature->FindNearestCreature(NPC_WATCHER_PAIGE, 200.0f))
                {
                    m_PaigeGuid = pPaige->GetObjectGuid();
                    pPaige->GetMotionMaster()->MovePoint(0, -10906.221680f, -375.957214f, 39.960278f, MOVE_PATHFINDING, 0, 1.19f);                    
                }
            }
            else
                m_uiSayTimer -= uiDiff;
        }

        ScriptedAI::UpdateAI(uiDiff);
    }
};

CreatureAI* GetAI_watcherBlomberg(Creature* pCreature)
{
    return new npc_watcher_blombergAI(pCreature);
}

/*
 * Watcher Selkin (Stitches event support)
 */

struct npc_watcher_selkinAI : npc_escortAI
{
    explicit npc_watcher_selkinAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        npc_watcher_selkinAI::Reset();
    }

    void Reset() override
    {
        m_creature->SetWalk(false);
    }

    void WaypointReached(uint32 /*uiPoint*/) override {}
};

CreatureAI* GetAI_watcherSelkin(Creature* pCreature)
{
    return new npc_watcher_selkinAI(pCreature);
}

/*
 *
 */

struct npc_commander_felstromAI : ScriptedAI
{
    explicit npc_commander_felstromAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        npc_commander_felstromAI::Reset();
    }

    bool b_suicide;
    uint32 m_uiSuicide_Timer;

    void Reset() override
    {
        m_uiSuicide_Timer = 1500;
        b_suicide = false;
        m_creature->ForceValuesUpdateAtIndex(UNIT_DYNAMIC_FLAGS);
    }

    void JustDied(Unit* Killer) override
    {
        if (Killer->GetEntry() == 771)
            m_creature->SetLootRecipient(nullptr);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiSuicide_Timer < uiDiff)
        {
            if (m_creature->GetHealthPercent() <= 10.0f)
            {
                if (!b_suicide)
                {
                    DoCastSpellIfCan(m_creature, 3488, true);
                    b_suicide = true;
                }
            }
        }
        else
            m_uiSuicide_Timer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_commanderFelstrom(Creature* pCreature)
{
    return new npc_commander_felstromAI(pCreature);
}

/*
* Lord Ello Ebonlocke
*/

enum
{
    NPC_STITCHES = 412,

    QUEST_TRANSLATION_TO_ELO = 252,
};

struct elloEbonlockeAI : ScriptedAI
{
    explicit elloEbonlockeAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        elloEbonlockeAI::Reset();
        elloEbonlockeAI::ResetCreature();
    }

    ObjectGuid m_stitchesGuid;
    uint32 m_uiTimer;
    bool m_bCanSummon;

    void Reset() override {}

    void ResetCreature() override
    {
        m_bCanSummon = false;
        m_uiTimer = 3000;
    }

    void StitchesDied()
    {
        m_uiTimer = 10 * MINUTE * IN_MILLISECONDS;
        m_stitchesGuid.Clear();
    }

    void SummonStitches();
    void LaunchStitches(Creature* pStitches) const;

    void UpdateAI(uint32 const uiDiff) override
    {
        if (!m_stitchesGuid && !m_bCanSummon)
        {
            if (m_uiTimer < uiDiff)
                m_bCanSummon = true;
            else
                m_uiTimer -= uiDiff;
        }

        ScriptedAI::UpdateAI(uiDiff);
    }
};

/*
 * Stitches
 */

enum
{
    NPC_WATCHER_CORWIN      = 1204,
    NPC_WATCHER_SARYS       = 1203,
    NPC_TOWN_CRIER          = 468,
    NPC_WATCHER_HUTCHINS    = 1001,
    NPC_WATCHER_BLOMBERG    = 1000,
    NPC_WATCHER_CUTFORD     = 1436,
    NPC_WATCHER_MERANT      = 1098,
    NPC_WATCHER_GELWIN      = 1099,
    NPC_WATCHER_SELKIN      = 1100,
    NPC_WATCHER_THAYER      = 1101,

    STITCHES_YELL           = -1500000,

    SPELL_AURA_OF_ROT       = 3106
};

struct Coords
{
    uint32 entry;
    float x, y, z, o;
};

static const Coords Watchman[] = 
{
    { NPC_WATCHER_HUTCHINS, -10912.09f, -394.11f, 41.11f, 5.93f },
    { NPC_WATCHER_BLOMBERG, -10909.53f, -397.48f, 41.14f, 5.93f },

    { NPC_WATCHER_CUTFORD,  -10910.27f, -519.03f, 52.99f, 5.93f },

    { NPC_WATCHER_SELKIN,   -10618.22f, -1185.36f, 28.58f, 5.93f },
    { NPC_WATCHER_GELWIN,   -10618.90f, -1182.15f, 28.57f, 5.93f },
    { NPC_WATCHER_MERANT,   -10616.64f, -1181.67f, 28.49f, 5.93f },
    { NPC_WATCHER_THAYER,   -10615.99f, -1184.56f, 28.46f, 5.93f },

    { NPC_WATCHER_SARYS,    -10574.31f, -1179.06f, 28.03f, 3.05f },
    { NPC_WATCHER_CORWIN,   -10575.13f, -1170.07f, 28.25f, 3.61f }
};

#define TOWNCRIER_YELL_1    "The abomination has overrun the Night Watch camp! Quickly, we must intercept it before it reaches town!"
#define TOWNCRIER_YELL_2    "The abomination has come! Forward!"
#define TOWNCRIER_YELL_3    "The beast is slain! All's well in Darkshire!"

struct npc_stitchesAI : npc_escortAI
{
    explicit npc_stitchesAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        npc_stitchesAI::Reset();
    }

    std::list<ObjectGuid> m_lWatchman;
    ObjectGuid m_townCrierGuid, m_DoddsGuid, m_PaigeGuid, m_lordElloGuid;
    uint32 m_uiAuraOfRotTimer;
    uint32 m_uiLaunchTimer;
    bool m_bLaunchChecked;

    void Reset() override
    {
        m_uiAuraOfRotTimer = 0;
        m_uiLaunchTimer = 10000;
        m_bLaunchChecked = false;
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        auto pTownCrier = m_creature->GetMap()->GetCreature(m_townCrierGuid);
        if (pTownCrier && pTownCrier->isAlive())
            pTownCrier->MonsterYell(TOWNCRIER_YELL_3);

        DespawnWatcher();

        if (auto pDodds = m_creature->GetMap()->GetCreature(m_DoddsGuid))
            pDodds->GetMotionMaster()->MoveTargetedHome();

        if (auto pPaige = m_creature->GetMap()->GetCreature(m_PaigeGuid))
            pPaige->GetMotionMaster()->MoveTargetedHome();

        if (auto pEllo = m_creature->GetMap()->GetCreature(m_lordElloGuid))
        {
            if (auto pElloAI = static_cast<elloEbonlockeAI*>(pEllo->AI()))
                pElloAI->StitchesDied();
        }
    }

    void DespawnWatcher()
    {
        for (auto itr = m_lWatchman.begin(); itr != m_lWatchman.end(); ++itr)
        {
            if (auto pWatchman = m_creature->GetMap()->GetCreature(*itr))
            {
                if (pWatchman->isAlive())
                    pWatchman->DisappearAndDie();
            }
        }
    }

    Creature* SummonWatchman(uint8 index) const
    {
        return m_creature->SummonCreature(Watchman[index].entry, 
            Watchman[index].x,
            Watchman[index].y,
            Watchman[index].z,
            Watchman[index].o, TEMPSUMMON_CORPSE_DESPAWN, 10000, true);
    }

    void AddToFormation(Creature* pLeader, Creature* pAdd) const
    {
        if (!pLeader || !pAdd)
            return;

        pAdd->JoinCreatureGroup(pLeader, 4.0f, pLeader->GetAngle(pAdd) - (pAdd)->GetOrientation(),
            OPTION_FORMATION_MOVE | OPTION_AGGRO_TOGETHER | OPTION_EVADE_TOGETHER);
    }

    void JustSummoned(Creature* pSummoned) override
    {
        m_lWatchman.push_back(pSummoned->GetObjectGuid());

        switch (pSummoned->GetEntry())
        {
        case NPC_WATCHER_HUTCHINS:
            pSummoned->SetWalk(false);
            pSummoned->GetMotionMaster()->MovePoint(0, -10905.52f, -374.1f, 39.88f, MOVE_PATHFINDING);
            pSummoned->SetCombatStartPosition(-10905.52f, -374.1f, 39.88f);
            pSummoned->SetHomePosition(-10905.52f, -374.1f, 39.88f, 0.0f); 
            break;
        case NPC_WATCHER_BLOMBERG:
            pSummoned->SetWalk(false);
            pSummoned->GetMotionMaster()->MovePoint(0, -10902.211914f, -375.488495f, 40.000954f, MOVE_PATHFINDING);
            pSummoned->SetCombatStartPosition(-10902.211914f, -375.488495f, 40.000954f);
            pSummoned->SetHomePosition(-10902.211914f, -375.488495f, 40.000954f, 0.0f);
            if (auto pSummonedAI = static_cast<npc_watcher_blombergAI*>(pSummoned->AI()))
            {
                m_DoddsGuid = pSummonedAI->m_DoddsGuid;
                m_PaigeGuid = pSummonedAI->m_PaigeGuid;
            }
            break;
        case NPC_WATCHER_CUTFORD:
            pSummoned->SetWalk(false);
            pSummoned->GetMotionMaster()->MovePoint(0, -10904.029297f, -374.387299f, 39.899269f, MOVE_PATHFINDING);
            pSummoned->SetCombatStartPosition(-10904.029297f, -374.387299f, 39.899269f);
            pSummoned->SetHomePosition(-10904.029297f, -374.387299f, 39.899269f, 0.0f);
            break;
        case NPC_WATCHER_SELKIN:
            if (auto pSummonedAI = static_cast<npc_watcher_selkinAI*>(pSummoned->AI()))
                pSummonedAI->Start(true);
            break;
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        m_lWatchman.remove(pSummoned->GetObjectGuid());
    }

    void WaypointReached(uint32 uiPoint) override
    {
        switch (uiPoint)
        {
        case 10:
        case 29:
            m_creature->MonsterYellToZone(STITCHES_YELL);
            break;
        case 21:
            SummonWatchman(0);
            SummonWatchman(1);
            break;
        case 23:
            {
                auto pTownCrier = m_creature->GetMap()->GetCreature(m_townCrierGuid);
                if (pTownCrier && pTownCrier->isAlive())
                    pTownCrier->MonsterYell(TOWNCRIER_YELL_1);
            }
            break;
        case 33:
            SummonWatchman(2);
            break;
        case 41:
            {
                auto pLeader = SummonWatchman(3);
                AddToFormation(pLeader, SummonWatchman(4));
                AddToFormation(pLeader, SummonWatchman(5));
                AddToFormation(pLeader, SummonWatchman(6));
                pLeader->SetWalk(false);             
            }
            break;
        case 61:
            {
                m_creature->MonsterYellToZone(STITCHES_YELL);
                SummonWatchman(7);
                SummonWatchman(8);
                auto pTownCrier = m_creature->GetMap()->GetCreature(m_townCrierGuid);
                if (pTownCrier && pTownCrier->isAlive())
                    pTownCrier->MonsterYell(TOWNCRIER_YELL_2);                
            }
            break;
        case 65:
            m_creature->SetHomePosition(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 1.64f);
            Stop();
            m_creature->GetMotionMaster()->MoveRandom();
            break;
        }
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (!urand(0, 3))
            m_creature->MonsterYell("ROAAAARR!");
    }

    void UpdateEscortAI(uint32 const uiDiff) override
    {
        if (!m_bLaunchChecked)
        {
            if (m_uiLaunchTimer < uiDiff)
            {
                if (!HasEscortState(STATE_ESCORT_ESCORTING))
                {
                    sLog.outError("[Duskwood.Stitches] Emergency launch.");
                    Start();
                }

                m_bLaunchChecked = true;
            }
            else
                m_uiLaunchTimer -= uiDiff;            
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiAuraOfRotTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_AURA_OF_ROT) == CAST_OK)
                m_uiAuraOfRotTimer = 3000;
        }
        else
        {
            m_uiAuraOfRotTimer -= uiDiff;
            DoMeleeAttackIfReady();
        }  
    }
};

CreatureAI* GetAI_stitches(Creature* pCreature)
{
    return new npc_stitchesAI(pCreature);
}

/*
 * Lord Ello Ebonlocke
 */

void elloEbonlockeAI::SummonStitches()
{
    if (m_creature->GetMap()->GetCreature(m_stitchesGuid))
        return;

    if (!m_bCanSummon)
        return;

    if (auto pStitches = m_creature->SummonCreature(NPC_STITCHES, -10277.63f, 54.27f, 42.2f, 4.22f, TEMPSUMMON_DEAD_DESPAWN, 0, true))
    {
        m_bCanSummon = false;
        m_stitchesGuid = pStitches->GetObjectGuid();
        pStitches->SetObjectScale(2.0f);

        LaunchStitches(pStitches);
    }
}

void elloEbonlockeAI::LaunchStitches(Creature* pStitches) const
{
    if (auto stitchesAI = static_cast<npc_stitchesAI*>(pStitches->AI()))
    {
        if (auto pTownCrier = m_creature->FindNearestCreature(NPC_TOWN_CRIER, 400.0f))
            stitchesAI->m_townCrierGuid = pTownCrier->GetObjectGuid();

        stitchesAI->m_lordElloGuid = m_creature->GetObjectGuid();

        stitchesAI->Start();
    }
    else
        sLog.outError("[Duskwood.Stitches] Failed to cast AI.");
}


CreatureAI* GetAI_ElloEbonlocke(Creature* pCreature)
{
    return new elloEbonlockeAI(pCreature);
}

bool GossipHello_npc_lord_ello_ebonlocke(Player* pPlayer, Creature* pCreature)
{
    /** Show quest menu */
    pPlayer->PrepareQuestMenu(pCreature->GetGUID());
    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());
    return true;
}

bool QuestRewarded_npc_lord_ello_ebonlocke(Player* pPlayer, Creature* pCreature, Quest const* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_TRANSLATION_TO_ELO)
    {
        if (auto pCreatureAI = static_cast<elloEbonlockeAI*>(pCreature->AI()))
            pCreatureAI->SummonStitches();
    }

    return true;
}

bool GossipHelloGo_go_weathered_grave(Player* pPlayer, GameObject* pGo)
{
    if (pGo && pGo->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER && pPlayer)
    {
        if (pPlayer->GetQuestStatus(QUEST_THE_WEATHERED_GRAVE) == QUEST_STATUS_NONE || pPlayer->GetQuestStatus(QUEST_A_DAUGHTERS_LOVE) == QUEST_STATUS_COMPLETE)
            pPlayer->PrepareQuestMenu(pGo->GetObjectGuid());
    }

    pPlayer->SEND_GOSSIP_MENU(GOSSIP_GREETINGS, pGo->GetObjectGuid());
    return true;
}

/* ######
## npc_morgan_ladimore
###### */

const char* ladimore_say1 = "My sword Archeus served me well in life, but as at last my spirit may pass from the unhappy existance, I need it no longer.";
const char* ladimore_say2 = "I shall cling to the love of my daugter and hope that I will find forgiveness under the Light for my sins.";

struct npc_morgan_ladimoreAI : public ScriptedAI
{
    npc_morgan_ladimoreAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        npc_morgan_ladimoreAI::Reset();
    }

    uint32  m_uiTimer;
    uint8   m_uiPhrase;

    bool    m_uiDone;

    void Reset() override
    {
        m_uiTimer = 3000;
        m_uiPhrase = 0;

        m_uiDone = false;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (m_uiDone) return;

        if (m_uiTimer < uiDiff)
        {
            switch (m_uiPhrase)
            {
                case 0:
                    m_creature->MonsterSay(ladimore_say1, LANG_UNIVERSAL);
                    m_uiTimer = 6000;
                    ++m_uiPhrase;
                    break;
                case 1:
                    m_creature->MonsterSay(ladimore_say2, LANG_UNIVERSAL);
                    m_uiDone = true;
                    break;
            }
        }
        else
            m_uiTimer -= uiDiff;
    }
};

CreatureAI* GetAI_npc_morgan_ladimore(Creature* pCreature)
{
    return new npc_morgan_ladimoreAI(pCreature);
}

void AddSC_duskwood()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "commander_felstrom";
    pNewScript->GetAI = &GetAI_commanderFelstrom;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "watcher_selkin";
    pNewScript->GetAI = &GetAI_watcherSelkin;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "watcher_blomberg";
    pNewScript->GetAI = &GetAI_watcherBlomberg;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_stitches";
    pNewScript->GetAI = &GetAI_stitches;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_lord_ello_ebonlocke";
    pNewScript->GetAI = &GetAI_ElloEbonlocke;
    pNewScript->pQuestRewardedNPC = &QuestRewarded_npc_lord_ello_ebonlocke;
    pNewScript->pGossipHello = &GossipHello_npc_lord_ello_ebonlocke;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_twilight_grove";
    pNewScript->pAreaTrigger = &AreaTrigger_at_twilight_grove;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_twilight_corrupter";
    pNewScript->GetAI = &GetAI_npc_twilight_corrupter;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_elizas_grave_dirt";
    pNewScript->pQuestRewardedGO = &QuestRewarded_go_elizas_grave_dirt;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_weathered_grave";
    pNewScript->pGossipHelloGO = &GossipHelloGo_go_weathered_grave;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_morgan_ladimore";
    pNewScript->GetAI = &GetAI_npc_morgan_ladimore;
    pNewScript->RegisterSelf();
}
