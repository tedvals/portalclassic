
#include "PlayerbotMageAI.h"

class PlayerbotAI;

PlayerbotMageAI::PlayerbotMageAI(Player* const master, Player* const bot, PlayerbotAI* const ai) : PlayerbotClassAI(master, bot, ai)
{
    ARCANE_MISSILES         = m_ai->initSpell(ARCANE_MISSILES_1);
    ARCANE_EXPLOSION        = m_ai->initSpell(ARCANE_EXPLOSION_1);
    COUNTERSPELL            = m_ai->initSpell(COUNTERSPELL_1);
    SLOW                    = m_ai->initSpell(SLOW_1);
    ARCANE_BARRAGE          = m_ai->initSpell(ARCANE_BARRAGE_1);
    ARCANE_BLAST            = m_ai->initSpell(ARCANE_BLAST_1);
    ARCANE_POWER            = m_ai->initSpell(ARCANE_POWER_1);
    DAMPEN_MAGIC            = m_ai->initSpell(DAMPEN_MAGIC_1);
    AMPLIFY_MAGIC           = m_ai->initSpell(AMPLIFY_MAGIC_1);
    MAGE_ARMOR              = m_ai->initSpell(MAGE_ARMOR_1);
    MIRROR_IMAGE            = m_ai->initSpell(MIRROR_IMAGE_1);
    ARCANE_INTELLECT        = m_ai->initSpell(ARCANE_INTELLECT_1);
    ARCANE_BRILLIANCE       = m_ai->initSpell(ARCANE_BRILLIANCE_1);
    DALARAN_INTELLECT       = m_ai->initSpell(DALARAN_INTELLECT_1);
    DALARAN_BRILLIANCE      = m_ai->initSpell(DALARAN_BRILLIANCE_1);
    MANA_SHIELD             = m_ai->initSpell(MANA_SHIELD_1);
    CONJURE_WATER           = m_ai->initSpell(CONJURE_WATER_1);
    CONJURE_FOOD            = m_ai->initSpell(CONJURE_FOOD_1);
    FIREBALL                = m_ai->initSpell(FIREBALL_1);
    FIRE_BLAST              = m_ai->initSpell(FIRE_BLAST_1);
    FLAMESTRIKE             = m_ai->initSpell(FLAMESTRIKE_1);
    SCORCH                  = m_ai->initSpell(SCORCH_1);
    PYROBLAST               = m_ai->initSpell(PYROBLAST_1);
    BLAST_WAVE              = m_ai->initSpell(BLAST_WAVE_1);
    COMBUSTION              = m_ai->initSpell(COMBUSTION_1);
    DRAGONS_BREATH          = m_ai->initSpell(DRAGONS_BREATH_1);
    LIVING_BOMB             = m_ai->initSpell(LIVING_BOMB_1);
    FROSTFIRE_BOLT          = m_ai->initSpell(FROSTFIRE_BOLT_1);
    FIRE_WARD               = m_ai->initSpell(FIRE_WARD_1);
    MOLTEN_ARMOR            = m_ai->initSpell(MOLTEN_ARMOR_1);
    ICY_VEINS               = m_ai->initSpell(ICY_VEINS_1);
    DEEP_FREEZE             = m_ai->initSpell(DEEP_FREEZE_1);
    FROSTBOLT               = m_ai->initSpell(FROSTBOLT_1);
    FROST_NOVA              = m_ai->initSpell(FROST_NOVA_1);
    BLIZZARD                = m_ai->initSpell(BLIZZARD_1);
    CONE_OF_COLD            = m_ai->initSpell(CONE_OF_COLD_1);
    ICE_BARRIER             = m_ai->initSpell(ICE_BARRIER_1);
    SUMMON_WATER_ELEMENTAL  = m_ai->initSpell(SUMMON_WATER_ELEMENTAL_1);
    FROST_WARD              = m_ai->initSpell(FROST_WARD_1);
    ICE_LANCE               = m_ai->initSpell(ICE_LANCE_1);
    FROST_ARMOR             = m_ai->initSpell(FROST_ARMOR_1);
    ICE_ARMOR               = m_ai->initSpell(ICE_ARMOR_1);
    ICE_BLOCK               = m_ai->initSpell(ICE_BLOCK_1);
    COLD_SNAP               = m_ai->initSpell(COLD_SNAP_1);
	CONJURE_MANA_GEM        = m_ai->initSpell(CONJURE_MANA_GEM_1);
	CONJURE_MANA_JADE       = m_ai->initSpell(CONJURE_MANA_GEM_2);
	CONJURE_MANA_CITRINE    = m_ai->initSpell(CONJURE_MANA_GEM_3);
	EVOCATION               = m_ai->initSpell(EVOCATION_1);
    // RANGED COMBAT
    SHOOT                   = m_ai->initSpell(SHOOT_2);

    RECENTLY_BANDAGED       = 11196; // first aid check

    // racial
    ARCANE_TORRENT          = m_ai->initSpell(ARCANE_TORRENT_MANA_CLASSES); // blood elf
    GIFT_OF_THE_NAARU       = m_ai->initSpell(GIFT_OF_THE_NAARU_MAGE); // draenei
    ESCAPE_ARTIST           = m_ai->initSpell(ESCAPE_ARTIST_ALL); // gnome
    EVERY_MAN_FOR_HIMSELF   = m_ai->initSpell(EVERY_MAN_FOR_HIMSELF_ALL); // human
    BERSERKING              = m_ai->initSpell(BERSERKING_ALL); // troll
    WILL_OF_THE_FORSAKEN    = m_ai->initSpell(WILL_OF_THE_FORSAKEN_ALL); // undead
}

PlayerbotMageAI::~PlayerbotMageAI() {}

CombatManeuverReturns PlayerbotMageAI::DoFirstCombatManeuver(Unit* pTarget)
{
    // There are NPCs in BGs and Open World PvP, so don't filter this on PvP scenarios (of course if PvP targets anyone but tank, all bets are off anyway)
    // Wait until the tank says so, until any non-tank gains aggro or X seconds - whichever is shortest
    if (m_ai->GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO)
    {
        if (m_WaitUntil > m_ai->CurrentTime() && m_ai->GroupTankHoldsAggro())
        {
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
            return DoFirstCombatManeuverPVP(pTarget);
        case PlayerbotAI::SCENARIO_PVE:
        case PlayerbotAI::SCENARIO_PVE_ELITE:
        case PlayerbotAI::SCENARIO_PVE_RAID:
        default:
            return DoFirstCombatManeuverPVE(pTarget);
            break;
    }

    return RETURN_NO_ACTION_ERROR;
}

CombatManeuverReturns PlayerbotMageAI::DoFirstCombatManeuverPVE(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotMageAI::DoFirstCombatManeuverPVP(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotMageAI::DoNextCombatManeuver(Unit *pTarget)
{
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

CombatManeuverReturns PlayerbotMageAI::DoNextCombatManeuverPVE(Unit *pTarget)
{
    if (!m_ai)  return RETURN_NO_ACTION_ERROR;
    if (!m_bot) return RETURN_NO_ACTION_ERROR;

    Unit* pVictim = pTarget->getVictim();
    bool meleeReach = m_bot->CanReachWithMeleeAttack(pTarget);

    uint32 spec = m_bot->GetSpec();
	
    if (m_ai->GetCombatStyle() != PlayerbotAI::COMBAT_RANGED && !meleeReach)
        m_ai->SetCombatStyle(PlayerbotAI::COMBAT_RANGED);
    // if can't shoot OR have no ranged (wand) equipped
    //else if(m_ai->GetCombatStyle() != PlayerbotAI::COMBAT_MELEE && (SHOOT == 0 || !m_bot->GetWeaponForAttack(RANGED_ATTACK, true, true)))
        //m_ai->SetCombatStyle(PlayerbotAI::COMBAT_MELEE);
	if (m_bot->getRace() == RACE_UNDEAD && (m_bot->HasAuraType(SPELL_AURA_MOD_FEAR) || m_bot->HasAuraType(SPELL_AURA_MOD_CHARM)) && !m_bot->HasSpellCooldown(WILL_OF_THE_FORSAKEN) && m_ai->CastSpell(WILL_OF_THE_FORSAKEN, *m_bot))
		return RETURN_CONTINUE;
	
	if (m_ai->GetManaPercent() < 30)
	{
		Item* ManaStone = m_ai->FindConsumable(ManaAgateSTONE_DISPLAYID);
		Item* ManaStone2 = m_ai->FindConsumable(ManaJadeSTONE_DISPLAYID);
		Item* ManaStone3 = m_ai->FindConsumable(ManaCitrineSTONE_DISPLAYID);
		if (ManaStone3 && !m_bot->HasSpellCooldown(10057))
		{
			m_ai->UseItem(ManaStone3);
			return RETURN_CONTINUE;
		}
		if (ManaStone2 && !m_bot->HasSpellCooldown(10052))
		{
			m_ai->UseItem(ManaStone2);
			return RETURN_CONTINUE;
		}
		if (ManaStone && !m_bot->HasSpellCooldown(5405))
		{
			m_ai->UseItem(ManaStone);
			return RETURN_CONTINUE;
		}

	}
	
	if (EVOCATION > 0 && m_ai->GetManaPercent() < 10 && !m_bot->HasSpellCooldown(EVOCATION) && CastSpell(EVOCATION, m_bot))
	{
		m_ai->SetIgnoreUpdateTime(8);
		return RETURN_CONTINUE;
	}
    //Used to determine if this bot is highest on threat
    Unit *newTarget = m_ai->FindAttacker((PlayerbotAI::ATTACKERINFOTYPE) (PlayerbotAI::AIT_VICTIMSELF | PlayerbotAI::AIT_HIGHESTTHREAT), m_bot);
    if (newTarget) // TODO: && party has a tank
    {
		switch (spec)
		{
		case MAGE_SPEC_FIRE:
			// Insert instant threat reducing spell (if a mage has one)
			if (m_ai->GetAttackerCount() >= 5)
			{
				m_bot->GetMotionMaster()->MoveFollow(pTarget, 6.0f, m_bot->GetOrientation());
				if (FROST_NOVA > 0 && meleeReach && !m_bot->HasSpellCooldown(FROST_NOVA) && CastSpell(FROST_NOVA, pTarget))
					return RETURN_CONTINUE;
				if (BLAST_WAVE > 0 && meleeReach && !m_bot->HasSpellCooldown(BLAST_WAVE) && CastSpell(BLAST_WAVE, pTarget))
					return RETURN_CONTINUE;
				//if (FLAMESTRIKE > 0 && m_ai->In_Reach(pTarget, FLAMESTRIKE) && CastSpell(FLAMESTRIKE, pTarget));
				if (ARCANE_EXPLOSION>0 && CastSpell(ARCANE_EXPLOSION, pTarget))
				{
					return RETURN_CONTINUE;
				}
			}
			// Have threat, can't quickly lower it. 3 options remain: Stop attacking, lowlevel damage (wand), keep on keeping on.
			if (newTarget->GetHealthPercent() > 25)
			{
				// If elite, do nothing and pray tank gets aggro off you
				// TODO: Is there an IsElite function? If so, find it and insert.
				//if (newTarget->IsElite())
				//    return;

				// Not an elite. You could insert FEAR here but in any PvE situation that's 90-95% likely
				// to worsen the situation for the group. ... So please don't.
				CastSpell(SHOOT, pTarget);
				return RETURN_CONTINUE;
			}
			break;
		case MAGE_SPEC_FROST:
			if (m_ai->GetAttackerCount() >= 5)
			{
				//m_bot->GetMotionMaster()->MoveFollow(pTarget, 6.0f, m_bot->GetOrientation());
				if (FROST_NOVA > 0 && meleeReach && !m_bot->HasSpellCooldown(FROST_NOVA) && CastSpell(FROST_NOVA, pTarget))
					return RETURN_CONTINUE;
				if (CONE_OF_COLD > 0 && meleeReach && !m_bot->HasSpellCooldown(CONE_OF_COLD) && CastSpell(CONE_OF_COLD, pTarget))
					return RETURN_CONTINUE;
				if (ARCANE_EXPLOSION > 0 && meleeReach && CastSpell(ARCANE_EXPLOSION, pTarget))
					return RETURN_CONTINUE;
				if (BLIZZARD>0 && CastSpell(BLIZZARD, pTarget))
				{
					m_ai->SetIgnoreUpdateTime(8);
					return RETURN_CONTINUE;
				}
			}
			if (newTarget->GetHealthPercent() > 25)
			{
				// If elite, do nothing and pray tank gets aggro off you
				// TODO: Is there an IsElite function? If so, find it and insert.
				//if (newTarget->IsElite())
				//    return;

				// Not an elite. You could insert FEAR here but in any PvE situation that's 90-95% likely
				// to worsen the situation for the group. ... So please don't.
				CastSpell(SHOOT, pTarget);
				return RETURN_CONTINUE;
			}
			break;
		}

    }

    switch (spec)
    {
        case MAGE_SPEC_FROST:
            if (COLD_SNAP > 0 && m_ai->In_Reach(m_bot, COLD_SNAP) && m_ai->GetHealthPercent() < 30 && m_bot->HasSpellCooldown(ICE_BLOCK) && CastSpell(COLD_SNAP, m_bot))
				return RETURN_CONTINUE;
			if (ICE_BLOCK > 0 && m_ai->In_Reach(m_bot, ICE_BLOCK) && !m_bot->HasAura(ICE_BLOCK, EFFECT_INDEX_0) && m_ai->GetHealthPercent() < 30 && !m_bot->HasSpellCooldown(ICE_BLOCK) && CastSpell(ICE_BLOCK, m_bot))
			{
				m_ai->SetIgnoreUpdateTime(10);
				return RETURN_CONTINUE;
			}
			if (ICE_BARRIER > 0 && m_ai->In_Reach(m_bot, ICE_BARRIER) && !m_bot->HasAura(ICE_BARRIER, EFFECT_INDEX_0) && !m_bot->HasSpellCooldown(ICE_BARRIER) && CastSpell(ICE_BARRIER, m_bot))
                return RETURN_CONTINUE;
			if (m_ai->GetAttackerCount() >= 5)
			{
				//m_bot->GetMotionMaster()->MoveFollow(pTarget, 6.0f, m_bot->GetOrientation());
				if (FROST_NOVA > 0 && meleeReach && !m_bot->HasSpellCooldown(FROST_NOVA) && CastSpell(FROST_NOVA, pTarget))
					return RETURN_CONTINUE;
				if (CONE_OF_COLD > 0 && meleeReach && !m_bot->HasSpellCooldown(CONE_OF_COLD) && CastSpell(CONE_OF_COLD, pTarget))
					return RETURN_CONTINUE;
				if (ARCANE_EXPLOSION > 0 && meleeReach && CastSpell(ARCANE_EXPLOSION, pTarget))
					return RETURN_CONTINUE;
				if (BLIZZARD>0 && CastSpell(BLIZZARD, pTarget))
				{
					m_ai->SetIgnoreUpdateTime(8);
					return RETURN_CONTINUE;
				}
			}
			if (FROST_WARD > 0 && m_ai->In_Reach(m_bot, FROST_WARD) && !m_bot->HasAura(FROST_WARD, EFFECT_INDEX_0) && !m_bot->HasSpellCooldown(FROST_WARD) && CastSpell(FROST_WARD, m_bot))
                return RETURN_CONTINUE;
			//if (FROSTBOLT > 0 && CastSpell(FROSTBOLT, pTarget))
                //return RETURN_CONTINUE;
           if (FROSTBOLT > 0 && m_ai->In_Reach(pTarget,FROSTBOLT))
           return CastSpell(FROSTBOLT, pTarget);
            break;

        case MAGE_SPEC_FIRE:
			if (m_ai->GetAttackerCount() >= 5)
			{
				m_bot->GetMotionMaster()->MoveFollow(pTarget, 6.0f, m_bot->GetOrientation());
				if (FROST_NOVA > 0 && meleeReach && !m_bot->HasSpellCooldown(FROST_NOVA) && CastSpell(FROST_NOVA, pTarget))
					return RETURN_CONTINUE;
				if (BLAST_WAVE > 0 && meleeReach && !m_bot->HasSpellCooldown(BLAST_WAVE) && CastSpell(BLAST_WAVE, pTarget))
					return RETURN_CONTINUE;
				//if (FLAMESTRIKE > 0 && m_ai->In_Reach(pTarget, FLAMESTRIKE) && CastSpell(FLAMESTRIKE, pTarget));
				if (ARCANE_EXPLOSION>0 && CastSpell(ARCANE_EXPLOSION, pTarget))
				{
					return RETURN_CONTINUE;
				}
			}
			if (FIRE_WARD > 0 && m_ai->In_Reach(m_bot, FIRE_WARD) && !m_bot->HasAura(FIRE_WARD, EFFECT_INDEX_0) && !m_bot->HasSpellCooldown(FIRE_WARD) && CastSpell(FIRE_WARD, m_bot))
				return RETURN_CONTINUE;
			if (SCORCH > 0 && m_ai->In_Reach(pTarget, SCORCH) && SpellSequence < 6 && CastSpell(SCORCH, pTarget))
			{
				SpellSequence = SpellSequence + 1;
				return RETURN_CONTINUE;
			}
			if (SCORCH > 0 && m_ai->In_Reach(pTarget, SCORCH) && !pTarget->HasAura(22959) && CastSpell(SCORCH, pTarget))
				{
					SpellSequence = 0;
					return RETURN_CONTINUE;
				}
			if (COMBUSTION > 0 && m_ai->In_Reach(m_bot, COMBUSTION) && !m_bot->HasAura(COMBUSTION, EFFECT_INDEX_0) && !m_bot->HasSpellCooldown(COMBUSTION) && CastSpell(COMBUSTION, m_bot))
				return RETURN_CONTINUE;
			if (PYROBLAST > 0 && m_ai->In_Reach(pTarget, PYROBLAST) && m_bot->HasAura(12536) && CastSpell(PYROBLAST, pTarget))
				return RETURN_CONTINUE;
			if (FIRE_BLAST > 0 && m_ai->In_Reach(pTarget, FIRE_BLAST) && !m_bot->HasSpellCooldown(FIRE_BLAST) && CastSpell(FIRE_BLAST, pTarget))
				return RETURN_CONTINUE;
			//if (FIREBALL > 0 && m_ai->In_Reach(pTarget, FIREBALL) && CastSpell(FIREBALL, pTarget))
				//return RETURN_CONTINUE;
			if (FIREBALL > 0 && m_ai->In_Reach(pTarget, FIREBALL))
				return CastSpell(FIREBALL, pTarget);
			break;
			
        case MAGE_SPEC_ARCANE:
            if (ARCANE_POWER > 0 && m_ai->In_Reach(pTarget,ARCANE_POWER) && CastSpell(ARCANE_POWER, pTarget))
                return RETURN_CONTINUE;
            if (ARCANE_MISSILES > 0 && m_ai->In_Reach(pTarget,ARCANE_MISSILES) && CastSpell(ARCANE_MISSILES, pTarget))
            {
                m_ai->SetIgnoreUpdateTime(3);
                return RETURN_CONTINUE;
            }
			if (m_ai->GetAttackerCount() >= 5)
			{
				if (FROST_NOVA > 0 && meleeReach   && !m_bot->HasSpellCooldown(FROST_NOVA) && CastSpell(FROST_NOVA, pTarget));
				//if (FLAMESTRIKE > 0 && m_ai->In_Reach(pTarget, FLAMESTRIKE) && CastSpell(FLAMESTRIKE, pTarget));

				m_bot->GetMotionMaster()->MoveFollow(pTarget, 6.0f, m_bot->GetOrientation());
				//return RETURN_FINISHED_FIRST_MOVES;
				if (meleeReach && CastSpell(ARCANE_EXPLOSION, pTarget))
				{
					return RETURN_CONTINUE;
				}
			}
            if (COUNTERSPELL > 0 && pTarget->IsNonMeleeSpellCasted(true) && CastSpell(COUNTERSPELL, pTarget))
                return RETURN_CONTINUE;
            if (SLOW > 0 && m_ai->In_Reach(pTarget,SLOW) && !pTarget->HasAura(SLOW, EFFECT_INDEX_0) && CastSpell(SLOW, pTarget))
                return RETURN_CONTINUE;
            if (ARCANE_BARRAGE > 0 && m_ai->In_Reach(pTarget,ARCANE_BARRAGE) && CastSpell(ARCANE_BARRAGE, pTarget))
                return RETURN_CONTINUE;
            if (ARCANE_BLAST > 0 && m_ai->In_Reach(pTarget,ARCANE_BLAST) && CastSpell(ARCANE_BLAST, pTarget))
                return RETURN_CONTINUE;
            if (MIRROR_IMAGE > 0 && m_ai->In_Reach(pTarget,MIRROR_IMAGE) && CastSpell(MIRROR_IMAGE))
                return RETURN_CONTINUE;
            if (MANA_SHIELD > 0 && m_ai->GetHealthPercent() < 70 && pVictim == m_bot && !m_bot->HasAura(MANA_SHIELD, EFFECT_INDEX_0) && CastSpell(MANA_SHIELD, m_bot))
                return RETURN_CONTINUE;

            if (FIREBALL > 0 && m_ai->In_Reach(pTarget,FIREBALL))
                return CastSpell(FIREBALL, pTarget);
            break;
    }

    // No spec due to low level OR no spell found yet
    //if (FROSTBOLT > 0 && m_ai->In_Reach(pTarget,FROSTBOLT) && !pTarget->HasAura(FROSTBOLT, EFFECT_INDEX_0))
       // return CastSpell(FROSTBOLT, pTarget);
    //if (FIREBALL > 0 && m_ai->In_Reach(pTarget,FIREBALL)) // Very low levels
        //return CastSpell(FIREBALL, pTarget);

    return RETURN_NO_ACTION_ERROR; // What? Not even Fireball is available?
} // end DoNextCombatManeuver

CombatManeuverReturns PlayerbotMageAI::DoNextCombatManeuverPVP(Unit* pTarget)
{
    if (FIREBALL && m_ai->In_Reach(pTarget,FIREBALL) && m_ai->CastSpell(FIREBALL))
        return RETURN_CONTINUE;

    return DoNextCombatManeuverPVE(pTarget); // TODO: bad idea perhaps, but better than the alternative
}

void PlayerbotMageAI::DoNonCombatActions()
{
    Player* master = GetMaster();
	
    if (!m_bot || !master)
        return;

    // Buff armor
    if (MOLTEN_ARMOR)
    {
        if (m_ai->SelfBuff(MOLTEN_ARMOR))
            return;
    }
    else if (MAGE_ARMOR)
    {
        if (m_ai->SelfBuff(MAGE_ARMOR))
            return;
    }
    else if (ICE_ARMOR)
    {
        if (m_ai->SelfBuff(ICE_ARMOR))
            return;
    }
    else if (FROST_ARMOR)
        if (m_ai->SelfBuff(FROST_ARMOR))
            return;

    // buff group
	if (m_bot->GetGroup() && m_ai->HasSpellReagents(ARCANE_BRILLIANCE) && Buff(&PlayerbotMageAI::BuffHelper, ARCANE_BRILLIANCE) & RETURN_CONTINUE)
        return;
	else if (Buff(&PlayerbotMageAI::BuffHelper, ARCANE_INTELLECT, JOB_ALL) & RETURN_CONTINUE)
        return;

    

    // TODO: The beauty of a mage is not only its ability to supply itself with water, but to share its water
    // So, conjure at *least* 1.25 stacks, ready to trade a stack and still have some left for self
    if (m_ai->FindDrink() == NULL && CONJURE_WATER && m_ai->CastSpell(CONJURE_WATER, *m_bot))
    {
        m_ai->TellMaster("I'm conjuring some water.");
        m_ai->SetIgnoreUpdateTime(3);
        return;
    }
    if (m_ai->FindFood() == NULL && CONJURE_FOOD && m_ai->CastSpell(CONJURE_FOOD, *m_bot))
    {
        m_ai->TellMaster("I'm conjuring some food.");
        m_ai->SetIgnoreUpdateTime(3);
        return;
    }

	if (CONJURE_MANA_GEM)
	{
		Item* const ManaStone = m_ai->FindConsumable(ManaAgateSTONE_DISPLAYID);
		Item* const ManaStone2 = m_ai->FindConsumable(ManaJadeSTONE_DISPLAYID);
		Item* const ManaStone3 = m_ai->FindConsumable(ManaCitrineSTONE_DISPLAYID);
		if (!ManaStone && m_ai->CastSpell(CONJURE_MANA_GEM))
			return;
		if (!ManaStone2 && m_ai->CastSpell(CONJURE_MANA_JADE))
			return;
		if (!ManaStone3 && m_ai->CastSpell(CONJURE_MANA_CITRINE))
			return;
	}

	if (EatDrinkBandage())
		return;
	// hp/mana check
	//if (m_bot->getStandState() != UNIT_STAND_STATE_STAND)
		//m_bot->SetStandState(UNIT_STAND_STATE_STAND);
} // end DoNonCombatActions

// TODO: this and priest's BuffHelper are identical and thus could probably go in PlayerbotClassAI.cpp somewhere
bool PlayerbotMageAI::BuffHelper(PlayerbotAI* ai, uint32 spellId, Unit *target)
{
    if (!ai)          return false;
    if (spellId == 0) return false;
    if (!target)      return false;

    Pet* pet = target->GetPet();
    if (pet && !pet->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE) && ai->Buff(spellId, pet))
        return true;

    if (ai->Buff(spellId, target))
        return true;

    return false;
}
