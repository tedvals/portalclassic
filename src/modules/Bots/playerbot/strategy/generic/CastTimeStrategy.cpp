#include "../../../pchdef.h"
#include "../../playerbot.h"
#include "CastTimeStrategy.h"
#include "../actions/GenericSpellActions.h"

using namespace ai;

float CastTimeMultiplier::GetValue(Action* action)
{
    if (action == NULL) return 1.0f;

    bool normal = AI_VALUE2(bool, "target normal", "current target");
    bool elite = AI_VALUE2(bool, "target elite", "current target");
    bool boss = AI_VALUE2(bool, "target boss", "current target");
    bool player = AI_VALUE2(bool, "target boss", "current target");

    uint8 targetHealth = AI_VALUE2(uint8, "health", "current target");
    string name = action->getName();

    if (action->GetTarget() != AI_VALUE(Unit*, "current target"))
        return 1.0f;

    if (normal && targetHealth < sPlayerbotAIConfig.lowHealth && dynamic_cast<CastSpellAction*>(action))
    {
        uint32 spellId = AI_VALUE2(uint32, "spell id", name);
        const SpellInfo* const pSpellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!pSpellInfo)
            return 1.0f;

        Spell *spell = new Spell(bot, pSpellInfo, TRIGGERED_NONE);
        int32 castTime = spell->GetCastTime();
        delete spell;

        if (spellId && castTime >= 3000)
            return 0.0f;
        else if (spellId && castTime >= 1500)
            return 0.5f;
    }
    else if ((elite || player)&& targetHealth < sPlayerbotAIConfig.almostDead && dynamic_cast<CastSpellAction*>(action))
    {
        uint32 spellId = AI_VALUE2(uint32, "spell id", name);
        const SpellInfo* const pSpellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!pSpellInfo)
            return 1.0f;

        Spell *spell = new Spell(bot, pSpellInfo, TRIGGERED_NONE);
        int32 castTime = spell->GetCastTime();
        delete spell;

        if (spellId && castTime >= 3000)
            return 0.0f;
        else if (spellId && castTime >= 1500)
            return 0.5f;
    }

    return 1.0f;
}


void CastTimeStrategy::InitMultipliers(std::list<Multiplier*> &multipliers)
{
    multipliers.push_back(new CastTimeMultiplier(ai));
}
