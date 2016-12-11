#include "../../../botpch.h"
#include "../../playerbot.h"
#include "EquipAction.h"

#include "../values/ItemCountValue.h"

using namespace ai;

bool EquipAction::Execute(Event event)
{
    string text = event.getParam();
    if (text == "?")
    {
        TellEquipmentSets();
        return true;
    }

    if (UseEquipmentSet(text))
        return true;

    ItemIds ids = chat->parseItems(text);

    for (ItemIds::iterator i =ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        EquipItem(&visitor);
    }

    return true;
}

bool EquipAction::UseEquipmentSet(string& name)
{
    EquipmentSets &sets = bot->GetEquipmentSets();
    for (EquipmentSets::iterator i = sets.begin(); i != sets.end(); i++)
    {
        if (i->second.state == EQUIPMENT_SET_DELETED || i->second.Name != name)
            continue;

        UseEquipmentSet(i->second);

        ostringstream out; out << name << " set equipped";
        ai->TellMaster(out);
        return true;
    }
    return false;
}

bool EquipAction::UseEquipmentSet(EquipmentSet& set)
{
    WorldPacket* p = new WorldPacket(CMSG_EQUIPMENT_SET_USE);
    uint8 srcbag = 0;
    for(uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        ObjectGuid guid;
        uint32 itemId = set.Items[slot];
        if (set.IgnoreMask & (1 << slot))
            p->appendPackGUID((uint64(1)));
        else
            p->appendPackGUID(itemId);
        *p << srcbag << slot;
    }
    bot->GetSession()->QueuePacket(p);
    return true;
}

void EquipAction::TellEquipmentSets()
{
    ai->TellMaster("=== Equipment sets ===");
    EquipmentSets &sets = bot->GetEquipmentSets();
    for (EquipmentSets::iterator i = sets.begin(); i != sets.end(); i++)
    {
        if (i->second.state != EQUIPMENT_SET_DELETED)
            ai->TellMaster(i->second.Name);
    }
}

void EquipAction::EquipItem(FindItemVisitor* visitor)
{
    IterateItems(visitor);
    list<Item*> items = visitor->GetResult();
	if (!items.empty()) EquipItem(**items.begin());
}


void EquipAction::EquipItem(Item& item)
{
    uint8 bagIndex = item.GetBagSlot();
    uint8 slot = item.GetSlot();
    uint32 itemId = item.GetProto()->ItemId;

    if (item.GetProto()->InventoryType == INVTYPE_AMMO)
    {
        bot->SetAmmo(itemId);
    }
    else
    {
		std::unique_ptr<WorldPacket> packet(new WorldPacket(CMSG_AUTOEQUIP_ITEM, 2));
            *packet << bagIndex << slot;
        bot->GetSession()->QueuePacket(std::move(packet));
    }

    ostringstream out; out << "equipping " << chat->formatItem(item.GetProto());
    ai->TellMaster(out);
}
