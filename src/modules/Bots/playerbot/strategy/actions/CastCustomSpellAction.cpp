#include "../../../botpch.h"
#include "../../playerbot.h"
#include "CastCustomSpellAction.h"

using namespace ai;

bool CastCustomSpellAction::Execute(Event event)
{
    Unit* target = NULL;

    Player* master = GetMaster();
    if (master)
        target = master->GetSelectedUnit();

    if (!target)
        target = bot;

    string text = event.getParam();

    uint32 spell = AI_VALUE2(uint32, "spell id", text);

    ostringstream msg;
    if (!ai->CanCastSpell(spell, target))
    {
        msg << "Cannot cast " << text << " on " << target->GetName();
        ai->TellMaster(msg.str());
        return false;
    }

    bool result = false;
    if (spell)
        result = ai->CastSpell(spell, target);
    else
        ai->CastSpell(text, target);

    if (result)
    {
        msg << "Casting " << text << " on " << target->GetName();
        ai->TellMasterNoFacing(msg.str());
    }
    else
    {
        msg << "Cast " << text << " on " << target->GetName() << " is failed";
        ai->TellMaster(msg.str());
    }

    return result;
}
