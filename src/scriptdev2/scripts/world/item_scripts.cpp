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
SDName: Item_Scripts
SD%Complete: 100
SDComment: quest 4743
SDCategory: Items
EndScriptData */

/* ContentData
EndContentData */

#include "precompiled.h"
#include "Spell.h"

/*#####
# item_orb_of_draconic_energy
#####*/

enum
{
    SPELL_DOMINION_SOUL     = 16053,
    NPC_EMBERSTRIFE         = 10321
};

bool ItemUse_item_orb_of_draconic_energy(Player* pPlayer, Item* pItem, const SpellCastTargets& pTargets)
{
    Creature* pEmberstrife = GetClosestCreatureWithEntry(pPlayer, NPC_EMBERSTRIFE, 20.0f);
    // If Emberstrife is already mind controled or above 10% HP: force spell cast failure
    if (pEmberstrife && pEmberstrife->HasAura(SPELL_DOMINION_SOUL) 
        || 10 * pEmberstrife->GetHealth() > pEmberstrife->GetMaxHealth())
    {
        pPlayer->SendEquipError(EQUIP_ERR_NONE, pItem, NULL);

        if (const SpellEntry* pSpellInfo = GetSpellStore()->LookupEntry<SpellEntry>(SPELL_DOMINION_SOUL))
            Spell::SendCastResult(pPlayer, pSpellInfo, SPELL_FAILED_TARGET_AURASTATE);

        return true;
     }

    return false;
}

/*#####
# item_arcane_charges
#####*/

enum
{
	SPELL_ARCANE_CHARGES = 45072
};

bool ItemUse_item_arcane_charges(Player* pPlayer, Item* pItem, const SpellCastTargets& /*pTargets*/)
{
	if (pPlayer->IsTaxiFlying())
		return false;

	pPlayer->SendEquipError(EQUIP_ERR_NONE, pItem, nullptr);

	if (const SpellEntry* pSpellInfo = GetSpellStore()->LookupEntry<SpellEntry>(SPELL_ARCANE_CHARGES))
		Spell::SendCastResult(pPlayer, pSpellInfo, SPELL_FAILED_ERROR);

	return true;
}

/*#####
# item_flying_machine
#####*/

bool ItemUse_item_flying_machine(Player* pPlayer, Item* pItem, const SpellCastTargets& /*pTargets*/)
{
	uint32 itemId = pItem->GetEntry();

	if (itemId == 34060)
		if (pPlayer->GetBaseSkillValue(SKILL_RIDING) >= 225)
			return false;

	if (itemId == 34061)
		if (pPlayer->GetBaseSkillValue(SKILL_RIDING) == 300)
			return false;

	debug_log("SD2: Player attempt to use item %u, but did not meet riding requirement", itemId);
	pPlayer->SendEquipError(EQUIP_ERR_CANT_EQUIP_SKILL, pItem, nullptr);
	return true;
}

/*#####
# item_gor_dreks_ointment
#####*/

enum
{
	NPC_TH_DIRE_WOLF = 20748,
	SPELL_GORDREKS_OINTMENT = 32578
};

bool ItemUse_item_gor_dreks_ointment(Player* pPlayer, Item* pItem, const SpellCastTargets& pTargets)
{
	if (pTargets.getUnitTarget() && pTargets.getUnitTarget()->GetTypeId() == TYPEID_UNIT && pTargets.getUnitTarget()->HasAura(SPELL_GORDREKS_OINTMENT))
	{
		pPlayer->SendEquipError(EQUIP_ERR_NONE, pItem, nullptr);

		if (const SpellEntry* pSpellInfo = GetSpellStore()->LookupEntry<SpellEntry>(SPELL_GORDREKS_OINTMENT))
			Spell::SendCastResult(pPlayer, pSpellInfo, SPELL_FAILED_TARGET_AURASTATE);

		return true;
	}

	return false;
}

void AddSC_item_scripts()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "item_orb_of_draconic_energy";
    pNewScript->pItemUse = &ItemUse_item_orb_of_draconic_energy;
    pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "item_arcane_charges";
	pNewScript->pItemUse = &ItemUse_item_arcane_charges;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "item_flying_machine";
	pNewScript->pItemUse = &ItemUse_item_flying_machine;
	pNewScript->RegisterSelf();

	pNewScript = new Script;
	pNewScript->Name = "item_gor_dreks_ointment";
	pNewScript->pItemUse = &ItemUse_item_gor_dreks_ointment;
	pNewScript->RegisterSelf();
}
