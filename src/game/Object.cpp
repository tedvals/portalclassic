/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
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

#include "Object.h"
#include "SharedDefines.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Log.h"
#include "World.h"
#include "Creature.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "UpdateData.h"
#include "UpdateMask.h"
#include "Util.h"
#include "MapManager.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ObjectPosSelector.h"
#include "TemporarySummon.h"
#include "movement/packet_builder.h"
#include "CreatureLinkingMgr.h"
#include "Chat.h"
#include "LootMgr.h"
#include "LuaEngine.h"
#include "ElunaEventMgr.h"

 ////////////////////////////////////////////////////////////
 // Methods of class MovementInfo

void MovementInfo::Read(ByteBuffer& data)
{
	time = WorldTimer::getMSTime();
	data >> moveFlags;
	data >> time;
	data >> pos.x;
	data >> pos.y;
	data >> pos.z;
	data >> pos.o;

	if (HasMovementFlag(MOVEFLAG_ONTRANSPORT))
	{
		data >> t_guid;
		data >> t_pos.x;
		data >> t_pos.y;
		data >> t_pos.z;
		data >> t_pos.o;
	}
	if (HasMovementFlag(MOVEFLAG_SWIMMING))
	{
		data >> s_pitch;
	}

	data >> fallTime;

	if (HasMovementFlag(MOVEFLAG_FALLING))
	{
		data >> jump.velocity;
		data >> jump.sinAngle;
		data >> jump.cosAngle;
		data >> jump.xyspeed;
		if (!jump.startClientTime)
		{
			jump.startClientTime = ctime;
			jump.start = pos;
		}
	}
	else
		jump.startClientTime = 0;

	if (HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
	{
		data >> splineElevation;                                     // unknown
	}
}

void MovementInfo::CorrectData(Unit* mover)
{
	// Nostalrius: remove incompatible flags, causing client freezes for example
#define REMOVE_VIOLATING_FLAGS(check, maskToRemove) \
        if (check) \
            RemoveMovementFlag(MovementFlags(maskToRemove));


	/*! This must be a packet spoofing attempt. MOVEFLAG_ROOT sent from the client is not valid
	in conjunction with any of the moving movement flags such as MOVEMENTFLAG_FORWARD.
	It will freeze clients that receive this player's movement info.
	*/
	REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEFLAG_ROOT) && HasMovementFlag(movementFlagsMask),
		MOVEFLAG_ROOT);

	//! Cannot hover without SPELL_AURA_HOVER
	//REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEFLAG_HOVER) && !GetPlayer()->HasAuraType(SPELL_AURA_HOVER),
	//    MOVEFLAG_HOVER);

	//! Cannot move left and right at the same time
	REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEFLAG_TURN_LEFT) && HasMovementFlag(MOVEFLAG_TURN_RIGHT),
		MOVEFLAG_TURN_LEFT | MOVEFLAG_TURN_RIGHT);

	//! Cannot strafe left and right at the same time
	REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEFLAG_STRAFE_LEFT) && HasMovementFlag(MOVEFLAG_STRAFE_RIGHT),
		MOVEFLAG_STRAFE_LEFT | MOVEFLAG_STRAFE_RIGHT);

	//! Cannot pitch up and down at the same time
	REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEFLAG_PITCH_UP) && HasMovementFlag(MOVEFLAG_PITCH_DOWN),
		MOVEFLAG_PITCH_UP | MOVEFLAG_PITCH_DOWN);

	//! Cannot move forwards and backwards at the same time
	REMOVE_VIOLATING_FLAGS(HasMovementFlag(MOVEFLAG_FORWARD) && HasMovementFlag(MOVEFLAG_BACKWARD),
		MOVEFLAG_FORWARD | MOVEFLAG_BACKWARD);

#undef REMOVE_VIOLATING_FLAGS
}

void MovementInfo::Write(ByteBuffer& data) const
{
	data << moveFlags;
	data << time;
	data << pos.x;
	data << pos.y;
	data << pos.z;
	data << pos.o;

	if (HasMovementFlag(MOVEFLAG_ONTRANSPORT))
	{
		data << t_guid;
		data << t_pos.x;
		data << t_pos.y;
		data << t_pos.z;
		data << t_pos.o;
	}
	if (HasMovementFlag(MOVEFLAG_SWIMMING))
	{
		data << s_pitch;
	}

	data << fallTime;

	if (HasMovementFlag(MOVEFLAG_FALLING))
	{
		data << jump.velocity;
		data << jump.sinAngle;
		data << jump.cosAngle;
		data << jump.xyspeed;
	}

	if (HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
	{
		data << splineElevation;                                     // unknown
	}
}

Object::Object(): m_updateFlag(0)
{
    m_objectTypeId      = TYPEID_OBJECT;
    m_objectType        = TYPEMASK_OBJECT;

    m_uint32Values      = nullptr;
    m_valuesCount       = 0;

    m_inWorld           = false;
    m_objectUpdated     = false;
    loot              = nullptr;
}

Object::~Object()
{
    if (IsInWorld())
    {
        ///- Do NOT call RemoveFromWorld here, if the object is a player it will crash
        sLog.outError("Object::~Object (GUID: %u TypeId: %u) deleted but still in world!!", GetGUIDLow(), GetTypeId());
        MANGOS_ASSERT(false);
    }

    if (m_objectUpdated)
    {
        sLog.outError("Object::~Object (GUID: %u TypeId: %u) deleted but still have updated status!!", GetGUIDLow(), GetTypeId());
        MANGOS_ASSERT(false);
    }

    delete[] m_uint32Values;

    delete loot;
}

void Object::_InitValues()
{
    m_uint32Values = new uint32[ m_valuesCount ];
    memset(m_uint32Values, 0, m_valuesCount * sizeof(uint32));

    m_changedValues.resize(m_valuesCount, false);

    m_objectUpdated = false;
}

void Object::_Create(uint32 guidlow, uint32 entry, HighGuid guidhigh)
{
    if (!m_uint32Values)
        _InitValues();

    ObjectGuid guid = ObjectGuid(guidhigh, entry, guidlow);
    SetGuidValue(OBJECT_FIELD_GUID, guid);
    SetUInt32Value(OBJECT_FIELD_TYPE, m_objectType);
    m_PackGUID.Set(guid);
}

void Object::SetObjectScale(float newScale)
{
    SetFloatValue(OBJECT_FIELD_SCALE_X, newScale);
}

void Object::SendForcedObjectUpdate()
{
    if (!m_inWorld || !m_objectUpdated)
        return;

    UpdateDataMapType update_players;

    BuildUpdateData(update_players);
    RemoveFromClientUpdateList();

    WorldPacket packet;                                     // here we allocate a std::vector with a size of 0x10000
    for (UpdateDataMapType::iterator iter = update_players.begin(); iter != update_players.end(); ++iter)
    {
        iter->second.BuildPacket(packet);
        iter->first->GetSession()->SendPacket(packet);
        packet.clear();                                     // clean the string
    }
}

void Object::BuildMovementUpdateBlock(UpdateData* data, uint8 flags) const
{
    ByteBuffer buf(500);

    buf << uint8(UPDATETYPE_MOVEMENT);
    buf << GetObjectGuid();

    BuildMovementUpdate(&buf, flags);

    data->AddUpdateBlock(buf);
}

void Object::BuildCreateUpdateBlockForPlayer(UpdateData* data, Player* target) const
{
    if (!target)
        return;

    uint8  updatetype   = UPDATETYPE_CREATE_OBJECT;
    uint8 updateFlags  = m_updateFlag;

    /** lower flag1 **/
    if (target == this)                                     // building packet for yourself
        updateFlags |= UPDATEFLAG_SELF;

    if (m_itsNewObject)
    {
        switch (GetObjectGuid().GetHigh())
        {
            case HighGuid::HIGHGUID_DYNAMICOBJECT:
            case HighGuid::HIGHGUID_CORPSE:
            case HighGuid::HIGHGUID_PLAYER:
            case HighGuid::HIGHGUID_UNIT:
            case HighGuid::HIGHGUID_GAMEOBJECT:
                updatetype = UPDATETYPE_CREATE_OBJECT2;
                break;

            default:
                break;
        }
    }

    // DEBUG_LOG("BuildCreateUpdate: update-type: %u, object-type: %u got updateFlags: %X", updatetype, m_objectTypeId, updateFlags);

    ByteBuffer buf(500);
    buf << uint8(updatetype);
    buf << GetPackGUID();
    buf << uint8(m_objectTypeId);

    BuildMovementUpdate(&buf, updateFlags);
    UpdateMask updateMask;
    updateMask.SetCount(m_valuesCount);
    _SetCreateBits(&updateMask, target);
    BuildValuesUpdate(updatetype, &buf, &updateMask, target);
    data->AddUpdateBlock(buf);
}

void Object::SendCreateUpdateToPlayer(Player* player) const
{
    // send create update to player
    UpdateData upd;
    WorldPacket packet;

    BuildCreateUpdateBlockForPlayer(&upd, player);
    upd.BuildPacket(packet);
    player->GetSession()->SendPacket(packet);
}

void WorldObject::DirectSendPublicValueUpdate(uint32 index)
{
	// Do we need an update ?
	if (m_uint32Values_mirror[index] == m_uint32Values[index])
		return;
	m_uint32Values_mirror[index] = m_uint32Values[index];
	UpdateData data;
	ByteBuffer buf(50);
	buf << uint8(UPDATETYPE_VALUES);
	buf << GetPackGUID();

	UpdateMask updateMask;
	updateMask.SetCount(m_valuesCount);
	updateMask.SetBit(index);

	buf << (uint8)updateMask.GetBlockCount();
	buf.append(updateMask.GetMask(), updateMask.GetLength());
	buf << uint32(m_uint32Values[index]);

	data.AddUpdateBlock(buf);
	WorldPacket packet;
	data.BuildPacket(packet);
	SendObjectMessageToSet(&packet, true);
}

void Object::BuildValuesUpdateBlockForPlayer(UpdateData* data, Player* target) const
{
    ByteBuffer buf(500);

    buf << uint8(UPDATETYPE_VALUES);
    buf << GetPackGUID();

    UpdateMask updateMask;
    updateMask.SetCount(m_valuesCount);

    _SetUpdateBits(&updateMask, target);
    BuildValuesUpdate(UPDATETYPE_VALUES, &buf, &updateMask, target);

    data->AddUpdateBlock(buf);
}

void Object::BuildOutOfRangeUpdateBlock(UpdateData* data) const
{
    data->AddOutOfRangeGUID(GetObjectGuid());
}

void Object::DestroyForPlayer(Player* target) const
{
    MANGOS_ASSERT(target);

    WorldPacket data(SMSG_DESTROY_OBJECT, 8);
    data << GetObjectGuid();
    target->GetSession()->SendPacket(data);
}

void Object::BuildMovementUpdate(ByteBuffer* data, uint8 updateFlags) const
{
    Unit const* unit = nullptr;
    uint32 highGuid = 0;
    MovementFlags moveflags = MOVEFLAG_NONE;

    switch (m_objectTypeId)
    {
        case TYPEID_OBJECT:
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
        case TYPEID_GAMEOBJECT:
        case TYPEID_DYNAMICOBJECT:
        case TYPEID_CORPSE:
            highGuid = uint32(GetObjectGuid().GetHigh());
            break;

        case TYPEID_PLAYER:
            // TODO: this code must not be here
            if (static_cast<Player const*>(this)->GetTransport())
                ((Unit*)this)->m_movementInfo.AddMovementFlag(MOVEFLAG_ONTRANSPORT);
            else
                ((Unit*)this)->m_movementInfo.RemoveMovementFlag(MOVEFLAG_ONTRANSPORT);

        case TYPEID_UNIT:
            unit = static_cast<Unit const*>(this);
            moveflags = unit->m_movementInfo.GetMovementFlags();
            break;

        default:
            break;
    }

    *data << uint8(updateFlags);

    if (updateFlags & UPDATEFLAG_LIVING)
    {
		Unit const* unit = ToUnit();
		ASSERT(unit);
		WorldObject const* wobject = (WorldObject*)this;
		MovementInfo m = wobject->m_movementInfo;
		if (!m.ctime)
		{
			m.time = WorldTimer::getMSTime() + 1000;
			m.ChangePosition(wobject->GetPositionX(), wobject->GetPositionY(), wobject->GetPositionZ(), wobject->GetOrientation());
		}
		if (unit->ToCreature())
			m.moveFlags = m.moveFlags & ~MOVEFLAG_ROOT;

        if (unit->IsStopped() && unit->m_movementInfo.HasMovementFlag(MOVEFLAG_SPLINE_ENABLED))
        {
            sLog.outError("%s is not moving but have spline movement enabled!", GetGuidStr().c_str());
            ((Unit*)this)->m_movementInfo.RemoveMovementFlag(MovementFlags(MOVEFLAG_SPLINE_ENABLED | MOVEFLAG_FORWARD));
        }

        *data << unit->m_movementInfo;
        // Unit speeds
        *data << float(unit->GetSpeed(MOVE_WALK));
        *data << float(unit->GetSpeed(MOVE_RUN));
        *data << float(unit->GetSpeed(MOVE_RUN_BACK));
        *data << float(unit->GetSpeed(MOVE_SWIM));
        *data << float(unit->GetSpeed(MOVE_SWIM_BACK));
        *data << float(unit->GetSpeed(MOVE_TURN_RATE));

        if (unit->m_movementInfo.HasMovementFlag(MOVEFLAG_SPLINE_ENABLED))
            Movement::PacketBuilder::WriteCreate(*unit->movespline, *data);
    }
    else if (updateFlags & UPDATEFLAG_HAS_POSITION)
    {
        *data << ((WorldObject*)this)->GetPositionX();
        *data << ((WorldObject*)this)->GetPositionY();
        *data << ((WorldObject*)this)->GetPositionZ();
        *data << ((WorldObject*)this)->GetOrientation();
    }

    if (updateFlags & UPDATEFLAG_HIGHGUID)
        *data << highGuid;

    if (updateFlags & UPDATEFLAG_ALL)
        *data << (uint32)0x1;

    if (updateFlags & UPDATEFLAG_FULLGUID)
    {
        if (unit && unit->getVictim())
            *data << unit->getVictim()->GetPackGUID();
        else
            data->appendPackGUID(0);
    }

    if (updateFlags & UPDATEFLAG_TRANSPORT)
        *data << uint32(WorldTimer::getMSTime());
}

void Object::BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, UpdateMask* updateMask, Player* target) const
{
    if (!target)
        return;

    bool IsActivateToQuest = false;
    bool IsPerCasterAuraState = false;
    bool sendPercent = false;

    if (isType(TYPEMASK_UNIT))
    {
        if (!static_cast<Unit const*>(this)->IsTargetUnderControl(*target))
        {
            if (m_objectTypeId == TYPEID_UNIT)
            {
                if (!static_cast<Creature const*>(this)->IsPet() && !target->IsFriendlyTo(static_cast<Unit const*>(this)))
                    sendPercent = true;
            }
            else
            {
                if (target != this && m_objectTypeId == TYPEID_PLAYER)
                {
                    if (static_cast<Player const*>(this)->GetTeam() != static_cast<Player const*>(target)->GetTeam() ||
                        !target->IsFriendlyTo(static_cast<Unit const*>(this)))
                    {
                        // not same faction or not friendly
                        sendPercent = true;
                    }
                }
            }
        }
    }

    if (updatetype == UPDATETYPE_CREATE_OBJECT || updatetype == UPDATETYPE_CREATE_OBJECT2)
    {
        if (isType(TYPEMASK_GAMEOBJECT) && !((GameObject*)this)->IsTransport())
        {
            if (((GameObject*)this)->ActivateToQuest(target) || target->isGameMaster())
                IsActivateToQuest = true;

            updateMask->SetBit(GAMEOBJECT_DYN_FLAGS);
        }
    }
    else                                                    // case UPDATETYPE_VALUES
    {
        if (isType(TYPEMASK_GAMEOBJECT) && !((GameObject*)this)->IsTransport())
        {
            if (((GameObject*)this)->ActivateToQuest(target) || target->isGameMaster())
                IsActivateToQuest = true;

            updateMask->SetBit(GAMEOBJECT_DYN_FLAGS);
            updateMask->SetBit(GAMEOBJECT_ANIMPROGRESS);
        }
    }

    MANGOS_ASSERT(updateMask && updateMask->GetCount() == m_valuesCount);

    *data << (uint8)updateMask->GetBlockCount();
    data->append(updateMask->GetMask(), updateMask->GetLength());

    // 2 specialized loops for speed optimization in non-unit case
    if (isType(TYPEMASK_UNIT))                              // unit (creature/player) case
    {
        for (uint16 index = 0; index < m_valuesCount; ++index)
        {
            if (updateMask->GetBit(index))
            {
                if (index == UNIT_NPC_FLAGS)
                {
                    uint32 appendValue = m_uint32Values[index];

                    if (GetTypeId() == TYPEID_UNIT)
                    {
                        if (appendValue & UNIT_NPC_FLAG_TRAINER)
                        {
                            if (!((Creature*)this)->IsTrainerOf(target, false))
                                appendValue &= ~UNIT_NPC_FLAG_TRAINER;
                        }

                        if (appendValue & UNIT_NPC_FLAG_STABLEMASTER)
                        {
                            if (target->getClass() != CLASS_HUNTER)
                                appendValue &= ~UNIT_NPC_FLAG_STABLEMASTER;
                        }
                        
                        if (appendValue & UNIT_NPC_FLAG_FLIGHTMASTER)
                        {
                            QuestRelationsMapBounds bounds = sObjectMgr.GetCreatureQuestRelationsMapBounds(((Creature*)this)->GetEntry());
                            for (QuestRelationsMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
                            {
                                Quest const* pQuest = sObjectMgr.GetQuestTemplate(itr->second);
                                if (target->CanSeeStartQuest(pQuest))
                                {
                                    appendValue &= ~UNIT_NPC_FLAG_FLIGHTMASTER;
                                    break;
                                }
                            }

                            bounds = sObjectMgr.GetCreatureQuestInvolvedRelationsMapBounds(((Creature*)this)->GetEntry());
                            for (QuestRelationsMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
                            {
                                Quest const* pQuest = sObjectMgr.GetQuestTemplate(itr->second);
                                if (target->CanRewardQuest(pQuest, false))
                                {
                                    appendValue &= ~UNIT_NPC_FLAG_FLIGHTMASTER;
                                    break;
                                }
                            }
                        }
                    }

                    *data << uint32(appendValue);
                }
                else if (sendPercent && index == UNIT_FIELD_HEALTH)
                {
                    // send health percentage instead of real value to enemy
                    if (m_uint32Values[UNIT_FIELD_HEALTH] == 0)
                        *data << uint32(0);
                    else
                        *data << uint32(ceil(m_uint32Values[UNIT_FIELD_HEALTH] * 100 / float(m_uint32Values[UNIT_FIELD_MAXHEALTH]))); // never less than 1 as health is not zero
                }
                else if (sendPercent && index == UNIT_FIELD_MAXHEALTH)
                {
                    *data << uint32(100);
                }
                // FIXME: Some values at server stored in float format but must be sent to client in uint32 format
                else if (index >= UNIT_FIELD_BASEATTACKTIME && index <= UNIT_FIELD_RANGEDATTACKTIME)
                {
                    // convert from float to uint32 and send
                    *data << uint32(m_floatValues[index] < 0 ? 0 : m_floatValues[index]);
                }

                // there are some float values which may be negative or can't get negative due to other checks
                else if ((index >= PLAYER_FIELD_NEGSTAT0    && index <= PLAYER_FIELD_NEGSTAT4) ||
                         (index >= PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE  && index <= (PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE + 6)) ||
                         (index >= PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE  && index <= (PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE + 6)) ||
                         (index >= PLAYER_FIELD_POSSTAT0    && index <= PLAYER_FIELD_POSSTAT4))
                {
                    *data << uint32(m_floatValues[index]);
                }

                // Gamemasters should be always able to select units - remove not selectable flag
                else if (index == UNIT_FIELD_FLAGS && target->isGameMaster())
                {
                    *data << (m_uint32Values[index] & ~UNIT_FLAG_NOT_SELECTABLE);
                }
                // Hide lootable animation for unallowed players
                // Handle tapped flag
                else if (index == UNIT_DYNAMIC_FLAGS && GetTypeId() == TYPEID_UNIT)
                {
                    Creature* creature = (Creature*)this;
                    uint32 dynflagsValue = m_uint32Values[index];
                    bool setTapFlags = false;

                    if (creature->isAlive())
                    {
                        // creature is alive so, not lootable
                        dynflagsValue = dynflagsValue & ~UNIT_DYNFLAG_LOOTABLE;

                        if (creature->isInCombat())
                        {
                            // as creature is in combat we have to manage tap flags
                            setTapFlags = true;
                        }
                        else
                        {
                            // creature is not in combat so its not tapped
                            dynflagsValue = dynflagsValue & ~UNIT_DYNFLAG_TAPPED;
                            //sLog.outString(">> %s is not in combat so not tapped by %s", this->GetObjectGuid().GetString().c_str(), target->GetObjectGuid().GetString().c_str());
                        }
                    }
                    else
                    {
                        // check loot flag
                        if (creature->loot && creature->loot->CanLoot(target))
                        {
                            // creature is dead and this player can loot it
                            dynflagsValue = dynflagsValue | UNIT_DYNFLAG_LOOTABLE;
                            //sLog.outString(">> %s is lootable for %s", this->GetObjectGuid().GetString().c_str(), target->GetObjectGuid().GetString().c_str());
                        }
                        else
                        {
                            // creature is dead but this player cannot loot it
                            dynflagsValue = dynflagsValue & ~UNIT_DYNFLAG_LOOTABLE;
                            //sLog.outString(">> %s is not lootable for %s", this->GetObjectGuid().GetString().c_str(), target->GetObjectGuid().GetString().c_str());
                        }

                        // as creature is died we have to manage tap flags
                        setTapFlags = true;
                    }

                    // check tap flags
                    if (setTapFlags)
                    {
                        if (creature->IsTappedBy(target))
                        {
                            // creature is in combat or died and tapped by this player
                            dynflagsValue = dynflagsValue & ~UNIT_DYNFLAG_TAPPED;
                            //sLog.outString(">> %s is tapped by %s", this->GetObjectGuid().GetString().c_str(), target->GetObjectGuid().GetString().c_str());
                        }
                        else
                        {
                            // creature is in combat or died but not tapped by this player
                            dynflagsValue = dynflagsValue | UNIT_DYNFLAG_TAPPED;
                            //sLog.outString(">> %s is not tapped by %s", this->GetObjectGuid().GetString().c_str(), target->GetObjectGuid().GetString().c_str());
                        }
                    }

                    *data << dynflagsValue;
                }
                else                                        // Unhandled index, just send
                {
                    // send in current format (float as float, uint32 as uint32)
                    *data << m_uint32Values[index];
                }
            }
        }
    }
    else if (isType(TYPEMASK_GAMEOBJECT))                   // gameobject case
    {
        for (uint16 index = 0; index < m_valuesCount; ++index)
        {
            if (updateMask->GetBit(index))
            {
                // send in current format (float as float, uint32 as uint32)
                if (index == GAMEOBJECT_DYN_FLAGS)
                {
                    if (IsActivateToQuest)
                    {
                        GameObject const* gameObject = static_cast<GameObject const*>(this);
                        switch (((GameObject*)this)->GetGoType())
                        {
                            case GAMEOBJECT_TYPE_QUESTGIVER:
                            case GAMEOBJECT_TYPE_CHEST:
                                if (gameObject->getLootState() == GO_READY || gameObject->getLootState() == GO_ACTIVATED)
                                    *data << uint16(GO_DYNFLAG_LO_ACTIVATE | GO_DYNFLAG_LO_SPARKLE);
                                else
                                    *data << uint16(0);
                                *data << uint16(0);
                                break;
                            case GAMEOBJECT_TYPE_GENERIC:
                            case GAMEOBJECT_TYPE_SPELL_FOCUS:
                            case GAMEOBJECT_TYPE_GOOBER:
                                *data << uint16(GO_DYNFLAG_LO_ACTIVATE);
                                *data << uint16(0);
                                break;
                            default:
                                *data << uint32(0);         // unknown, not happen.
                                break;
                        }
                    }
                    else
                        *data << uint32(0);                 // disable quest object
                }
                else
                    *data << m_uint32Values[index];         // other cases
            }
        }
    }
    else                                                    // other objects case (no special index checks)
    {
        for (uint16 index = 0; index < m_valuesCount; ++index)
        {
            if (updateMask->GetBit(index))
            {
                // send in current format (float as float, uint32 as uint32)
                *data << m_uint32Values[index];
            }
        }
    }
}

void Object::ClearUpdateMask(bool remove)
{
    if (m_uint32Values)
    {
        for (uint16 index = 0; index < m_valuesCount; ++index)
            m_changedValues[index] = false;
    }

    if (m_objectUpdated)
    {
        if (remove)
            RemoveFromClientUpdateList();
        m_objectUpdated = false;
    }
}

bool Object::LoadValues(const char* data)
{
    if (!m_uint32Values) _InitValues();

    Tokens tokens = StrSplit(data, " ");

    if (tokens.size() != m_valuesCount)
        return false;

    Tokens::iterator iter;
    int index;
    for (iter = tokens.begin(), index = 0; index < m_valuesCount; ++iter, ++index)
    {
        m_uint32Values[index] = std::stoul((*iter).c_str());
    }

    return true;
}

void Object::_LoadIntoDataField(std::string const& data, uint32 startOffset, uint32 count)
{
	if (data.empty())
		return;

	Tokenizer tokens(data, ' ', count);

	if (tokens.size() != count)
		return;

	for (uint32 index = 0; index < count; ++index)
	{
		m_uint32Values[startOffset + index] = strtoul(tokens[index], nullptr, 10);
		m_uint32Values_mirror[startOffset + index] = m_uint32Values[startOffset + index] + 1;
	}
}

void Object::_SetUpdateBits(UpdateMask* updateMask, Player* /*target*/) const
{
    for (uint16 index = 0; index < m_valuesCount; ++index)
        if (m_changedValues[index])
            updateMask->SetBit(index);
}

void Object::_SetCreateBits(UpdateMask* updateMask, Player* /*target*/) const
{
    for (uint16 index = 0; index < m_valuesCount; ++index)
        if (GetUInt32Value(index) != 0)
            updateMask->SetBit(index);
}

void Object::SetInt32Value(uint16 index, int32 value)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (m_int32Values[index] != value)
    {
        m_int32Values[index] = value;
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetUInt32Value(uint16 index, uint32 value)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (m_uint32Values[index] != value)
    {
        m_uint32Values[index] = value;
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::UpdateUInt32Value(uint16 index, uint32 value)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    m_uint32Values[index] = value;
    m_changedValues[index] = true;
}

void Object::SetUInt64Value(uint16 index, const uint64& value)
{
    MANGOS_ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, true));
    if (*((uint64*) & (m_uint32Values[index])) != value)
    {
        m_uint32Values[index] = *((uint32*)&value);
        m_uint32Values[index + 1] = *(((uint32*)&value) + 1);
        m_changedValues[index] = true;
        m_changedValues[index + 1] = true;
        MarkForClientUpdate();
    }
}

void Object::SetFloatValue(uint16 index, float value)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (m_floatValues[index] != value)
    {
        m_floatValues[index] = value;
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetByteValue(uint16 index, uint8 offset, uint8 value)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (offset > 4)
    {
        sLog.outError("Object::SetByteValue: wrong offset %u", offset);
        return;
    }

    if (uint8(m_uint32Values[index] >> (offset * 8)) != value)
    {
        m_uint32Values[index] &= ~uint32(uint32(0xFF) << (offset * 8));
        m_uint32Values[index] |= uint32(uint32(value) << (offset * 8));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetUInt16Value(uint16 index, uint8 offset, uint16 value)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (offset > 2)
    {
        sLog.outError("Object::SetUInt16Value: wrong offset %u", offset);
        return;
    }

    if (uint16(m_uint32Values[index] >> (offset * 16)) != value)
    {
        m_uint32Values[index] &= ~uint32(uint32(0xFFFF) << (offset * 16));
        m_uint32Values[index] |= uint32(uint32(value) << (offset * 16));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetStatFloatValue(uint16 index, float value)
{
    if (value < 0)
        value = 0.0f;

    SetFloatValue(index, value);
}

void Object::SetStatInt32Value(uint16 index, int32 value)
{
    if (value < 0)
        value = 0;

    SetUInt32Value(index, uint32(value));
}

void Object::ApplyModUInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetUInt32Value(index);
    cur += (apply ? val : -val);
    if (cur < 0)
        cur = 0;
    SetUInt32Value(index, cur);
}

void Object::ApplyModInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetInt32Value(index);
    cur += (apply ? val : -val);
    SetInt32Value(index, cur);
}

void Object::ApplyModSignedFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    SetFloatValue(index, cur);
}

void Object::ApplyModPositiveFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    if (cur < 0)
        cur = 0;
    SetFloatValue(index, cur);
}

void Object::SetFlag(uint16 index, uint32 newFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    uint32 oldval = m_uint32Values[index];
    uint32 newval = oldval | newFlag;

    if (oldval != newval)
    {
        m_uint32Values[index] = newval;
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::RemoveFlag(uint16 index, uint32 oldFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    uint32 oldval = m_uint32Values[index];
    uint32 newval = oldval & ~oldFlag;

    if (oldval != newval)
    {
        m_uint32Values[index] = newval;
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetByteFlag(uint16 index, uint8 offset, uint8 newFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (offset > 4)
    {
        sLog.outError("Object::SetByteFlag: wrong offset %u", offset);
        return;
    }

    if (!(uint8(m_uint32Values[index] >> (offset * 8)) & newFlag))
    {
        m_uint32Values[index] |= uint32(uint32(newFlag) << (offset * 8));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::RemoveByteFlag(uint16 index, uint8 offset, uint8 oldFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (offset > 4)
    {
        sLog.outError("Object::RemoveByteFlag: wrong offset %u", offset);
        return;
    }

    if (uint8(m_uint32Values[index] >> (offset * 8)) & oldFlag)
    {
        m_uint32Values[index] &= ~uint32(uint32(oldFlag) << (offset * 8));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetShortFlag(uint16 index, bool highpart, uint16 newFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (!(uint16(m_uint32Values[index] >> (highpart ? 16 : 0)) & newFlag))
    {
        m_uint32Values[index] |= uint32(uint32(newFlag) << (highpart ? 16 : 0));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::RemoveShortFlag(uint16 index, bool highpart, uint16 oldFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (uint16(m_uint32Values[index] >> (highpart ? 16 : 0)) & oldFlag)
    {
        m_uint32Values[index] &= ~uint32(uint32(oldFlag) << (highpart ? 16 : 0));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

bool Object::PrintIndexError(uint32 index, bool set) const
{
    sLog.outError("Attempt %s nonexistent value field: %u (count: %u) for object typeid: %u type mask: %u", (set ? "set value to" : "get value from"), index, m_valuesCount, GetTypeId(), m_objectType);

    // ASSERT must fail after function call
    return false;
}

bool Object::PrintEntryError(char const* descr) const
{
    sLog.outError("Object Type %u, Entry %u (lowguid %u) with invalid call for %s", GetTypeId(), GetEntry(), GetObjectGuid().GetCounter(), descr);

    // always false for continue assert fail
    return false;
}


void Object::BuildUpdateDataForPlayer(Player* pl, UpdateDataMapType& update_players)
{
    UpdateDataMapType::iterator iter = update_players.find(pl);

    if (iter == update_players.end())
    {
        std::pair<UpdateDataMapType::iterator, bool> p = update_players.insert(UpdateDataMapType::value_type(pl, UpdateData()));
        MANGOS_ASSERT(p.second);
        iter = p.first;
    }

    BuildValuesUpdateBlockForPlayer(&iter->second, iter->first);
}

void Object::AddToClientUpdateList()
{
    sLog.outError("Unexpected call of Object::AddToClientUpdateList for object (TypeId: %u Update fields: %u)", GetTypeId(), m_valuesCount);
    MANGOS_ASSERT(false);
}

void Object::RemoveFromClientUpdateList()
{
    sLog.outError("Unexpected call of Object::RemoveFromClientUpdateList for object (TypeId: %u Update fields: %u)", GetTypeId(), m_valuesCount);
    MANGOS_ASSERT(false);
}

void Object::BuildUpdateData(UpdateDataMapType& /*update_players */)
{
    sLog.outError("Unexpected call of Object::BuildUpdateData for object (TypeId: %u Update fields: %u)", GetTypeId(), m_valuesCount);
    MANGOS_ASSERT(false);
}

void Object::MarkForClientUpdate()
{
	if (!m_inWorld)
		return;
	if (GetTypeId() == TYPEID_ITEM || GetTypeId() == TYPEID_CONTAINER)
	{
		if (m_inWorld)
		{
			if (!m_objectUpdated)
			{
				AddToClientUpdateList();
				m_objectUpdated = true;
			}
		}
	}
	else
		AddDelayedAction(OBJECT_DELAYED_MARK_CLIENT_UPDATE);
}

void Object::ExecuteDelayedActions()
{
	if (_delayedActions & OBJECT_DELAYED_MARK_CLIENT_UPDATE)
	{
		if (m_inWorld)
		{
			if (!m_objectUpdated)
			{
				AddToClientUpdateList();
				m_objectUpdated = true;
			}
		}
		_delayedActions &= ~OBJECT_DELAYED_MARK_CLIENT_UPDATE;
	}
	if (_delayedActions & OBJECT_DELAYED_ADD_TO_REMOVE_LIST)
	{
		if (!IsDeleted() && IsInWorld())
			((WorldObject*)this)->AddObjectToRemoveList();
		_delayedActions &= ~OBJECT_DELAYED_ADD_TO_REMOVE_LIST;
	}
}

void Object::ForceValuesUpdateAtIndex(uint32 index)
{
    m_changedValues[index] = true;
    if (m_inWorld && !m_objectUpdated)
    {
        AddToClientUpdateList();
        m_objectUpdated = true;
    }
}

WorldObject::WorldObject() :
    elunaEvents(NULL),
    m_currMap(nullptr),
    m_mapId(0), m_InstanceId(0),
    m_isActiveObject(false)
{    
	// Phasing
	worldMask = WORLD_DEFAULT_OBJECT;
	m_zoneScript = nullptr;
	m_transport = nullptr;
	m_movementInfo.time = WorldTimer::getMSTime();
}

WorldObject::~WorldObject()
{
    delete elunaEvents;
    elunaEvents = NULL;
}

void WorldObject::CleanupsBeforeDelete()
{
    RemoveFromWorld();

	if (Transport* transport = GetTransport())
		transport->RemovePassenger(this);
}

void WorldObject::Update(uint32 update_diff, uint32 /*time_diff*/)
{
    elunaEvents->Update(update_diff);
}

void WorldObject::_Create(uint32 guidlow, HighGuid guidhigh)
{
    Object::_Create(guidlow, 0, guidhigh);
}

void WorldObject::Relocate(float x, float y, float z, float orientation)
{
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
    m_position.o = orientation;

   // if (isType(TYPEMASK_UNIT))
   //     ((Unit*)this)->m_movementInfo.ChangePosition(x, y, z, orientation);

	m_movementInfo.ChangePosition(x, y, z, orientation);
	m_movementInfo.UpdateTime(WorldTimer::getMSTime());
	/*if (Transport* t = GetTransport())
	{
	t->CalculatePassengerOffset(x, y, z);
	m_movementInfo.t_pos.x = x;
	m_movementInfo.t_pos.y = y;
	m_movementInfo.t_pos.z = z;
	}*/
}

void WorldObject::Relocate(float x, float y, float z)
{
	Relocate(x, y, z, GetOrientation());
}

void WorldObject::SetOrientation(float orientation)
{
    m_position.o = orientation;

    if (isType(TYPEMASK_UNIT))
        ((Unit*)this)->m_movementInfo.ChangeOrientation(orientation);
}

uint32 WorldObject::GetZoneId() const
{
    return GetTerrain()->GetZoneId(m_position.x, m_position.y, m_position.z);
}

uint32 WorldObject::GetAreaId() const
{
    return GetTerrain()->GetAreaId(m_position.x, m_position.y, m_position.z);
}

void WorldObject::GetZoneAndAreaId(uint32& zoneid, uint32& areaid) const
{
    GetTerrain()->GetZoneAndAreaId(zoneid, areaid, m_position.x, m_position.y, m_position.z);
}

InstanceData* WorldObject::GetInstanceData() const
{
    return GetMap()->GetInstanceData();
}

// slow
float WorldObject::GetExactDistance(const WorldObject* obj) const
{
	ASSERT(obj);
	float dx = GetPositionX() - obj->GetPositionX();
	float dy = GetPositionY() - obj->GetPositionY();
	float dz = GetPositionZ() - obj->GetPositionZ();
	float dist = sqrt((dx * dx) + (dy * dy) + (dz * dz));
	return dist;
}


float WorldObject::GetExactDistance(float x, float y, float z) const
{
	float dx = GetPositionX() - x;
	float dy = GetPositionY() - y;
	float dz = GetPositionZ() - z;
	float dist = sqrt((dx * dx) + (dy * dy) + (dz * dz));
	return dist;
}

float WorldObject::GetDistance(const WorldObject* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float dz = GetPositionZ() - obj->GetPositionZ();
    float sizefactor = GetObjectBoundingRadius() + obj->GetObjectBoundingRadius();
    float dist = sqrt((dx * dx) + (dy * dy) + (dz * dz)) - sizefactor;
    return (dist > 0 ? dist : 0);
}

float WorldObject::GetDistance2d(float x, float y) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float sizefactor = GetObjectBoundingRadius();
    float dist = sqrt((dx * dx) + (dy * dy)) - sizefactor;
    return (dist > 0 ? dist : 0);
}

float WorldObject::GetDistance(float x, float y, float z) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float sizefactor = GetObjectBoundingRadius();
    float dist = sqrt((dx * dx) + (dy * dy) + (dz * dz)) - sizefactor;
    return (dist > 0 ? dist : 0);
}

float WorldObject::GetDistance2d(const WorldObject* obj) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float sizefactor = GetObjectBoundingRadius() + obj->GetObjectBoundingRadius();
    float dist = sqrt((dx * dx) + (dy * dy)) - sizefactor;
    return (dist > 0 ? dist : 0);
}

float WorldObject::GetDistanceZ(const WorldObject* obj) const
{
    float dz = fabs(GetPositionZ() - obj->GetPositionZ());
    float sizefactor = GetObjectBoundingRadius() + obj->GetObjectBoundingRadius();
    float dist = dz - sizefactor;
    return (dist > 0 ? dist : 0);
}

bool WorldObject::IsWithinDist3d(float x, float y, float z, float dist2compare) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float distsq = dx * dx + dy * dy + dz * dz;

    float sizefactor = GetObjectBoundingRadius();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::IsWithinDist2d(float x, float y, float dist2compare) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float distsq = dx * dx + dy * dy;

    float sizefactor = GetObjectBoundingRadius();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::_IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float distsq = dx * dx + dy * dy;
    if (is3D)
    {
        float dz = GetPositionZ() - obj->GetPositionZ();
        distsq += dz * dz;
    }
    float sizefactor = GetObjectBoundingRadius() + obj->GetObjectBoundingRadius();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::IsWithinLOSInMap(const WorldObject* obj) const
{
    if (!IsInMap(obj)) return false;
    float ox, oy, oz;
    obj->GetPosition(ox, oy, oz);
    return IsWithinLOS(ox, oy, oz);
}

bool WorldObject::IsWithinLOS(float ox, float oy, float oz) const
{
    float x, y, z;
    GetPosition(x, y, z);
    return GetMap()->IsInLineOfSight(x, y, z + 2.0f, ox, oy, oz + 2.0f);
}

bool WorldObject::GetDistanceOrder(WorldObject const* obj1, WorldObject const* obj2, bool is3D /* = true */) const
{
    float dx1 = GetPositionX() - obj1->GetPositionX();
    float dy1 = GetPositionY() - obj1->GetPositionY();
    float distsq1 = dx1 * dx1 + dy1 * dy1;
    if (is3D)
    {
        float dz1 = GetPositionZ() - obj1->GetPositionZ();
        distsq1 += dz1 * dz1;
    }

    float dx2 = GetPositionX() - obj2->GetPositionX();
    float dy2 = GetPositionY() - obj2->GetPositionY();
    float distsq2 = dx2 * dx2 + dy2 * dy2;
    if (is3D)
    {
        float dz2 = GetPositionZ() - obj2->GetPositionZ();
        distsq2 += dz2 * dz2;
    }

    return distsq1 < distsq2;
}

bool WorldObject::IsInRange(WorldObject const* obj, float minRange, float maxRange, bool is3D /* = true */) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float distsq = dx * dx + dy * dy;
    if (is3D)
    {
        float dz = GetPositionZ() - obj->GetPositionZ();
        distsq += dz * dz;
    }

    float sizefactor = GetObjectBoundingRadius() + obj->GetObjectBoundingRadius();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

bool WorldObject::IsInRange2d(float x, float y, float minRange, float maxRange) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float distsq = dx * dx + dy * dy;

    float sizefactor = GetObjectBoundingRadius();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

bool WorldObject::IsInRange3d(float x, float y, float z, float minRange, float maxRange) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float distsq = dx * dx + dy * dy + dz * dz;

    float sizefactor = GetObjectBoundingRadius();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

float WorldObject::GetAngle(const WorldObject* obj) const
{
    if (!obj)
        return 0.0f;

    // Rework the assert, when more cases where such a call can happen have been fixed
    // MANGOS_ASSERT(obj != this || PrintEntryError("GetAngle (for self)"));
    if (obj == this)
    {
        sLog.outError("INVALID CALL for GetAngle for %s", obj->GetGuidStr().c_str());
        return 0.0f;
    }
    return GetAngle(obj->GetPositionX(), obj->GetPositionY());
}

// Return angle in range 0..2*pi
float WorldObject::GetAngle(const float x, const float y) const
{
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();

    float ang = atan2(dy, dx);                              // returns value between -Pi..Pi
    ang = (ang >= 0) ? ang : 2 * M_PI_F + ang;
    return ang;
}

bool WorldObject::HasInArc(const float arcangle, const WorldObject* obj) const
{
    // always have self in arc
    if (obj == this)
        return true;

    float arc = arcangle;

    // move arc to range 0.. 2*pi
    arc = MapManager::NormalizeOrientation(arc);

    float angle = GetAngle(obj);
    angle -= m_position.o;

    // move angle to range -pi ... +pi
    angle = MapManager::NormalizeOrientation(angle);
    if (angle > M_PI_F)
        angle -= 2.0f * M_PI_F;

    float lborder =  -1 * (arc / 2.0f);                     // in range -pi..0
    float rborder = (arc / 2.0f);                           // in range 0..pi
    return ((angle >= lborder) && (angle <= rborder));
}

bool WorldObject::IsFacingTargetsBack(const WorldObject* target, float arc /*= M_PI_F*/) const
{
    if (!target)
        return false;

    //if target is facing the current object then we know its not possible that the current object would be facing the targets back
    if (target->HasInArc(arc, this))
        return false;

    //if current object is not facing the target then we know the current object is not facing the target at all
    if (!this->HasInArc(arc, target))
        return false;

    return true;
}

bool WorldObject::IsFacingTargetsFront(const WorldObject* target, float arc /*= M_PI_F*/) const
{
    if (!target)
        return false;

    //if target is not facing the current object then we know its not possible that the current object would be facing the targets front
    if (!target->HasInArc(arc, this))
        return false;

    //if current object is not facing the target then we know the current object is not facing the target at all
    if (!this->HasInArc(arc, target))
        return false;

    return true;
}

bool WorldObject::isInFrontInMap(WorldObject const* target, float distance,  float arc) const
{
    return IsWithinDistInMap(target, distance) && HasInArc(arc, target);
}

bool WorldObject::isInBackInMap(WorldObject const* target, float distance, float arc) const
{
    return IsWithinDistInMap(target, distance) && !HasInArc(2 * M_PI_F - arc, target);
}

bool WorldObject::isInFront(WorldObject const* target, float distance,  float arc) const
{
    return IsWithinDist(target, distance) && HasInArc(arc, target);
}

bool WorldObject::isInBack(WorldObject const* target, float distance, float arc) const
{
    return IsWithinDist(target, distance) && !HasInArc(2 * M_PI_F - arc, target);
}

bool WorldObject::GetRandomPoint(float x, float y, float z, float distance, float& rand_x, float& rand_y, float& rand_z, float minDist /*=0.0f*/, float const* ori /*=nullptr*/) const
{
	/*
	if (distance == 0)
	{
		rand_x = x;
		rand_y = y;
		rand_z = z;
		return;
	}

	float angle;
	if (!ori)
		angle = rand_norm_f() * 2 * M_PI_F;
	else
		angle = *ori;

	float new_dist;
	if (minDist == 0.0f)
		new_dist = rand_norm_f() * distance;
	else
		new_dist = minDist + rand_norm_f() * (distance - minDist);

	rand_x = x + new_dist * cos(angle);
	rand_y = y + new_dist * sin(angle);
	rand_z = z;

	MaNGOS::NormalizeMapCoord(rand_x);
	MaNGOS::NormalizeMapCoord(rand_y);
	UpdateGroundPositionZ(rand_x, rand_y, rand_z);          // update to LOS height if available
	*/

	if (distance < 0.1f)
	{
		rand_x = x;
		rand_y = y;
		rand_z = z;
		return true;
	}
	ASSERT(FindMap());
	Map const* map = GetMap();

	bool is_air_ok = isType(TYPEMASK_UNIT) ? ((Unit*)this)->CanFly() : false;

	if (is_air_ok)
	{
		if (!ori)
		{
			float randAngle1 = rand_norm_f() * 2 * M_PI;
			float randAngle2 = rand_norm_f() * 2 * M_PI;
			float randDist = rand_norm_f() * distance;
			rand_x = x + randDist * cos(randAngle1) * sin(randAngle2);
			rand_y = y + randDist * sin(randAngle2) * sin(randAngle2);
			rand_z = z + randDist * sin(randAngle2);
		}
		else
		{
			float angle = *ori;
			float new_dist;
			if (minDist == 0.0f)
				new_dist = rand_norm_f() * distance;
			else
				new_dist = minDist + rand_norm_f() * (distance - minDist);

			rand_x = x + new_dist * cos(angle);
			rand_y = y + new_dist * sin(angle);
			rand_z = z;
		}
		// May happen in the border of the map
		if (!MaNGOS::IsValidMapCoord(x, y, z) || !MaNGOS::IsValidMapCoord(rand_x, rand_y, rand_z))
			return false;
		map->GetLosHitPosition(x, y, z, rand_x, rand_y, rand_z, -0.5f);
		return true;
	}
	else
	{
		uint32 moveAllowed = NAV_GROUND | NAV_WATER;
		if (GetTypeId() != TYPEID_PLAYER)
			moveAllowed |= NAV_MAGMA | NAV_SLIME;
		bool is_water_ok = isType(TYPEMASK_UNIT) ? ((Unit*)this)->CanSwim() : false;
		rand_x = x;
		rand_y = y;
		rand_z = z;
		if (map->GetWalkRandomPosition(GetTransport(), rand_x, rand_y, rand_z, distance, moveAllowed))
		{
			if (!is_water_ok)
				return true;
			// La position renvoyee par le pathfinding est tout au fond de l'eau. On randomise ca un peu ...
			float ground = 0.0f;
			float waterSurface = GetTerrain()->GetWaterLevel(x, y, z, &ground);
			if (waterSurface == VMAP_INVALID_HEIGHT_VALUE)
				return true;
			if (ground > waterSurface) // Possible ?
				return true;
			rand_z += rand_norm_f() * distance / 2.0f;
			if (rand_z < ground)
				rand_z = ground;
			// Ici 'is_air_ok' = false, donc on reste SOUS l'eau.
			if (rand_z > waterSurface)
				rand_z = waterSurface;
			return true;
		}
		rand_x = x;
		rand_y = y;
		rand_z = z;
		return false;
	}
}

void WorldObject::UpdateGroundPositionZ(float x, float y, float& z) const
{
    float new_z = GetMap()->GetHeight(x, y, z);
    if (new_z > INVALID_HEIGHT)
        z = new_z + 0.05f;                                  // just to be sure that we are not a few pixel under the surface
}

void WorldObject::UpdateAllowedPositionZ(float x, float y, float& z, Map* atMap /*=nullptr*/) const
{
    if (!atMap)
        atMap = GetMap();

    switch (GetTypeId())
    {
        case TYPEID_UNIT:
        {
            // non fly unit don't must be in air
            // non swim unit must be at ground (mostly speedup, because it don't must be in water and water level check less fast
            if (!((Creature const*)this)->CanFly())
            {
                bool canSwim = ((Creature const*)this)->CanSwim();
                float ground_z = z;
                float max_z = canSwim
                              ? atMap->GetTerrain()->GetWaterOrGroundLevel(x, y, z, &ground_z, !((Unit const*)this)->HasAuraType(SPELL_AURA_WATER_WALK))
                              : ((ground_z = atMap->GetHeight(x, y, z)));
                if (max_z > INVALID_HEIGHT)
                {
                    if (z > max_z)
                        z = max_z;
                    else if (z < ground_z)
                        z = ground_z;
                }
            }
            else
            {
                float ground_z = atMap->GetHeight(x, y, z);
                if (z < ground_z)
                    z = ground_z;
            }
            break;
        }
        case TYPEID_PLAYER:
        {
            // for server controlled moves player work same as creature (but it can always swim)
            {
                float ground_z = z;
                float max_z = atMap->GetTerrain()->GetWaterOrGroundLevel(x, y, z, &ground_z, !((Unit const*)this)->HasAuraType(SPELL_AURA_WATER_WALK));
                if (max_z > INVALID_HEIGHT)
                {
                    if (z > max_z)
                        z = max_z;
                    else if (z < ground_z)
                        z = ground_z;
                }
            }
            break;
        }
        default:
        {
            float ground_z = atMap->GetHeight(x, y, z);
            if (ground_z > INVALID_HEIGHT)
                z = ground_z;
            break;
        }
    }
}

bool WorldObject::IsPositionValid() const
{
    return MaNGOS::IsValidMapCoord(m_position.x, m_position.y, m_position.z, m_position.o);
}

void WorldObject::MonsterSay(const char* text, uint32 /*language*/, Unit const* target) const
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_MONSTER_SAY, text, LANG_UNIVERSAL, CHAT_TAG_NONE, GetObjectGuid(), GetName(),
                                 target ? target->GetObjectGuid() : ObjectGuid(), target ? target->GetName() : "");
    SendMessageToSetInRange(data, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_SAY), true);
}

void WorldObject::MonsterYell(const char* text, uint32 /*language*/, Unit const* target) const
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_MONSTER_YELL, text, LANG_UNIVERSAL, CHAT_TAG_NONE, GetObjectGuid(), GetName(),
                                 target ? target->GetObjectGuid() : ObjectGuid(), target ? target->GetName() : "");
    SendMessageToSetInRange(data, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_YELL), true);
}

void WorldObject::MonsterTextEmote(const char* text, Unit const* target, bool IsBossEmote) const
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, IsBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, text, LANG_UNIVERSAL, CHAT_TAG_NONE, GetObjectGuid(), GetName(),
                                 target ? target->GetObjectGuid() : ObjectGuid(), target ? target->GetName() : "");
    SendMessageToSetInRange(data, sWorld.getConfig(IsBossEmote ? CONFIG_FLOAT_LISTEN_RANGE_YELL : CONFIG_FLOAT_LISTEN_RANGE_TEXTEMOTE), true);
}

void WorldObject::MonsterWhisper(const char* text, Unit const* target, bool IsBossWhisper) const
{
    if (!target || target->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, IsBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_WHISPER, text, LANG_UNIVERSAL, CHAT_TAG_NONE, GetObjectGuid(), GetName(),
                                 target->GetObjectGuid(), target->GetName());
    ((Player*)target)->GetSession()->SendPacket(data);
}

namespace MaNGOS
{
    class MonsterChatBuilder
    {
        public:
            MonsterChatBuilder(WorldObject const& obj, ChatMsg msgtype, MangosStringLocale const* textData, Language language, Unit const* target)
                : i_object(obj), i_msgtype(msgtype), i_textData(textData), i_language(language), i_target(target) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* text;
                if ((int32)i_textData->Content.size() > loc_idx + 1 && !i_textData->Content[loc_idx + 1].empty())
                    text = i_textData->Content[loc_idx + 1].c_str();
                else
                    text = i_textData->Content[0].c_str();

                ChatHandler::BuildChatPacket(data, i_msgtype, text, i_language, CHAT_TAG_NONE, i_object.GetObjectGuid(), i_object.GetNameForLocaleIdx(loc_idx),
                                             i_target ? i_target->GetObjectGuid() : ObjectGuid(), i_target ? i_target->GetNameForLocaleIdx(loc_idx) : "");
            }

        private:
            WorldObject const& i_object;
            ChatMsg i_msgtype;
            MangosStringLocale const* i_textData;
            Language i_language;
            Unit const* i_target;
    };
}                                                           // namespace MaNGOS

/// Helper function to create localized around a source
void _DoLocalizedTextAround(WorldObject const* source, MangosStringLocale const* textData, ChatMsg msgtype, Language language, Unit const* target, float range)
{
    MaNGOS::MonsterChatBuilder say_build(*source, msgtype, textData, language, target);
    MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder> say_do(say_build);
    MaNGOS::CameraDistWorker<MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder> > say_worker(source, range, say_do);
    Cell::VisitWorldObjects(source, say_worker, range);
}

/// Function that sends a text associated to a MangosString
void WorldObject::MonsterText(MangosStringLocale const* textData, Unit const* target) const
{
    MANGOS_ASSERT(textData);

    switch (textData->Type)
    {
        case CHAT_TYPE_SAY:
            _DoLocalizedTextAround(this, textData, CHAT_MSG_MONSTER_SAY, textData->LanguageId, target, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_SAY));
            break;
        case CHAT_TYPE_YELL:
            _DoLocalizedTextAround(this, textData, CHAT_MSG_MONSTER_YELL, textData->LanguageId, target, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_YELL));
            break;
        case CHAT_TYPE_TEXT_EMOTE:
            _DoLocalizedTextAround(this, textData, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, target, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_TEXTEMOTE));
            break;
        case CHAT_TYPE_BOSS_EMOTE:
            _DoLocalizedTextAround(this, textData, CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, target, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_YELL));
            break;
        case CHAT_TYPE_WHISPER:
        {
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return;
            MaNGOS::MonsterChatBuilder say_build(*this, CHAT_MSG_MONSTER_WHISPER, textData, LANG_UNIVERSAL, target);
            MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder> say_do(say_build);
            say_do((Player*)target);
            break;
        }
        case CHAT_TYPE_BOSS_WHISPER:
        {
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return;
            MaNGOS::MonsterChatBuilder say_build(*this, CHAT_MSG_RAID_BOSS_WHISPER, textData, LANG_UNIVERSAL, target);
            MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder> say_do(say_build);
            say_do((Player*)target);
            break;
        }
        case CHAT_TYPE_ZONE_YELL:
        {
            MaNGOS::MonsterChatBuilder say_build(*this, CHAT_MSG_MONSTER_YELL, textData, textData->LanguageId, target);
            MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder> say_do(say_build);
            uint32 zoneid = GetZoneId();
            Map::PlayerList const& pList = GetMap()->GetPlayers();
            for (Map::PlayerList::const_iterator itr = pList.begin(); itr != pList.end(); ++itr)
                if (itr->getSource()->GetZoneId() == zoneid)
                    say_do(itr->getSource());
            break;
        }
    }
}

void WorldObject::SendMessageToSet(WorldPacket const& data, bool /*bToSelf*/) const
{
    // if object is in world, map for it already created!
    if (IsInWorld())
        GetMap()->MessageBroadcast(this, data);
}

struct ObjectViewersDeliverer
{
	WorldPacket i_message;
	WorldObject* i_sender;
	WorldObject* i_except;
	explicit ObjectViewersDeliverer(WorldObject* sender, WorldPacket msg, WorldObject* except) : i_message(msg), i_sender(sender), i_except(except) {}
	void Visit(CameraMapType &m)
	{
		for (auto iter = m.begin(); iter != m.end(); ++iter)
			if (Player* player = iter->getSource()->GetOwner())
				if (player != i_except && player != i_sender)
					if (player->IsInVisibleList_Unsafe(i_sender))
						player->GetSession()->SendPacket(i_message);
	}
	template<class SKIP> void Visit(GridRefManager<SKIP> &) {}
};

void WorldObject::SendObjectMessageToSet(WorldPacket const& data, bool self, WorldObject* except)
{
	if (self && this != except)
		if (Player* me = ToPlayer())
			me->GetSession()->SendPacket(data);

	if (!IsInWorld())
		return;

	CellPair p = MaNGOS::ComputeCellPair(GetPositionX(), GetPositionY());

	if (p.x_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP || p.y_coord >= TOTAL_NUMBER_OF_CELLS_PER_MAP)
		return;

	Cell cell(p);
	cell.SetNoCreate();

	if (!GetMap()->IsLoaded(GetPositionX(), GetPositionY()))
		return;

	ObjectViewersDeliverer post_man(this, data, except);
	TypeContainerVisitor<ObjectViewersDeliverer, WorldTypeMapContainer> message(post_man);
	cell.Visit(p, message, *GetMap(), *this, GetMap()->GetVisibilityDistance());
}

void WorldObject::SendMovementMessageToSet(WorldPacket const& data, bool self, WorldObject* except)
{
	if (!IsPlayer() || !sWorld.GetBroadcaster()->IsEnabled())
		SendObjectMessageToSet(data, true, except);
	else
	{
		auto player_broadcast = ToPlayer()->m_broadcaster;

		if (player_broadcast)
			player_broadcast->QueuePacket(std::move(data), self, except ? except->GetObjectGuid() : ObjectGuid());
	}
}
void WorldObject::SendMessageToSetInRange(WorldPacket const& data, float dist, bool /*bToSelf*/) const
{
    // if object is in world, map for it already created!
    if (IsInWorld())
        GetMap()->MessageDistBroadcast(this, data, dist);
}

void WorldObject::SendMessageToSetExcept(WorldPacket& data, Player const* skipped_receiver) const
{
    // if object is in world, map for it already created!
    if (IsInWorld())
    {
        MaNGOS::MessageDelivererExcept notifier(data, skipped_receiver);
        Cell::VisitWorldObjects(this, notifier, GetMap()->GetVisibilityDistance());
    }
}

void WorldObject::SendObjectDeSpawnAnim(ObjectGuid guid) const
{
    WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
    data << ObjectGuid(guid);
    SendMessageToSet(data, true);
}

void WorldObject::SendGameObjectCustomAnim(ObjectGuid guid, uint32 animId /*= 0*/) const
{
    WorldPacket data(SMSG_GAMEOBJECT_CUSTOM_ANIM, 8 + 4);
    data << ObjectGuid(guid);
    data << uint32(animId);
    SendMessageToSet(data, true);
}

bool WorldObject::isWithinVisibilityDistanceOf(Unit const* viewer, WorldObject const* viewPoint, bool inVisibleList) const
{
	if (viewer->IsTaxiFlying())
	{
		// use object grey distance for all (only see objects any way)
		if (!IsWithinDistInMap(viewPoint, World::GetMaxVisibleDistanceInFlight() + (inVisibleList ? World::GetVisibleObjectGreyDistance() : 0.0f), false))
			return false;
	}
	else if (!GetTransport() || GetTransport() != viewer->GetTransport())
	{
		// Any units far than max visible distance for viewer or not in our map are not visible too
		if (!IsWithinDistInMap(viewPoint, GetMap()->GetVisibilityDistance() + (inVisibleList ? World::GetVisibleUnitGreyDistance() : 0.0f), false))
			return false;
	}
	return true;
}

void WorldObject::SetMap(Map* map)
{
    MANGOS_ASSERT(map);
    m_currMap = map;
    // lets save current map's Id/instanceId
    m_mapId = map->GetId();
    m_InstanceId = map->GetInstanceId();

    delete elunaEvents;
    // On multithread replace this with a pointer to map's Eluna pointer stored in a map
    elunaEvents = new ElunaEventProcessor(&Eluna::GEluna, this);
}

void WorldObject::ResetMap()
{
    delete elunaEvents;
    elunaEvents = NULL;

    m_currMap = NULL;
}

TerrainInfo const* WorldObject::GetTerrain() const
{
    MANGOS_ASSERT(m_currMap);
    return m_currMap->GetTerrain();
}

void WorldObject::AddObjectToRemoveList()
{
    GetMap()->AddObjectToRemoveList(this);
}

Creature* WorldObject::SummonCreature(uint32 id, float x, float y, float z, float ang, TempSummonType spwtype, uint32 despwtime, bool asActiveObject, bool setRun)
{
    CreatureInfo const* cinfo = ObjectMgr::GetCreatureTemplate(id);
    if (!cinfo)
    {
        sLog.outErrorDb("WorldObject::SummonCreature: Creature (Entry: %u) not existed for summoner: %s. ", id, GetGuidStr().c_str());
        return nullptr;
    }

    TemporarySummon* pCreature = new TemporarySummon(GetObjectGuid());

    Team team = TEAM_NONE;
    if (GetTypeId() == TYPEID_PLAYER)
        team = ((Player*)this)->GetTeam();

    CreatureCreatePos pos(GetMap(), x, y, z, ang);

    if (x == 0.0f && y == 0.0f && z == 0.0f)
        pos = CreatureCreatePos(this, GetOrientation(), CONTACT_DISTANCE, ang);

    if (!pCreature->Create(GetMap()->GenerateLocalLowGuid(cinfo->GetHighGuid()), pos, cinfo, team))
    {
        delete pCreature;
        return nullptr;
    }

    pCreature->SetRespawnCoord(pos);

    // Set run or walk before any other movement starts
    pCreature->SetWalk(!setRun);

    // Active state set before added to map
    pCreature->SetActiveObjectState(asActiveObject);

    pCreature->Summon(spwtype, despwtime);                  // Also initializes the AI and MMGen

    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->AI())
        ((Creature*)this)->AI()->JustSummoned(pCreature);
    if (Unit* summoner = ToUnit())
        sEluna->OnSummoned(pCreature, summoner);

    // Creature Linking, Initial load is handled like respawn
    if (pCreature->IsLinkingEventTrigger())
        GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_RESPAWN, pCreature);

    // return the creature therewith the summoner has access to it
    return pCreature;
}

GameObject* WorldObject::SummonGameObject(uint32 id, float x, float y, float z, float angle, uint32 despwtime)
{
    if (!IsInWorld())
        return NULL;

    if (!sObjectMgr.GetGameObjectInfo(id))
    {
        sLog.outErrorDb("WorldObject::SummonGameObject: GameObject (Entry: %u) not existed for summoner: %s. ", id, GetGuidStr().c_str());
        return NULL;
    }

    Map *map = GetMap();
    GameObject* pGameObj = new GameObject;
    if (!pGameObj->Create(map->GenerateLocalLowGuid(HIGHGUID_GAMEOBJECT), id, map, x, y, z, angle))
    {
        delete pGameObj;
        return NULL;
    }

    pGameObj->SetRespawnTime(despwtime / IN_MILLISECONDS);
    if (GetTypeId() == TYPEID_PLAYER || GetTypeId() == TYPEID_UNIT)
        ToUnit()->AddGameObject(pGameObj);
    else
        pGameObj->SetSpawnedByDefault(false);

    map->Add(pGameObj);

    return pGameObj;
}

// how much space should be left in front of/ behind a mob that already uses a space
#define OCCUPY_POS_DEPTH_FACTOR                          1.8f

namespace MaNGOS
{
    class NearUsedPosDo
    {
        public:
            NearUsedPosDo(WorldObject const& obj, WorldObject const* searcher, float absAngle, ObjectPosSelector& selector)
                : i_object(obj), i_searcher(searcher), i_absAngle(MapManager::NormalizeOrientation(absAngle)), i_selector(selector) {}

            void operator()(Corpse*) const {}
            void operator()(DynamicObject*) const {}

            void operator()(Creature* c) const
            {
                // skip self or target
                if (c == i_searcher || c == &i_object)
                    return;

                float x, y, z;

                if (c->IsStopped() || !c->GetMotionMaster()->GetDestination(x, y, z))
                {
                    x = c->GetPositionX();
                    y = c->GetPositionY();
                }

                add(c, x, y);
            }

            template<class T>
            void operator()(T* u) const
            {
                // skip self or target
                if (u == i_searcher || u == &i_object)
                    return;

                float x, y;

                x = u->GetPositionX();
                y = u->GetPositionY();

                add(u, x, y);
            }

            // we must add used pos that can fill places around center
            void add(WorldObject* u, float x, float y) const
            {
                float dx = i_object.GetPositionX() - x;
                float dy = i_object.GetPositionY() - y;
                float dist2d = sqrt((dx * dx) + (dy * dy));

                // It is ok for the objects to require a bit more space
                float delta = u->GetObjectBoundingRadius();
                if (i_selector.m_searchPosFor && i_selector.m_searchPosFor != u)
                    delta += i_selector.m_searchPosFor->GetObjectBoundingRadius();

                delta *= OCCUPY_POS_DEPTH_FACTOR;           // Increase by factor

                // u is too near/far away from i_object. Do not consider it to occupy space
                if (fabs(i_selector.m_searcherDist - dist2d) > delta)
                    return;

                float angle = i_object.GetAngle(u) - i_absAngle;

                // move angle to range -pi ... +pi, range before is -2Pi..2Pi
                if (angle > M_PI_F)
                    angle -= 2.0f * M_PI_F;
                else if (angle < -M_PI_F)
                    angle += 2.0f * M_PI_F;

                i_selector.AddUsedArea(u, angle, dist2d);
            }
        private:
            WorldObject const& i_object;
            WorldObject const* i_searcher;
            float              i_absAngle;
            ObjectPosSelector& i_selector;
    };
}                                                           // namespace MaNGOS

//===================================================================================================

void WorldObject::GetNearPoint2D(float& x, float& y, float distance2d, float absAngle) const
{
    x = GetPositionX() + distance2d * cos(absAngle);
    y = GetPositionY() + distance2d * sin(absAngle);

    MaNGOS::NormalizeMapCoord(x);
    MaNGOS::NormalizeMapCoord(y);
}

void WorldObject::GetNearPoint(WorldObject const* searcher, float& x, float& y, float& z, float searcher_bounding_radius, float distance2d, float absAngle) const
{
    GetNearPoint2D(x, y, distance2d, absAngle);
    const float init_z = z = GetPositionZ();

    // if detection disabled, return first point
    if (!sWorld.getConfig(CONFIG_BOOL_DETECT_POS_COLLISION))
    {
        if (searcher)
            searcher->UpdateAllowedPositionZ(x, y, z, GetMap()); // update to LOS height if available
        else
            UpdateGroundPositionZ(x, y, z);
        return;
    }

    // or remember first point
    float first_x = x;
    float first_y = y;
    bool first_los_conflict = false;                        // first point LOS problems

    const float dist = distance2d + searcher_bounding_radius + GetObjectBoundingRadius();

    // prepare selector for work
    ObjectPosSelector selector(GetPositionX(), GetPositionY(), distance2d, searcher_bounding_radius, searcher);

    // adding used positions around object
    {
        MaNGOS::NearUsedPosDo u_do(*this, searcher, absAngle, selector);
        MaNGOS::WorldObjectWorker<MaNGOS::NearUsedPosDo> worker(u_do);

        Cell::VisitAllObjects(this, worker, dist);
    }

    // maybe can just place in primary position
    if (selector.CheckOriginalAngle())
    {
        if (searcher)
            searcher->UpdateAllowedPositionZ(x, y, z, GetMap()); // update to LOS height if available
        else
            UpdateGroundPositionZ(x, y, z);

        if (fabs(init_z - z) < dist && IsWithinLOS(x, y, z))
            return;

        first_los_conflict = true;                          // first point have LOS problems
    }

    // set first used pos in lists
    selector.InitializeAngle();

    float angle;                                            // candidate of angle for free pos

    // select in positions after current nodes (selection one by one)
    while (selector.NextAngle(angle))                       // angle for free pos
    {
        GetNearPoint2D(x, y, distance2d, absAngle + angle);
        z = GetPositionZ();

        if (searcher)
            searcher->UpdateAllowedPositionZ(x, y, z, GetMap()); // update to LOS height if available
        else
            UpdateGroundPositionZ(x, y, z);

        if (fabs(init_z - z) < dist && IsWithinLOS(x, y, z))
            return;
    }

    // BAD NEWS: not free pos (or used or have LOS problems)
    // Attempt find _used_ pos without LOS problem
    if (!first_los_conflict)
    {
        x = first_x;
        y = first_y;

        if (searcher)
            searcher->UpdateAllowedPositionZ(x, y, z, GetMap()); // update to LOS height if available
        else
            UpdateGroundPositionZ(x, y, z);
        return;
    }

    // set first used pos in lists
    selector.InitializeAngle();

    // select in positions after current nodes (selection one by one)
    while (selector.NextUsedAngle(angle))                   // angle for used pos but maybe without LOS problem
    {
        GetNearPoint2D(x, y, distance2d, absAngle + angle);
        z = GetPositionZ();

        if (searcher)
            searcher->UpdateAllowedPositionZ(x, y, z, GetMap()); // update to LOS height if available
        else
            UpdateGroundPositionZ(x, y, z);

        if (fabs(init_z - z) < dist && IsWithinLOS(x, y, z))
            return;
    }

    // BAD BAD NEWS: all found pos (free and used) have LOS problem :(
    x = first_x;
    y = first_y;

    if (searcher)
        searcher->UpdateAllowedPositionZ(x, y, z, GetMap());// update to LOS height if available
    else
        UpdateGroundPositionZ(x, y, z);
}

void WorldObject::PlayDistanceSound(uint32 sound_id, Player const* target /*= nullptr*/) const
{
    WorldPacket data(SMSG_PLAY_OBJECT_SOUND, 4 + 8);
    data << uint32(sound_id);
    data << GetObjectGuid();
    if (target)
        target->SendDirectMessage(data);
    else
        SendMessageToSet(data, true);
}

void WorldObject::PlayDirectSound(uint32 sound_id, Player const* target /*= nullptr*/) const
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << uint32(sound_id);
    if (target)
        target->SendDirectMessage(data);
    else
        SendMessageToSet(data, true);
}

void WorldObject::PlayMusic(uint32 sound_id, Player const* target /*= nullptr*/) const
{
	WorldPacket data(SMSG_PLAY_MUSIC, 4);
	data << uint32(sound_id);
	if (target)
		target->SendDirectMessage(data);
	else
		SendMessageToSet(data, true);
}

void WorldObject::UpdateVisibilityAndView()
{
    GetViewPoint().Call_UpdateVisibilityForOwner();
    UpdateObjectVisibility();
    GetViewPoint().Event_ViewPointVisibilityChanged();
}

void WorldObject::UpdateObjectVisibility()
{
    CellPair p = MaNGOS::ComputeCellPair(GetPositionX(), GetPositionY());
    Cell cell(p);

    GetMap()->UpdateObjectVisibility(this, cell, p);
}

void WorldObject::AddToClientUpdateList()
{
    GetMap()->AddUpdateObject(this);
}

void WorldObject::RemoveFromClientUpdateList()
{
    GetMap()->RemoveUpdateObject(this);
}

struct WorldObjectChangeAccumulator
{
    UpdateDataMapType& i_updateDatas;
    WorldObject& i_object;
    WorldObjectChangeAccumulator(WorldObject& obj, UpdateDataMapType& d) : i_updateDatas(d), i_object(obj)
    {
        // send self fields changes in another way, otherwise
        // with new camera system when player's camera too far from player, camera wouldn't receive packets and changes from player
        if (i_object.isType(TYPEMASK_PLAYER))
            i_object.BuildUpdateDataForPlayer((Player*)&i_object, i_updateDatas);
    }

    void Visit(CameraMapType& m)
    {
        for (CameraMapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        {
            Player* owner = iter->getSource()->GetOwner();
            if (owner != &i_object && owner->HaveAtClient(&i_object))
                i_object.BuildUpdateDataForPlayer(owner, i_updateDatas);
        }
    }

    template<class SKIP> void Visit(GridRefManager<SKIP>&) {}
};

void WorldObject::BuildUpdateData(UpdateDataMapType& update_players)
{
    WorldObjectChangeAccumulator notifier(*this, update_players);
    Cell::VisitWorldObjects(this, notifier, GetMap()->GetVisibilityDistance());

    ClearUpdateMask(false);
}

bool WorldObject::IsControlledByPlayer() const
{
    switch (GetTypeId())
    {
        case TYPEID_GAMEOBJECT:
            return ((GameObject*)this)->GetOwnerGuid().IsPlayer();
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
            return ((Unit*)this)->IsCharmerOrOwnerPlayerOrPlayerItself();
        case TYPEID_DYNAMICOBJECT:
            return ((DynamicObject*)this)->GetCasterGuid().IsPlayer();
        case TYPEID_CORPSE:
            return true;
        default:
            return false;
    }
}

bool WorldObject::PrintCoordinatesError(float x, float y, float z, char const* descr) const
{
    sLog.outError("%s with invalid %s coordinates: mapid = %uu, x = %f, y = %f, z = %f", GetGuidStr().c_str(), descr, GetMapId(), x, y, z);
    return false;                                           // always false for continue assert fail
}

void WorldObject::SetActiveObjectState(bool active)
{
    if (m_isActiveObject == active || (isType(TYPEMASK_PLAYER) && !active))  // player shouldn't became inactive, never
        return;

    if (IsInWorld() && !isType(TYPEMASK_PLAYER))
        // player's update implemented in a different from other active worldobject's way
        // it's considired to use generic way in future
    {
        if (isActiveObject() && !active)
            GetMap()->RemoveFromActive(this);
        else if (!isActiveObject() && active)
            GetMap()->AddToActive(this);
    }
    m_isActiveObject = active;
}

// Nostalrius
void Object::ForceValuesUpdateAtIndex(uint16 i)
{
	m_uint32Values_mirror[i] = GetUInt32Value(i) + 1; // makes server think the field changed
	AddDelayedAction(OBJECT_DELAYED_MARK_CLIENT_UPDATE);
}

void WorldObject::SetWorldMask(uint32 newMask)
{
	if (this)
		worldMask = newMask;
	// Backup - Player::SaveToDb
}
bool WorldObject::CanSeeInWorld(WorldObject const* other) const
{
	// Les GMs voient tout
	if (GetTypeId() == TYPEID_PLAYER &&
		((Player*)this)->isGameMaster())
		return true;
	if (GetGUID() == other->GetGUID())
		return true;

	return CanSeeInWorld(other->worldMask);
}
bool WorldObject::CanSeeInWorld(uint32 otherPhaseMask) const
{
	// Les GMs voient tout
	if (GetTypeId() == TYPEID_PLAYER &&
		((Player*)this)->isGameMaster())
		return true;
	// Un monde en commun ?
	if (worldMask & otherPhaseMask)
		return true;
	if (otherPhaseMask & worldMask)
		return true;
	return false;
}

// Called by Creature::DisappearAndDie
void WorldObject::DestroyForNearbyPlayers()
{
	if (!IsInWorld())
		return;

	std::list<Player*> targets;
	MaNGOS::AnyPlayerInObjectRangeCheck check(this, GetMap()->GetVisibilityDistance());
	MaNGOS::PlayerListSearcher<MaNGOS::AnyPlayerInObjectRangeCheck> searcher(targets, check);
	Cell::VisitWorldObjects(this, searcher, GetMap()->GetVisibilityDistance());
	for (std::list<Player*>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
	{
		Player *plr = (*iter);

		if (plr == this)
			continue;

		if (!plr->IsInVisibleList_Unsafe(this))
			continue;

		if (isType(TYPEMASK_UNIT) && ((Unit*)this)->GetCharmerGuid() == plr->GetObjectGuid()) // TODO: this is for puppet
			continue;

		DestroyForPlayer(plr);
		plr->m_visibleGUIDs.erase(GetGUID());

		if (ToPlayer() && ToPlayer()->m_broadcaster)
			ToPlayer()->m_broadcaster->RemoveListener(plr);
	}
}

Creature* WorldObject::FindNearestCreature(uint32 uiEntry, float range, bool alive)
{
	Creature* pCreature = nullptr;

	CellPair pair(MaNGOS::ComputeCellPair(GetPositionX(), GetPositionY()));
	Cell cell(pair);
	cell.SetNoCreate();

	MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*this, uiEntry, alive, range);
	MaNGOS::CreatureLastSearcher<MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(pCreature, creature_check);

	TypeContainerVisitor<MaNGOS::CreatureLastSearcher<MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck>, GridTypeMapContainer> creature_searcher(searcher);

	cell.Visit(pair, creature_searcher, *(GetMap()), *this, range);

	return pCreature;
}

GameObject* WorldObject::FindNearestGameObject(uint32 uiEntry, float fMaxSearchRange)
{
	GameObject* pGo = nullptr;

	CellPair pair(MaNGOS::ComputeCellPair(GetPositionX(), GetPositionY()));
	Cell cell(pair);
	cell.SetNoCreate();

	MaNGOS::NearestGameObjectEntryInObjectRangeCheck go_check(*this, uiEntry, fMaxSearchRange);
	MaNGOS::GameObjectLastSearcher<MaNGOS::NearestGameObjectEntryInObjectRangeCheck> searcher(pGo, go_check);

	TypeContainerVisitor<MaNGOS::GameObjectLastSearcher<MaNGOS::NearestGameObjectEntryInObjectRangeCheck>, GridTypeMapContainer> go_searcher(searcher);

	cell.Visit(pair, go_searcher, *(GetMap()), *this, fMaxSearchRange);

	return pGo;
}

void WorldObject::GetGameObjectListWithEntryInGrid(std::list<GameObject*>& lList, uint32 uiEntry, float fMaxSearchRange)
{
	CellPair pair(MaNGOS::ComputeCellPair(GetPositionX(), GetPositionY()));
	Cell cell(pair);
	cell.SetNoCreate();

	MaNGOS::AllGameObjectsWithEntryInRange check(this, uiEntry, fMaxSearchRange);
	MaNGOS::GameObjectListSearcher<MaNGOS::AllGameObjectsWithEntryInRange> searcher(lList, check);
	TypeContainerVisitor<MaNGOS::GameObjectListSearcher<MaNGOS::AllGameObjectsWithEntryInRange>, GridTypeMapContainer> visitor(searcher);

	cell.Visit(pair, visitor, *(GetMap()), *this, fMaxSearchRange);
}

void WorldObject::GetCreatureListWithEntryInGrid(std::list<Creature*>& lList, uint32 uiEntry, float fMaxSearchRange)
{
	CellPair pair(MaNGOS::ComputeCellPair(GetPositionX(), GetPositionY()));
	Cell cell(pair);
	cell.SetNoCreate();

	MaNGOS::AllCreaturesOfEntryInRange check(this, uiEntry, fMaxSearchRange);
	MaNGOS::CreatureListSearcher<MaNGOS::AllCreaturesOfEntryInRange> searcher(lList, check);
	TypeContainerVisitor<MaNGOS::CreatureListSearcher<MaNGOS::AllCreaturesOfEntryInRange>, GridTypeMapContainer> visitor(searcher);

	cell.Visit(pair, visitor, *(GetMap()), *this, fMaxSearchRange);
}


void WorldObject::GetRelativePositions(float avantArriere, float gaucheDroite, float hautBas, float &x, float &y, float &z)
{
	float orientation = GetOrientation() + M_PI / 2.0f;

	float x_coef = cos(orientation);
	float y_coef = sin(orientation);

	float x_range_add = cos(GetOrientation()) * avantArriere;
	float y_range_add = sin(GetOrientation()) * avantArriere;

	x = GetPositionX() + x_coef * gaucheDroite + x_range_add;
	y = GetPositionY() + y_coef * gaucheDroite + y_range_add;
	z = GetPositionZ() + hautBas;
}

void WorldObject::GetInCirclePositions(float dist, uint32 curr, uint32 total, float &x, float &y, float &z, float &o)
{
	float circleAng = (float(curr) / float(total)) * (M_PI * 2);
	x = GetPositionX() + (cos(circleAng) * dist);
	y = GetPositionY() + (sin(circleAng) * dist);
	z = GetPositionZ();
	o = circleAng - M_PI;
}

void WorldObject::GetNearRandomPositions(float distance, float &x, float &y, float &z)
{
	x = rand_norm_f() * distance;
	y = rand_norm_f() * distance;
	z = GetPositionZ();
}

void WorldObject::GetFirstCollision(float dist, float angle, float &x, float &y, float &z)
{
	x = GetPositionX();
	y = GetPositionY();
	z = GetPositionZ();

	angle += m_position.o;
	float destx, desty, destz, ground, floor;

	destx = x + dist * cos(angle);
	desty = y + dist * sin(angle);
	ground = GetMap()->GetHeight(destx, desty, MAX_HEIGHT, true);
	floor = GetMap()->GetHeight(destx, desty, z, true);
	destz = fabs(ground - z) <= fabs(floor - z) ? ground : floor;

	// check static+dynamic collision
	bool col = GetMap()->GetLosHitPosition(x, y, z + 0.5f, destx, desty, destz, -0.5f);

	// Collided
	if (col)
	{
		destx -= CONTACT_DISTANCE * cos(angle);
		desty -= CONTACT_DISTANCE * sin(angle);
		dist = sqrt((x - destx) * (x - destx) + (y - desty) * (y - desty));
	}
	float step = dist / 10.0f;

	for (uint8 j = 0; j < 10; ++j)
	{
		// do not allow too big z changes
		if (fabs(z - destz) > 6)
		{
			destx -= step * cos(angle);
			desty -= step * sin(angle);
			ground = GetMap()->GetHeight(destx, desty, MAX_HEIGHT, true);
			floor = GetMap()->GetHeight(destx, desty, z, true);
			destz = fabs(ground - z) <= fabs(floor - z) ? ground : floor;
		}
		// we have correct destz now
		else
		{
			x = destx;
			y = desty;
			z = destz;
			break;
		}
	}

	MaNGOS::NormalizeMapCoord(x);
	MaNGOS::NormalizeMapCoord(y);
	UpdateGroundPositionZ(x, y, z);
}

bool Object::IsPet() const
{
	return IsCreature() && ToCreature()->IsPet();
}

Pet const* Object::ToPet() const
{
	return IsPet() ? (Pet const*)this : NULL;
}

Pet* Object::ToPet()
{
	return IsPet() ? (Pet*)this : NULL;
}

bool WorldObject::PrintCoordinatesError(float x, float y, float z, char const* descr) const
{
	sLog.outError("%s with invalid %s coordinates: mapid = %uu, x = %f, y = %f, z = %f", GetGuidStr().c_str(), descr, GetMapId(), x, y, z);
	return false;                                           // always false for continue assert fail
}

// Look for Db GUID
Creature* WorldObject::FindNearCreature(uint32 guid, float range)
{
	Creature* creature = nullptr;
	CellPair p(MaNGOS::ComputeCellPair(GetPositionX(), GetPositionY()));
	Cell cell(p);
	cell.SetNoCreate();

	MaNGOS::CreatureWithDbGUIDCheck target_check(this, guid);
	MaNGOS::CreatureSearcher<MaNGOS::CreatureWithDbGUIDCheck> checker(creature, target_check);

	TypeContainerVisitor<MaNGOS::CreatureSearcher <MaNGOS::CreatureWithDbGUIDCheck>, GridTypeMapContainer > unit_checker(checker);
	cell.Visit(p, unit_checker, *(GetMap()), *this, range);

	return creature;
}

GameObject* WorldObject::FindNearGameObject(uint32 guid, float range)
{
	GameObject* gameObject = nullptr;

	CellPair p(MaNGOS::ComputeCellPair(GetPositionX(), GetPositionY()));
	Cell cell(p);

	MaNGOS::GameObjectWithDbGUIDCheck goCheck(*this, guid);
	MaNGOS::GameObjectSearcher<MaNGOS::GameObjectWithDbGUIDCheck> checker(gameObject, goCheck);

	TypeContainerVisitor<MaNGOS::GameObjectSearcher<MaNGOS::GameObjectWithDbGUIDCheck>, GridTypeMapContainer > objectChecker(checker);
	cell.Visit(p, objectChecker, *(GetMap()), *this, range);

	return gameObject;
}

void WorldObject::SetActiveObjectState(bool on)
{
	if (m_isActiveObject == on)
		return;

	ASSERT(GetTypeId() == TYPEID_UNIT || GetTypeId() == TYPEID_GAMEOBJECT);

	bool world = IsInWorld();

	Map* map;
	if (world)
	{
		map = GetMap();
		if (GetTypeId() == TYPEID_UNIT)
			map->Remove((Creature*)this, false);
		else
			map->Remove((GameObject*)this, false);
	}

	m_isActiveObject = on;

	if (world)
	{
		if (GetTypeId() == TYPEID_UNIT)
			map->Add((Creature*)this);
		else
			map->Add((GameObject*)this);
	}
}

void WorldObject::Update(uint32 /*update_diff*/, uint32 /*time_diff*/)
{
	ExecuteDelayedActions();
}

class NULLNotifier
{
public:
	template<class T> void Visit(GridRefManager<T> &m) {}
	void Visit(CameraMapType&) {}
};

void WorldObject::LoadMapCellsAround(float dist) const
{
	ASSERT(IsInWorld());
	NULLNotifier notifier = NULLNotifier();
	Cell::VisitAllObjects(this, notifier, dist, false);
}
