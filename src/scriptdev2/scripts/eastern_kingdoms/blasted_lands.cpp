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
SDName: Blasted_Lands
SD%Complete: 90
SDComment: Quest support: 2784, 2801. Missing some texts for Fallen Hero. Teleporter to Rise of the Defiler missing group support.
SDCategory: Blasted Lands
EndScriptData */

/* ContentData
npc_fallen_hero_of_horde
EndContentData */

#include "precompiled.h"

#include "scriptPCH.h"

/*######
## npc_servants
######*/

enum
{
    SPELL_SPIRIT_SHOCK          = 10794,

    NPC_SERVANT_OF_RAZELIKH     = 7668,
    NPC_SERVANT_OF_ALISTARJ     = 7670,
    NPC_SERVANT_OF_SEVINE       = 7671,
    NPC_SERVANT_OF_GROL         = 7669,

    GO_STONE_BINDING_RAZELIKH   = 141812,
    GO_STONE_BINDING_GROL       = 141857,
    GO_STONE_BINDING_ALISTARJ   = 141858,
    GO_STONE_BINDING_SEVINE     = 141859
};

struct npc_servantsAI : public ScriptedAI
{
    npc_servantsAI(Creature* pCreature) : ScriptedAI(pCreature) { Reset(); }    

    bool m_bImmunity;
    bool m_bCanDead;

    ObjectGuid m_uiBindingStone;

    void Reset() override
    {
        m_bImmunity = false;
        m_bCanDead  = false;
        m_uiBindingStone.Clear();
    }

    void RemoveImmunity()
    {
        m_bCanDead = true;
    }

    void EnterEvadeMode() override
    {
        if (m_uiBindingStone.GetRawValue())
        {
            if (GameObject* pGo = m_creature->GetMap()->GetGameObject(m_uiBindingStone))
                pGo->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
        }

        ScriptedAI::EnterEvadeMode();
    }

    void DamageTaken(Unit* pDoneBy, uint32 &uiDamage) override
    {
        if (!m_bImmunity && m_creature->GetHealth() < uiDamage)
        {
            m_bImmunity = true;
            uint32 goEntry = 0;

            switch (m_creature->GetEntry())
            {
                case NPC_SERVANT_OF_RAZELIKH : goEntry = GO_STONE_BINDING_RAZELIKH;  break;
                case NPC_SERVANT_OF_ALISTARJ : goEntry = GO_STONE_BINDING_ALISTARJ;  break;
                case NPC_SERVANT_OF_SEVINE   : goEntry = GO_STONE_BINDING_SEVINE;    break;
                case NPC_SERVANT_OF_GROL     : goEntry = GO_STONE_BINDING_GROL;      break;
            }

            if (GameObject* pGo = GetClosestGameObjectWithEntry(m_creature, goEntry, 20.0f))
            {
                pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                m_uiBindingStone = pGo->GetObjectGuid();
            }

            m_creature->CastSpell(m_creature, SPELL_SPIRIT_SHOCK, true);
        }

        if (m_bImmunity && !m_bCanDead)
            uiDamage = 0;
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;     

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_servants(Creature* pCreature)
{
    return new npc_servantsAI(pCreature);
}

bool GOUSE_stone_of_binding(Player* pPlayer, GameObject* pGO)
{
    if (!pPlayer || !pGO)
        return false;

    uint32 ncpEntry = 0;

    switch (pGO->GetEntry())
    {
        case GO_STONE_BINDING_RAZELIKH : ncpEntry = NPC_SERVANT_OF_RAZELIKH;  break;
        case GO_STONE_BINDING_GROL     : ncpEntry = NPC_SERVANT_OF_GROL    ;  break;
        case GO_STONE_BINDING_ALISTARJ : ncpEntry = NPC_SERVANT_OF_ALISTARJ;  break;
        case GO_STONE_BINDING_SEVINE   : ncpEntry = NPC_SERVANT_OF_SEVINE  ;  break;
        default :
            return false;
    }

    if (Creature* pCreature = GetClosestCreatureWithEntry(pPlayer, ncpEntry, 11.0f))
    {
        if (npc_servantsAI* pServantAI = dynamic_cast<npc_servantsAI*>(pCreature->AI()))
            pServantAI->RemoveImmunity();
    }

    return true;
}

/*######
## npc_deathly_usher
######*/

#define GOSSIP_ITEM_USHER "I wish to to visit the Rise of the Defiler."

#define SPELL_TELEPORT_SINGLE           12885
#define SPELL_TELEPORT_SINGLE_IN_GROUP  13142
#define SPELL_TELEPORT_GROUP            27686

#define NPC_RAZELIKH                    7664

bool GossipHello_npc_deathly_usher(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(3628) == QUEST_STATUS_INCOMPLETE && pPlayer->HasItemCount(10757, 1))
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_USHER, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

     pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_deathly_usher(Player* pPlayer, Creature* pCreature, uint32 uiSender, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF)
    {
        pPlayer->CLOSE_GOSSIP_MENU();
        pCreature->CastSpell(pPlayer, SPELL_TELEPORT_SINGLE, true);
    }

    return true;
}

/*######
## npc_fallen_hero_of_horde
######*/

#define GOSSIP_ITEM_FALLEN "Continue..."

#define GOSSIP_ITEM_FALLEN1 "What could be worse than death?"
#define GOSSIP_ITEM_FALLEN2 "Subordinates?"
#define GOSSIP_ITEM_FALLEN3 "What are the stones of binding?"
#define GOSSIP_ITEM_FALLEN4 "You can count on me, Hero"
#define GOSSIP_ITEM_FALLEN5 "I shall"

bool GossipHello_npc_fallen_hero_of_horde(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestStatus(2784) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Why are you here?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (pPlayer->GetQuestStatus(2801) == QUEST_STATUS_INCOMPLETE && pPlayer->GetTeam() == HORDE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Continue story...", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

    if (pPlayer->GetQuestStatus(2801) == QUEST_STATUS_INCOMPLETE && pPlayer->GetTeam() == ALLIANCE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Why are you here?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    pPlayer->SEND_GOSSIP_MENU(pPlayer->GetGossipTextId(pCreature), pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_fallen_hero_of_horde(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 11);
            pPlayer->SEND_GOSSIP_MENU(1392, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+11:
            pPlayer->SEND_GOSSIP_MENU(1411, pCreature->GetObjectGuid());
            if (pPlayer->GetQuestStatus(2784) == QUEST_STATUS_INCOMPLETE)
                pPlayer->AreaExploredOrEventHappens(2784);
            if (pPlayer->GetTeam() == ALLIANCE)
            {
                pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                pPlayer->SEND_GOSSIP_MENU(1411, pCreature->GetObjectGuid());
            }
            break;

        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 21);
            pPlayer->SEND_GOSSIP_MENU(1451, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+21:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 22);
            pPlayer->SEND_GOSSIP_MENU(1452, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+22:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 23);
            pPlayer->SEND_GOSSIP_MENU(1453, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+23:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 24);
            pPlayer->SEND_GOSSIP_MENU(1454, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+24:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 25);
            pPlayer->SEND_GOSSIP_MENU(1455, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+25:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_FALLEN5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 26);
            pPlayer->SEND_GOSSIP_MENU(1456, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+26:
            pPlayer->CLOSE_GOSSIP_MENU();
            pPlayer->AreaExploredOrEventHappens(2801);
            break;
    }
    return true;
}

bool GO_Altar_of_the_defiler(Player* pPlayer, GameObject* pGO)
{
    if (!pPlayer || !pGO)
        return false;
    pPlayer->SummonCreature(NPC_RAZELIKH, -11234.38f, -2842.71f, 157.92f, 1.524f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 30000);
    return true;
}

struct ThadiusGrimshadeAI : public ScriptedAI
{
    ThadiusGrimshadeAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        underEvent = false;
        m_uiBeginTimer = 3000;
        m_uiTranceTimer = 8000;
        grimshadeGUID = 0;
        underTrance = false;
        underSpeak = false;
        m_uiSpeakTimer = 8000;
        sentence = false;
        createObject = false;
        isStun = false;
        playerGUID = 0;
        Reset();
    }

    bool underEvent;
    bool underTrance;
    bool underSpeak;
    bool sentence;
    bool createObject;
    bool isStun;
    uint32 m_uiSpeakTimer;
    uint32 m_uiBeginTimer;
    uint32 m_uiTranceTimer;
    uint64 grimshadeGUID;
    uint64 playerGUID;

    void Reset()
    {
    }

    void QuestCompleted(Player* pPlayer, Quest const* pQuest)
    {
        if (!underEvent)
        {
            playerGUID = pPlayer->GetGUID();
            underEvent = true;
        }
    }

    void UpdateAI(const uint32 uiDiff)
    {
        if (underEvent)
        {
            if (!createObject)
            {
                if (GameObject* pGo = m_creature->SummonGameObject(144069, -10999.166992f, -3484.187012f, 103.127243f, 2.592681f))
                    grimshadeGUID = pGo->GetGUID();
                createObject = true;
            }
            if (m_uiBeginTimer < uiDiff)
                underTrance = true;
            else
                m_uiBeginTimer -= uiDiff;

        }

        if (underTrance)
        {
            if (!isStun)
            {
                m_creature->HandleEmote(EMOTE_STATE_STUN);
                isStun = true;
            }
            if (m_uiTranceTimer < uiDiff)
                underSpeak = true;
            else
                m_uiTranceTimer -= uiDiff;

        }

        if (underSpeak)
        {
            if (!sentence)
            {
                m_creature->MonsterSay("...Cage...temple...trolls...", 0, 0);
                sentence = true;
            }
            if (m_uiSpeakTimer < uiDiff)
            {
                m_creature->HandleEmote(EMOTE_STATE_NONE);
                if (GameObject* pVision = m_creature->GetMap()->GetGameObject(grimshadeGUID))
                    pVision->Delete();

                underEvent = false;
                m_uiBeginTimer = 3000;
                m_uiTranceTimer = 3000;
                grimshadeGUID = 0;
                underTrance = false;
                underSpeak = false;
                m_uiSpeakTimer = 3000;
                sentence = false;
                createObject = false;
                isStun = false;

                if (Player* player = m_creature->GetMap()->GetPlayer(playerGUID))
                {
                    if (player->GetQuestStatus(2992) == QUEST_STATUS_INCOMPLETE)
                        player->AreaExploredOrEventHappens(2992);
                }
            }
            else
                m_uiSpeakTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_thadius_grimshade(Creature* pCreature)
{
    return new ThadiusGrimshadeAI(pCreature);
}

bool QuestAccept_npc_Thadius_Grimshade(Player* pPlayer, Creature* pQuestGiver, Quest const* pQuest)
{
    if (pQuest->GetQuestId() != 2992)
        return false;

    if (ThadiusGrimshadeAI* pThadius = dynamic_cast<ThadiusGrimshadeAI*>(pQuestGiver->AI()))
        pThadius->QuestCompleted(pPlayer, pQuest);
    return true;
}

void AddSC_blasted_lands()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "npc_deathly_usher";
    pNewScript->pGossipHello =  &GossipHello_npc_deathly_usher;
    pNewScript->pGossipSelect = &GossipSelect_npc_deathly_usher;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_fallen_hero_of_horde";
    pNewScript->pGossipHello =  &GossipHello_npc_fallen_hero_of_horde;
    pNewScript->pGossipSelect = &GossipSelect_npc_fallen_hero_of_horde;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "GO_Altar_of_the_defiler";
    pNewScript->pGOUse = &GO_Altar_of_the_defiler;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_servants";
    pNewScript->GetAI = GetAI_npc_servants;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_stone_of_binding";
    pNewScript->pGOUse = &GOUSE_stone_of_binding;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_thadius_grimshade";
    pNewScript->GetAI = &GetAI_thadius_grimshade;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_Thadius_Grimshade;
    pNewScript->RegisterSelf();

}
