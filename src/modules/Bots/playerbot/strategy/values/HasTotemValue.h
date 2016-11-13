#pragma once
#include "../Value.h"
#include "TargetValue.h"
#include "../../LootObjectStack.h"

namespace ai
{
    class HasTotemValue : public BoolCalculatedValue, public Qualified
	{
	public:
        HasTotemValue(PlayerbotAI* ai) : BoolCalculatedValue(ai) {}

    public:
        bool Calculate()
        {
            list<ObjectGuid> units = *context->GetValue<list<ObjectGuid> >("nearest npcs");
            for (list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
            {
                Unit* unit = ai->GetUnit(*i);
                if (!unit)
                    continue;

                Creature* creature = dynamic_cast<Creature*>(unit);
                if (!creature || !creature->IsTotem())
                    continue;

                if (strstri(creature->GetName().c_str(), qualifier.c_str()) && bot->GetDistance(creature) <= sPlayerbotAIConfig.spellDistance)
                    return true;
            }

            return false;
        }
    };

    class HasOwnTotemValue : public BoolCalculatedValue, public Qualified
	{
	public:
        HasOwnTotemValue(PlayerbotAI* ai) : BoolCalculatedValue(ai) {}

    public:
        bool Calculate()
        {
            list<ObjectGuid> units = *context->GetValue<list<ObjectGuid> >("nearest npcs");
            for (list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
            {
                Unit* unit = ai->GetUnit(*i);
                if (!unit)
                    continue;

                Creature* creature = dynamic_cast<Creature*>(unit);
                if (!creature || !creature->IsTotem())
                    continue;

                if (creature->GetOwner() != ai->GetBot())
                    return false;

                if (strstri(creature->GetName().c_str(), qualifier.c_str()) && bot->GetDistance(creature) <= sPlayerbotAIConfig.spellDistance)
                    return true;
            }

            return false;
        }
    };

    class HasAnyOwnTotemValue : public BoolCalculatedValue, public Qualified
	{
	public:
        HasAnyOwnTotemValue(PlayerbotAI* ai) : BoolCalculatedValue(ai) {}

    public:
        bool Calculate()
        {
            list<ObjectGuid> units = *context->GetValue<list<ObjectGuid> >("nearest npcs");
            for (list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
            {
                Unit* unit = ai->GetUnit(*i);
                if (!unit)
                    continue;

                Creature* creature = dynamic_cast<Creature*>(unit);
                if (!creature || !creature->IsTotem())
                    continue;

                if (creature->GetOwner() == ai->GetBot())
                    return true;
            }

            return false;
        }
    };
}
