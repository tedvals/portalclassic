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
SDName: Elwynn_Forest
SD%Complete: 0
SDComment: Placeholder
SDCategory: Elwynn Forest
EndScriptData */

/* ContentData
EndContentData */

#include "scriptPCH.h"

enum
{
//guid 12609 entry 177673 Serpant statue
    NPC_SPIRIT                  = 2044,
    QUEST_STALVANS_LEGEND       = 75

};
struct go_marshal_haggards_chestAI: public GameObjectAI
{
    go_marshal_haggards_chestAI(GameObject* pGo) : GameObjectAI(pGo)
    {
        timer = 0;
        state = 0;
        guid_kragaru = 0;
    }
    uint64 guid_kragaru;
    uint32 timer;
    bool state;//0 = usual, can launch. //1 = in use, cannot launch

    void UpdateAI(const uint32 uiDiff)
    {
        if (state)
        {
            if (timer < uiDiff)
                state = 0;
            else
                timer -= uiDiff;
        }
    }
    bool CheckCanStartEvent()
    {
        if (!state && !me->GetMap()->GetCreature(guid_kragaru))
            return true;
        return false;
    }

    void SetInUse(Creature* kragaru)
    {
        guid_kragaru = kragaru->GetGUID();
        state = 1;
        timer = 60000;
    }
};
GameObjectAI* GetAIgo_marshal_haggards_chest(GameObject *pGo)
{
    return new go_marshal_haggards_chestAI(pGo);
}
bool GOHello_go_marshal_haggards_chest(Player* pPlayer, GameObject* pGo)
{
    if (go_marshal_haggards_chestAI* pMarkAI = dynamic_cast<go_marshal_haggards_chestAI*>(pGo->AI()))
    {
        if (pMarkAI->CheckCanStartEvent())
        {
            if (pPlayer->GetQuestStatus(QUEST_STALVANS_LEGEND) == QUEST_STATUS_INCOMPLETE)
            {
                if (Creature* kragaru = pGo->SummonCreature(NPC_SPIRIT, 0, 0, 0, 0, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 310000))
                    pMarkAI->SetInUse(kragaru);
            }
        }
    }
    return true;
}
void AddSC_elwynn_forest()
{
    Script *pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "go_marshal_haggards_chest";
    pNewScript->GOGetAI = &GetAIgo_marshal_haggards_chest;
    pNewScript->pGOHello = &GOHello_go_marshal_haggards_chest;
    pNewScript->RegisterSelf();
}
