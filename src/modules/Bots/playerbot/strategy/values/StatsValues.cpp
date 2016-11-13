#include "../../../pchdef.h"
#include "../../playerbot.h"
#include "StatsValues.h"

using namespace ai;

bool IsTargetInLosValue::Calculate()
{
    Unit* target = GetTarget();

    if (!target)
        return false;
    else
    {

      float targetX = target->GetPositionX();
      float targetY = target->GetPositionY();
      float targetZ = target->GetPositionZ();

      return (bot->IsWithinLOS(targetX, targetY, targetZ));
    }
}

bool IsTargetPlayerValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    else
        return target->IsPlayer();
}

bool IsTargetNormalValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    else
        return (target->GetCreatureRank() == CREATURE_ELITE_NORMAL);
}

bool IsTargetBossValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    else
        return (target->GetCreatureRank() == CREATURE_ELITE_WORLDBOSS);
}

bool IsTargetEliteValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    else
        return (target->GetCreatureRank() == CREATURE_ELITE_WORLDBOSS) || (target->GetCreatureRank() == CREATURE_ELITE_ELITE) || ((target->GetCreatureRank() == CREATURE_ELITE_RAREELITE));
}


uint8 HealthValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return 100;
    return (static_cast<float> (target->GetHealth()) / target->GetMaxHealth()) * 100;
}

bool IsDeadValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->getDeathState() != ALIVE;
}

bool IsCcValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->UnderCc();
}

bool IsBleedingValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->isBleeding();
}

bool IsPolymorphedValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->IsPolymorphed();
}

bool TakesPeriodicDamageValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->TakesPeriodicDamage();
}

bool IsFrozenValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->isFrozen();
}

bool IsSilencedValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->isFrozen();
}

bool IsFearedValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->isFeared();
}

bool IsRootedValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->isInRoots();
}

bool IsCharmedValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->IsCharmed();
}

bool IsPossessedValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->isPossessed();
}

bool IsDisorientedValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->isDisoriented();
}

bool IsStunnedValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->isStunned();
}

bool IsSnaredValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->isSnared();
}

bool IsFleeingValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->IsFleeing();
}

uint8 RageValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return 0;
    return (static_cast<float> (target->GetPower(POWER_RAGE)));
}

uint8 EnergyValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return 0;
    return (static_cast<float> (target->GetPower(POWER_ENERGY)));
}

uint8 RuneValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return 0;
    return (static_cast<float> (target->GetPower(POWER_RUNE)));
}
uint8 RunicPowerValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return 0;
    return (static_cast<float> (target->GetPower(POWER_RUNIC_POWER)));
}

uint8 ManaValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return 100;
    return (static_cast<float> (target->GetPower(POWER_MANA)) / target->GetMaxPower(POWER_MANA)) * 100;
}

bool HasManaValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->GetPower(POWER_MANA);
}


uint8 ComboPointsValue::Calculate()
{
    Unit *target = GetTarget();
    if (!target || target->GetGUID() != bot->GetComboTarget())
        return 0;

    return bot->GetComboPoints();
}

bool IsMountedValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    return target->IsMounted();
}


bool IsInCombatValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    return target->IsInCombat();
}

uint8 BagSpaceValue::Calculate()
{
    uint32 totalused = 0, total = 16;
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; slot++)
    {
        if (bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            totalused++;
    }

    uint32 totalfree = 16 - totalused;
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        const Bag* const pBag = (Bag*) bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (pBag)
        {
            ItemTemplate const* pBagProto = pBag->GetTemplate();
            if (pBagProto->Class == ITEM_CLASS_CONTAINER && pBagProto->SubClass == ITEM_SUBCLASS_CONTAINER)
            {
                total += pBag->GetBagSize();
                totalfree += pBag->GetFreeSlots();
            }
        }

    }

    return (static_cast<float> (totalused) / total) * 100;
}

uint8 SpeedValue::Calculate()
{
    Unit* target = GetTarget();
    if (!target)
        return 100;

    return (uint8) (100.0f * target->GetSpeedRate(MOVE_RUN));
}
