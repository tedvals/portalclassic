/*
   Name    : PlayerbotWarriorAI.cpp
   Complete: maybe around 37%
   Author  : Natsukawa
   Version : 0.39
   */
#include "PlayerbotWarriorAI.h"
#include "PlayerbotMgr.h"
#include "../SpellAuras.h"

class PlayerbotAI;
PlayerbotWarriorAI::PlayerbotWarriorAI(Player* const master, Player* const bot, PlayerbotAI* const ai) : PlayerbotClassAI(master, bot, ai)
{
	SHOOT_BOW = m_ai->initSpell(SHOOT_BOW_1); // GENERAL
	SHOOT_GUN = m_ai->initSpell(SHOOT_GUN_1); // GENERAL
	SHOOT_XBOW = m_ai->initSpell(SHOOT_XBOW_1); // GENERAL

	BATTLE_STANCE = m_ai->initSpell(BATTLE_STANCE_1); //ARMS
	CHARGE = m_ai->initSpell(CHARGE_1); //ARMS
	OVERPOWER = m_ai->initSpell(OVERPOWER_1); // ARMS
	HEROIC_STRIKE = m_ai->initSpell(HEROIC_STRIKE_1); //ARMS
	REND = m_ai->initSpell(REND_1); //ARMS
	THUNDER_CLAP = m_ai->initSpell(THUNDER_CLAP_1);  //ARMS
	HAMSTRING = m_ai->initSpell(HAMSTRING_1);  //ARMS
	MOCKING_BLOW = m_ai->initSpell(MOCKING_BLOW_1);  //ARMS
	RETALIATION = m_ai->initSpell(RETALIATION_1);  //ARMS
	SWEEPING_STRIKES = m_ai->initSpell(SWEEPING_STRIKES_1); //ARMS
	MORTAL_STRIKE = m_ai->initSpell(MORTAL_STRIKE_1);  //ARMS
	DEFENSIVE_STANCE = m_ai->initSpell(DEFENSIVE_STANCE_1); //PROTECTION
	SUNDER_ARMOR = m_ai->initSpell(SUNDER_ARMOR_1); //PROTECTION
	TAUNT = m_ai->initSpell(TAUNT_1); //PROTECTION
	SHIELD_BASH = m_ai->initSpell(SHIELD_BASH_1); //PROTECTION
	REVENGE = m_ai->initSpell(REVENGE_1); //PROTECTION
	SHIELD_BLOCK = m_ai->initSpell(SHIELD_BLOCK_1); //PROTECTION
	DISARM = m_ai->initSpell(DISARM_1); //PROTECTION
	SHIELD_WALL = m_ai->initSpell(SHIELD_WALL_1); //PROTECTION
	SHIELD_SLAM = m_ai->initSpell(SHIELD_SLAM_1); //PROTECTION
	CONCUSSION_BLOW = m_ai->initSpell(CONCUSSION_BLOW_1); //PROTECTION
	LAST_STAND = m_ai->initSpell(LAST_STAND_1); //PROTECTION
	BATTLE_SHOUT = m_ai->initSpell(BATTLE_SHOUT_1); //FURY
	DEMORALIZING_SHOUT = m_ai->initSpell(DEMORALIZING_SHOUT_1); //FURY
	CLEAVE = m_ai->initSpell(CLEAVE_1); //FURY
	INTIMIDATING_SHOUT = m_ai->initSpell(INTIMIDATING_SHOUT_1); //FURY
	EXECUTE = m_ai->initSpell(EXECUTE_1); //FURY
	CHALLENGING_SHOUT = m_ai->initSpell(CHALLENGING_SHOUT_1); //FURY
	SLAM = m_ai->initSpell(SLAM_1); //FURY
	BERSERKER_STANCE = m_ai->initSpell(BERSERKER_STANCE_1); //FURY
	INTERCEPT = m_ai->initSpell(INTERCEPT_1); //FURY
	DEATH_WISH = m_ai->initSpell(DEATH_WISH_1); //FURY
	BERSERKER_RAGE = m_ai->initSpell(BERSERKER_RAGE_1); //FURY
	WHIRLWIND = m_ai->initSpell(WHIRLWIND_1); //FURY
	PUMMEL = m_ai->initSpell(PUMMEL_1); //FURY
	BLOODTHIRST = m_ai->initSpell(BLOODTHIRST_1); //FURY
	RECKLESSNESS = m_ai->initSpell(RECKLESSNESS_1); //FURY
	PIERCING_HOWL = m_ai->initSpell(PIERCING_HOWL_1); //FURY

	RECENTLY_BANDAGED = 11196; // first aid check

	// racial
	STONEFORM = m_ai->initSpell(STONEFORM_ALL); // dwarf
	ESCAPE_ARTIST = m_ai->initSpell(ESCAPE_ARTIST_ALL); // gnome
	SHADOWMELD = m_ai->initSpell(SHADOWMELD_ALL); // night elf
	WAR_STOMP = m_ai->initSpell(WAR_STOMP_ALL); // tauren
	BERSERKING = m_ai->initSpell(BERSERKING_ALL); // troll
	WILL_OF_THE_FORSAKEN = m_ai->initSpell(WILL_OF_THE_FORSAKEN_ALL); // undead
	PERCEPTION = m_ai->initSpell(PERCEPTION_ALL); // human
	BLOOD_FURY = m_ai->initSpell(BLOOD_FURY_ALL); // orc

	//Procs
}
PlayerbotWarriorAI::~PlayerbotWarriorAI() {}

CombatManeuverReturns PlayerbotWarriorAI::DoFirstCombatManeuver(Unit* pTarget)
{
	// There are NPCs in BGs and Open World PvP, so don't filter this on PvP scenarios (of course if PvP targets anyone but tank, all bets are off anyway)
	// Wait until the tank says so, until any non-tank gains aggro or X seconds - whichever is shortest
	if (m_ai->GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO)
	{
		if (m_WaitUntil > m_ai->CurrentTime() && m_ai->GroupTankHoldsAggro())
		{
			if (PlayerbotAI::ORDERS_TANK & m_ai->GetCombatOrder())
			{
				if (m_bot->GetCombatDistance(pTarget, true) <= ATTACK_DISTANCE)
				{
					// Set everyone's UpdateAI() waiting to 5 seconds
					m_ai->SetGroupIgnoreUpdateTime(5);
					// Clear their TEMP_WAIT_TANKAGGRO flag
					m_ai->ClearGroupCombatOrder(PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO);
					// Start attacking, force target on current target
					m_ai->Attack(m_ai->GetCurrentTarget());

					// While everyone else is waiting 2 second, we need to build up aggro, so don't return
				}
				else
				{
					// TODO: add check if target is ranged
					return RETURN_NO_ACTION_OK; // wait for target to get nearer
				}
			}
			else
				return RETURN_NO_ACTION_OK; // wait it out
		}
		else
		{
			m_ai->ClearGroupCombatOrder(PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO);
		}
	}

	if (m_ai->GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_OOC)
	{
		if (m_WaitUntil > m_ai->CurrentTime() && !m_ai->IsGroupInCombat())
			return RETURN_NO_ACTION_OK; // wait it out
		else
			m_ai->ClearGroupCombatOrder(PlayerbotAI::ORDERS_TEMP_WAIT_OOC);
	}

	switch (m_ai->GetScenarioType())
	{
	case PlayerbotAI::SCENARIO_PVP_DUEL:
	case PlayerbotAI::SCENARIO_PVP_BG:
	case PlayerbotAI::SCENARIO_PVP_ARENA:
	case PlayerbotAI::SCENARIO_PVP_OPENWORLD:
		//return DoFirstCombatManeuverPVP(pTarget);
	case PlayerbotAI::SCENARIO_PVE:
	case PlayerbotAI::SCENARIO_PVE_ELITE:
	case PlayerbotAI::SCENARIO_PVE_RAID:
	default:
		return DoFirstCombatManeuverPVE(pTarget);
		break;
	}

	return RETURN_NO_ACTION_ERROR;
}

CombatManeuverReturns PlayerbotWarriorAI::DoFirstCombatManeuverPVE(Unit* pTarget)
{
	if (!m_ai)  return RETURN_NO_ACTION_ERROR;
	if (!m_bot) return RETURN_NO_ACTION_ERROR;

	float fTargetDist = m_bot->GetCombatDistance(pTarget, true);

	
		
		if (!m_bot->HasAura(DEFENSIVE_STANCE, EFFECT_INDEX_0) && m_ai->CastSpell(DEFENSIVE_STANCE))
			return RETURN_CONTINUE;
		else if (TAUNT > 0 && m_bot->HasAura(DEFENSIVE_STANCE, EFFECT_INDEX_0) && m_ai->CastSpell(TAUNT, *pTarget))
			return RETURN_FINISHED_FIRST_MOVES;
	
	
	/* if (BERSERKER_STANCE)
	 {
	 if (!m_bot->HasAura(BERSERKER_STANCE, EFFECT_INDEX_0) && m_ai->CastSpell(BERSERKER_STANCE))
	 return RETURN_CONTINUE;
	 if (BLOODRAGE > 0 && m_bot->HasAura(BERSERKER_STANCE, EFFECT_INDEX_0) && m_ai->GetRageAmount() <= 10)
	 return m_ai->CastSpell(BLOODRAGE) ? RETURN_FINISHED_FIRST_MOVES : RETURN_NO_ACTION_ERROR;
	 if (INTERCEPT > 0 && m_bot->HasAura(BERSERKER_STANCE, EFFECT_INDEX_0))
	 {
	 if (fTargetDist < 8.0f)
	 return RETURN_NO_ACTION_OK;
	 else if (fTargetDist > 25.0f)
	 return RETURN_CONTINUE; // wait to come into range
	 else if (INTERCEPT > 0 && m_ai->CastSpell(INTERCEPT, *pTarget))
	 {
	 float x, y, z;
	 pTarget->GetContactPoint(m_bot, x, y, z, 3.666666f);
	 m_bot->Relocate(x, y, z);
	 return RETURN_FINISHED_FIRST_MOVES;
	 }
	 }
	 }

	 if (BATTLE_STANCE)
	 {
	 if (!m_bot->HasAura(BATTLE_STANCE, EFFECT_INDEX_0) && m_ai->CastSpell(BATTLE_STANCE))
	 return RETURN_CONTINUE;
	 if (CHARGE > 0 && m_bot->HasAura(BATTLE_STANCE, EFFECT_INDEX_0))
	 {
	 if (fTargetDist < 8.0f)
	 return RETURN_NO_ACTION_OK;
	 if (fTargetDist > 25.0f)
	 return RETURN_CONTINUE; // wait to come into range
	 else if (CHARGE > 0 && m_ai->CastSpell(CHARGE, *pTarget))
	 {
	 float x, y, z;
	 pTarget->GetContactPoint(m_bot, x, y, z, 3.666666f);
	 m_bot->Relocate(x, y, z);
	 return RETURN_FINISHED_FIRST_MOVES;
	 }
	 }
	 }
*/
	return RETURN_NO_ACTION_OK;
}

// TODO: blatant copy of PVE for now, please PVP-port it
CombatManeuverReturns PlayerbotWarriorAI::DoFirstCombatManeuverPVP(Unit *pTarget)
{
	if (!m_ai)  return RETURN_NO_ACTION_ERROR;
	if (!m_bot) return RETURN_NO_ACTION_ERROR;

	float fTargetDist = m_bot->GetCombatDistance(pTarget, true);

	if (DEFENSIVE_STANCE && (m_ai->GetCombatOrder() & PlayerbotAI::ORDERS_TANK))
	{
		if (!m_bot->HasAura(DEFENSIVE_STANCE, EFFECT_INDEX_0) && m_ai->CastSpell(DEFENSIVE_STANCE))
			return RETURN_CONTINUE;
		else if (TAUNT > 0 && m_bot->HasAura(DEFENSIVE_STANCE, EFFECT_INDEX_0) && m_ai->CastSpell(TAUNT, *pTarget))
			return RETURN_FINISHED_FIRST_MOVES;
	}

	if (BERSERKER_STANCE)
	{
		if (!m_bot->HasAura(BERSERKER_STANCE, EFFECT_INDEX_0) && m_ai->CastSpell(BERSERKER_STANCE))
			return RETURN_CONTINUE;
		if (BLOODRAGE > 0 && m_bot->HasAura(BERSERKER_STANCE, EFFECT_INDEX_0) && m_ai->GetRageAmount() <= 10)
			return m_ai->CastSpell(BLOODRAGE) ? RETURN_FINISHED_FIRST_MOVES : RETURN_NO_ACTION_ERROR;
		if (INTERCEPT > 0 && m_bot->HasAura(BERSERKER_STANCE, EFFECT_INDEX_0))
		{
			if (fTargetDist < 8.0f)
				return RETURN_NO_ACTION_OK;
			else if (fTargetDist > 25.0f)
				return RETURN_CONTINUE; // wait to come into range
			else if (INTERCEPT > 0 && m_ai->CastSpell(INTERCEPT, *pTarget))
			{
				float x, y, z;
				pTarget->GetContactPoint(m_bot, x, y, z, 3.666666f);
				m_bot->Relocate(x, y, z);
				return RETURN_FINISHED_FIRST_MOVES;
			}
		}
	}

	if (BATTLE_STANCE)
	{
		if (!m_bot->HasAura(BATTLE_STANCE, EFFECT_INDEX_0) && m_ai->CastSpell(BATTLE_STANCE))
			return RETURN_CONTINUE;
		if (CHARGE > 0 && m_bot->HasAura(BATTLE_STANCE, EFFECT_INDEX_0))
		{
			if (fTargetDist < 8.0f)
				return RETURN_NO_ACTION_OK;
			if (fTargetDist > 25.0f)
				return RETURN_CONTINUE; // wait to come into range
			else if (CHARGE > 0 && m_ai->CastSpell(CHARGE, *pTarget))
			{
				float x, y, z;
				pTarget->GetContactPoint(m_bot, x, y, z, 3.666666f);
				m_bot->Relocate(x, y, z);
				return RETURN_FINISHED_FIRST_MOVES;
			}
		}
	}

	return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotWarriorAI::DoNextCombatManeuver(Unit *pTarget)
{
	// Face enemy, make sure bot is attacking
	m_ai->FaceTarget(pTarget);

	switch (m_ai->GetScenarioType())
	{
	case PlayerbotAI::SCENARIO_PVP_DUEL:
	case PlayerbotAI::SCENARIO_PVP_BG:
	case PlayerbotAI::SCENARIO_PVP_ARENA:
	case PlayerbotAI::SCENARIO_PVP_OPENWORLD:
		return DoNextCombatManeuverPVP(pTarget);
	case PlayerbotAI::SCENARIO_PVE:
	case PlayerbotAI::SCENARIO_PVE_ELITE:
	case PlayerbotAI::SCENARIO_PVE_RAID:
	default:
		return DoNextCombatManeuverPVE(pTarget);
		break;
	}

	return RETURN_NO_ACTION_ERROR;
}

CombatManeuverReturns PlayerbotWarriorAI::DoNextCombatManeuverPVE(Unit *pTarget)
{
	if (!m_ai)  return RETURN_NO_ACTION_ERROR;
	if (!m_bot) return RETURN_NO_ACTION_ERROR;
	
	//float fTargetDist = m_bot->GetCombatDistance(pTarget, true);
	uint32 spec = m_bot->GetSpec();
	bool meleeReach = m_bot->CanReachWithMeleeAttack(pTarget);
	//If we have devastate it will replace SA in our rotation
	uint32 SUNDER = (DEVASTATE > 0 ? DEVASTATE : SUNDER_ARMOR);
	Unit* victim = pTarget->getVictim();
	

	//Used to determine if this bot is highest on threat
	Unit* newTarget = m_ai->FindAttacker((PlayerbotAI::ATTACKERINFOTYPE) (PlayerbotAI::AIT_VICTIMSELF | PlayerbotAI::AIT_HIGHESTTHREAT), m_bot);

	if (m_bot->getRace() == RACE_UNDEAD && (m_bot->HasAuraType(SPELL_AURA_MOD_FEAR) || m_bot->HasAuraType(SPELL_AURA_MOD_CHARM)) && !m_bot->HasSpellCooldown(WILL_OF_THE_FORSAKEN) && m_ai->CastSpell(WILL_OF_THE_FORSAKEN, *m_bot))
		return RETURN_CONTINUE;
	if (m_bot->getRace() == RACE_TROLL && !m_bot->HasSpellCooldown(BERSERKING) && m_ai->CastSpell(BERSERKING, *m_bot))
		return RETURN_CONTINUE;
	if (m_bot->getRace() == RACE_ORC && !m_bot->HasAura(BLOOD_FURY, EFFECT_INDEX_0) && !m_bot->HasSpellCooldown(BLOOD_FURY) && m_ai->CastSpell(BLOOD_FURY, *m_bot))
		return RETURN_CONTINUE;
	// do shouts, berserker rage, etc...
	//if (BERSERKER_RAGE > 0 && !m_bot->HasAura(BERSERKER_RAGE, EFFECT_INDEX_0))
	//{
	//	m_ai->CastSpell(BERSERKER_RAGE); 
	//return RETURN_CONTINUE;
	//}
	//else 
	/*if (BLOODRAGE > 0 && m_ai->GetRageAmount() <= 10 && !m_bot->HasSpellCooldown(BLOODRAGE))
{
m_ai->CastSpell(BLOODRAGE);
return RETURN_CONTINUE;
}*/
	//check buff in combat
	if (m_bot->getLevel() == 60)
	{
		if (m_ai->GetHealthPercent() < 30)
		{
			//dodge
			if (!m_bot->HasAura(16321) && !m_bot->HasSpellCooldown(16321) && m_ai->CastSpell(16321, *m_bot))
				return RETURN_CONTINUE;
			//heal crystal
			if (!m_bot->HasAura(15229) && !m_bot->HasSpellCooldown(15229) && m_ai->CastSpell(15229, *m_bot))
				return RETURN_CONTINUE;
		}
		//attack speed
		if (!m_bot->HasAura(16323) && !m_bot->HasSpellCooldown(16323) && m_ai->CastSpell(16323, *m_bot))
			return RETURN_CONTINUE;
	}
	//use healing posion
	if (m_ai->GetHealthPercent() < 20)
	{
		
		Item* healingPosion = m_ai->FindHealingPoison();
		if (healingPosion)
			m_ai->UseItem(healingPosion);
	}

	if (WAR_STOMP > 0 && !m_bot->HasSpellCooldown(WAR_STOMP) && meleeReach && m_ai->CastSpell(WAR_STOMP, *pTarget))
		return RETURN_CONTINUE;

	CheckShouts();

	usetrinkets();

	switch (spec)
	{
	case WARRIOR_SPEC_ARMS:
		// Execute doesn't scale too well with extra rage and uses up *all* rage preventing use of other skills
		//Haven't found a way to make sudden death work yet, either wrong spell or it needs an effect index(probably)
		if (EXECUTE > 0 && pTarget->GetHealthPercent() < 20 && m_ai->CastSpell(EXECUTE, *pTarget))
			return RETURN_CONTINUE;
		if (REND > 0 && !pTarget->HasAura(REND, EFFECT_INDEX_0) && m_ai->CastSpell(REND, *pTarget))
			return RETURN_CONTINUE;
		if (MORTAL_STRIKE > 0 && !m_bot->HasSpellCooldown(MORTAL_STRIKE) && m_ai->CastSpell(MORTAL_STRIKE, *pTarget))
			return RETURN_CONTINUE;
		// No way to tell if overpower is active (yet), however taste for blood works
		if (OVERPOWER > 0 && m_ai->CastSpell(OVERPOWER, *pTarget))
			return RETURN_CONTINUE;
		if (HEROIC_STRIKE > 0 && m_ai->CastSpell(HEROIC_STRIKE, *pTarget))
			return RETURN_CONTINUE;
		if (SLAM > 0 && m_ai->CastSpell(SLAM, *pTarget))
		{
			m_ai->SetIgnoreUpdateTime(1.5); // TODO: SetIgnoreUpdateTime takes a uint8 - how will 1.5 work as a value?
			return RETURN_CONTINUE;
		}
		break;
	case WARRIOR_SPEC_FURY:
		if (EXECUTE > 0 && pTarget->GetHealthPercent() < 20 && m_ai->CastSpell(EXECUTE, *pTarget))
			return RETURN_CONTINUE;
		if (BLOODTHIRST > 0 && !m_bot->HasSpellCooldown(BLOODTHIRST) && m_ai->CastSpell(BLOODTHIRST, *pTarget))
			return RETURN_CONTINUE;
		if (WHIRLWIND > 0 && !m_bot->HasSpellCooldown(WHIRLWIND) && m_ai->CastSpell(WHIRLWIND, *pTarget))
			return RETURN_CONTINUE;
		if (HEROIC_STRIKE > 0 && m_ai->CastSpell(HEROIC_STRIKE, *pTarget))
			return RETURN_CONTINUE;
		break;
	case WARRIOR_SPEC_PROTECTION:
		if (!m_bot->HasAura(DEFENSIVE_STANCE, EFFECT_INDEX_0) && m_ai->CastSpell(DEFENSIVE_STANCE))
			return RETURN_CONTINUE;
		if (!newTarget && victim && pTarget->GetEntry() != 15348)
		{
			// Cast taunt on bot current target if the mob is targeting someone else
			if (m_ai->GetCombatOrder() & PlayerbotAI::ORDERS_TANK && TAUNT > 0 && !m_bot->HasSpellCooldown(TAUNT)  && m_ai->CastSpell(TAUNT, *pTarget))
				return RETURN_CONTINUE;
			if (CHALLENGING_SHOUT > 0 && m_ai->GetAttackerCount() >= 2 && !m_bot->HasSpellCooldown(CHALLENGING_SHOUT) && m_ai->CastSpell(CHALLENGING_SHOUT, *pTarget))
				return RETURN_CONTINUE;
			if (CONCUSSION_BLOW > 0 && !m_bot->HasSpellCooldown(CONCUSSION_BLOW) && m_ai->CastSpell(CONCUSSION_BLOW, *pTarget))
				return RETURN_CONTINUE;
		}

		if (!newTarget && victim && pTarget->GetEntry() == 15348)
		{
			// Cast taunt on bot current target if the mob is targeting someone else
			if (m_ai->GetCombatOrder() & PlayerbotAI::ORDERS_TANK && TAUNT > 0 && !m_bot->HasSpellCooldown(TAUNT) && m_ai->HasAuraStackAmount(25646,*victim)>=3 && m_ai->CastSpell(TAUNT, *pTarget))
				return RETURN_CONTINUE;
			if (CHALLENGING_SHOUT > 0 && m_ai->GetAttackerCount() >= 2 && !m_bot->HasSpellCooldown(CHALLENGING_SHOUT) && m_ai->CastSpell(CHALLENGING_SHOUT, *pTarget))
				return RETURN_CONTINUE;
			if (CONCUSSION_BLOW > 0 && !m_bot->HasSpellCooldown(CONCUSSION_BLOW) && m_ai->CastSpell(CONCUSSION_BLOW, *pTarget))
				return RETURN_CONTINUE;
		}

		// TODO: should behaviour (or treshold) be different between elite and normal mobs? We don't want bots to burn such precious cooldown needlessly 
		if (m_bot->GetHealthPercent() < 40)
		{
			// Cast Last Stand first because it has lower cooldown
			if (LAST_STAND > 0 && !m_bot->HasAura(LAST_STAND, EFFECT_INDEX_0) && !m_bot->HasSpellCooldown(LAST_STAND) && m_ai->CastSpell(LAST_STAND, *m_bot))
			{
				m_ai->TellMaster("I'm using LAST STAND");
				return RETURN_CONTINUE;
			}
			// Cast Shield Wall only if Last Stand is on cooldown and not active
			if (SHIELD_WALL > 0 && (m_bot->HasSpellCooldown(LAST_STAND) || LAST_STAND == 0) && !m_bot->HasAura(LAST_STAND, EFFECT_INDEX_0) && !m_bot->HasAura(SHIELD_WALL, EFFECT_INDEX_0) && m_ai->CastSpell(SHIELD_WALL, *m_bot))
			{
				m_ai->TellMaster("I'm using SHIELD WALL");
				return RETURN_CONTINUE;
			}
		}
		if (SHIELD_BASH > 0 && m_ai->GetRageAmount() >= 10 && !m_bot->HasSpellCooldown(SHIELD_BASH) && pTarget->IsNonMeleeSpellCasted(true)  && m_ai->CastSpell(SHIELD_BASH, *pTarget))
			return RETURN_CONTINUE;
		if (REVENGE > 0 && !m_bot->HasSpellCooldown(REVENGE))
		{
			uint8 base = pTarget->RollMeleeOutcomeAgainst(m_bot, BASE_ATTACK);
			uint8 off = pTarget->RollMeleeOutcomeAgainst(m_bot, OFF_ATTACK);
			if (base == MELEE_HIT_PARRY || base == MELEE_HIT_DODGE || base == MELEE_HIT_BLOCK || off == MELEE_HIT_PARRY || off == MELEE_HIT_DODGE || off == MELEE_HIT_BLOCK)
				if (m_ai->CastSpell(REVENGE, *pTarget))
					return RETURN_CONTINUE;

		}
		if (DISARM > 0 && m_ai->GetRageAmount() >= 20 && !pTarget->HasAura(DISARM, EFFECT_INDEX_0) && !m_bot->HasSpellCooldown(DISARM) && m_ai->CastSpell(DISARM, *pTarget))
			return RETURN_CONTINUE;
		if (DEMORALIZING_SHOUT > 0 && m_ai->GetRageAmount() >= 10 && !pTarget->HasAura(DEMORALIZING_SHOUT, EFFECT_INDEX_0) && m_ai->CastSpell(DEMORALIZING_SHOUT, *pTarget))
			return RETURN_CONTINUE;
		if (SHIELD_BLOCK > 0 && m_ai->GetRageAmount() >= 10 && !m_bot->HasAura(SHIELD_BLOCK, EFFECT_INDEX_0) && !m_bot->HasSpellCooldown(SHIELD_BLOCK) && m_ai->GetHealthPercent() < 80 && m_ai->CastSpell(SHIELD_BLOCK, *m_bot))
			return RETURN_CONTINUE;
		if (SHIELD_SLAM > 0 && m_ai->GetRageAmount() >= 20 && !m_bot->HasSpellCooldown(SHIELD_SLAM) && m_ai->CastSpell(SHIELD_SLAM, *pTarget))
			return RETURN_CONTINUE;
		if (SUNDER > 0 && m_ai->CastSpell(SUNDER, *pTarget))
			return RETURN_CONTINUE;
		if (HEROIC_STRIKE > 0 && m_ai->CastSpell(HEROIC_STRIKE, *pTarget))
			return RETURN_CONTINUE;
		break;
		
	}

	return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotWarriorAI::DoNextCombatManeuverPVP(Unit* pTarget)
{
	if (m_ai->CastSpell(HEROIC_STRIKE))
		return RETURN_CONTINUE;

	return DoNextCombatManeuverPVE(pTarget); // TODO: bad idea perhaps, but better than the alternative
}

//Buff and rebuff shouts
void PlayerbotWarriorAI::CheckShouts()
{
	if (!m_ai)  return;
	if (!m_bot) return;


	if (!m_bot->HasAura(BATTLE_SHOUT, EFFECT_INDEX_0) && m_ai->CastSpell(BATTLE_SHOUT))
		return;

}

void PlayerbotWarriorAI::DoNonCombatActions()
{
	if (!m_ai)  return;
	if (!m_bot) return;
	m_bot->RemoveAllSpellCooldown();
	uint32 spec = m_bot->GetSpec();
	//check buff
	if (m_bot->getLevel() == 60)
	{   
		//str
		if (!m_bot->HasAura(16323))
			m_ai->CastSpell(16323);
	    //fire r
		if (!m_bot->HasAura(16326))
			m_ai->CastSpell(16326);
		//attack power
		if (!m_bot->HasAura(16329))
			m_ai->CastSpell(16329);
		//forst r
		if (!m_bot->HasAura(16325))
			m_ai->CastSpell(16325);
		//Crystal Spire 12 damage when attack
		if (!m_bot->HasAura(15279))
			m_ai->CastSpell(15279);
		//Armor
		if (!m_bot->HasAura(15233))
			m_ai->CastSpell(15233);
		//Songflower Serenade
		if (!m_bot->HasAura(15366))
			m_ai->CastSpell(15366);
		//sta stone
		if (!m_bot->HasAura(30090))
			m_ai->CastSpell(30090);
		//zanza sta spi posion
		if (!m_bot->HasAura(24382))
			m_ai->CastSpell(24382);
		//hp r
		if (!m_bot->HasAura(24361))
			m_ai->CastSpell(24361);
		//sta 
		if (!m_bot->HasAura(10668))
			m_ai->CastSpell(10668);
	}
	//Stance Check
	if (spec == WARRIOR_SPEC_ARMS && !m_bot->HasAura(BATTLE_STANCE, EFFECT_INDEX_0))
		m_ai->CastSpell(BATTLE_STANCE);
	else if (spec == WARRIOR_SPEC_FURY && !m_bot->HasAura(BERSERKER_STANCE, EFFECT_INDEX_0))
		m_ai->CastSpell(BERSERKER_STANCE);
	else if (spec == WARRIOR_SPEC_PROTECTION && !m_bot->HasAura(DEFENSIVE_STANCE, EFFECT_INDEX_0))
		m_ai->CastSpell(DEFENSIVE_STANCE);

	if (EatDrinkBandage())
		return;
	// hp/mana check
	//if (m_bot->getStandState() != UNIT_STAND_STATE_STAND)
	//	m_bot->SetStandState(UNIT_STAND_STATE_STAND);
} // end DoNonCombatActions

// Match up with "Pull()" below
bool PlayerbotWarriorAI::CanPull()
{
	if (!m_bot) return false;
	if (!m_ai) return false;

	if (m_bot->GetUInt32Value(PLAYER_AMMO_ID)) // Having ammo equipped means a weapon is equipped as well. Probably. [TODO: does this work with throwing knives? Can a playerbot 'cheat' ammo into the slot without a proper weapon?]
	{
		// Can't do this, CanPull CANNOT check for anything that requires a target
		//if (!m_ai->IsInRange(m_ai->GetCurrentTarget(), AUTO_SHOT))
		//{
		//    m_ai->TellMaster("I'm out of range.");
		//    return false;
		// }
		return true;
	}

	return false;
}

// Match up with "CanPull()" above
bool PlayerbotWarriorAI::Pull()
{
	if (!m_bot) return false;
	if (!m_ai)  return false;
	// In Classic, Warriors had 3 differents spells for shooting with range weapons
	// So we need to determine which one to use
	// First step: look for the item equiped in range slot
	Item* pItem = m_bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
	if (!pItem)
	{
		m_ai->TellMaster("I don't have ranged weapon equiped.");
		return false;
	}

	ItemPrototype const* pProto = pItem->GetProto();
	if (pProto && pProto->Class == ITEM_CLASS_WEAPON)
	{
		switch (pProto->SubClass)
		{
		case ITEM_SUBCLASS_WEAPON_BOW:
			SHOOT = SHOOT_BOW;
			break;
		case ITEM_SUBCLASS_WEAPON_GUN:
			SHOOT = SHOOT_GUN;
			break;
		case ITEM_SUBCLASS_WEAPON_CROSSBOW:
			SHOOT = SHOOT_XBOW;
			break;
		default:
			m_ai->TellMaster("Can't pull: equiped range item is neither a gun, bow or crossbow.");
			return false;
		}
	}
	else
		return false;

	if (m_bot->GetCombatDistance(m_ai->GetCurrentTarget(), true) > ATTACK_DISTANCE)
	{
		if (!m_ai->In_Reach(m_ai->GetCurrentTarget(), SHOOT))
		{
			m_ai->TellMaster("I'm out of range.");
			return false;
		}

		// shoot at the target
		m_bot->CastSpell(m_ai->GetCurrentTarget(), SHOOT, true);
		m_ai->TellMaster("I'm PULLING %s.", m_ai->GetCurrentTarget()->GetName());
		return true;
	}
	else // target is in melee range
	{
		m_ai->Attack(m_ai->GetCurrentTarget());
		return true;
	}

	return false;
}
