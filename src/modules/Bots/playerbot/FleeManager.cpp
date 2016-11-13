#include "../pchdef.h"
#include "playerbot.h"
#include "FleeManager.h"
#include "PlayerbotAIConfig.h"
#include "../Groups/Group.h"
#include "AiFactory.h"
#include "strategy/values/LastMovementValue.h"

using namespace ai;
using namespace std;

void FleeManager::calculateDistanceToPlayers(FleePoint *point)
{
	Group* group = bot->GetGroup();
	if (!group)
		return;

	for (GroupReference *gref = group->GetFirstMember(); gref; gref = gref->next())
    {
		Player* player = gref->GetSource();
		if(player == bot)
			continue;

        uint8 spec = AiFactory::GetPlayerSpecTab(bot);

		float d = player->GetDistance(point->x, point->y, point->z);
		point->toAllPlayers.probe(d);
		switch (player->getClass()) {
			case CLASS_HUNTER:
			case CLASS_MAGE:
			case CLASS_PRIEST:
			case CLASS_WARLOCK:
				point->toRangedPlayers.probe(d);
				break;
            case CLASS_SHAMAN:
                if (spec == 1)
                    point->toMeleePlayers.probe(d);
                else
                    point->toRangedPlayers.probe(d);
                break;
            case CLASS_DRUID:
                if (spec == 1)
                    point->toMeleePlayers.probe(d);
                else
                    point->toRangedPlayers.probe(d);
                break;
			case CLASS_PALADIN:
			    if (spec == 0)
                    point->toRangedPlayers.probe(d);
                else
                    point->toMeleePlayers.probe(d);
                break;
			case CLASS_ROGUE:
			case CLASS_WARRIOR:
            case CLASS_DEATH_KNIGHT:
				point->toMeleePlayers.probe(d);
				break;
		}
	}
}

bool FleeManager::calculateDistanceToCreatures(FleePoint *point)
{
	RangePair &distance = point->toCreatures;
    float d = 0;

	list<ObjectGuid> units = *bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<list<ObjectGuid> >("possible targets");
	for (list<ObjectGuid>::iterator i = units.begin(); i != units.end(); ++i)
    {
		Unit* unit = bot->GetPlayerbotAI()->GetUnit(*i);
		if (!unit)
		    continue;

		d = unit->GetDistance(point->x, point->y, point->z);
		if (d <= sPlayerbotAIConfig.aggroDistance)
		    continue;

		distance.probe(d);
	}

	return (d > sPlayerbotAIConfig.aggroDistance);
}

void FleeManager::calculateDistanceToDestination(FleePoint *point)
{
    RangePair &distance = point->toDestination;

	float d = bot->GetDistance(point->x, point->y, point->z);
    distance.probe(d);
}

void FleeManager::calculatePossibleDestinations(list<FleePoint*> &points)
{
	float botPosX = bot->GetPositionX();
	float botPosY = bot->GetPositionY();
	float botPosZ = bot->GetPositionZ();
	float oldPosX;
	float oldPosY;

	float maxDistance;
	float minDistance;
	float distanceStep;

	bool frontonly = 0;
	bool backonly = 0;

	uint32 mapId = bot->GetMapId();
	bool movePoint = false;

	if (bot)
		movePoint = bot->GetPlayerbotAI()->GetMovePoint(mapId, oldPosX, oldPosY, oldPosX);

    uint8 spec = AiFactory::GetPlayerSpecTab(bot);

    switch (bot->getClass()) {
			case CLASS_HUNTER:
			case CLASS_MAGE:
			case CLASS_PRIEST:
			case CLASS_WARLOCK:
				maxDistance = maxAllowedDistance;
				minDistance = sPlayerbotAIConfig.tooCloseDistance + 4.0f;

				if (maxDistance < minDistance  + 5.0f)
                    maxDistance = minDistance  + 5.0f;

				distanceStep = 5.0f;
				break;
			case CLASS_ROGUE:
			case CLASS_WARRIOR:
            case CLASS_DEATH_KNIGHT:
				maxDistance = sPlayerbotAIConfig.meleeDistance + 10.0f;
				minDistance = sPlayerbotAIConfig.tooCloseDistance + 4.0f;
				distanceStep = 3.0f;

                if (bot->GetPlayerbotAI()->IsTank(bot))
                    frontonly = 1;
                else backonly = 1;

				break;
            case CLASS_SHAMAN:
                if (spec == 1)
                {
                    maxDistance = sPlayerbotAIConfig.meleeDistance + 10.0f;
                    minDistance = sPlayerbotAIConfig.tooCloseDistance + 4.0f;
                    distanceStep = 3.0f;
                    backonly = 1;
                }
                else
                {
                    maxDistance = sPlayerbotAIConfig.meleeDistance + 10.0f;
                    minDistance = sPlayerbotAIConfig.tooCloseDistance + 4.0f;
                    distanceStep = 3.0f;

                    if (maxDistance < minDistance  + 5.0f)
                        maxDistance = minDistance  + 5.0f;

                    distanceStep = 5.0f;
                }
                break;
			case CLASS_DRUID:
			    if (spec == 1)
                {
                    maxDistance = sPlayerbotAIConfig.meleeDistance + 10.0f;
                    minDistance = sPlayerbotAIConfig.tooCloseDistance + 4.0f;
                    distanceStep = 3.0f;

                    if (bot->GetPlayerbotAI()->IsTank(bot))
                        frontonly = 1;
                    else backonly = 1;
                }
                else
                {
                    maxDistance = maxAllowedDistance;
                    minDistance = sPlayerbotAIConfig.tooCloseDistance + 5.0f;

                    if (maxDistance < minDistance  + 5.0f)
                        maxDistance = minDistance  + 5.0f;

                    distanceStep = 5.0f;
                }
                break;
            case CLASS_PALADIN:
				if (spec == 0 && bot->getLevel() > 55)
                {
                    maxDistance = maxAllowedDistance;
                    minDistance = sPlayerbotAIConfig.tooCloseDistance + 5.0f;

                    if (maxDistance < minDistance  + 5.0f)
                        maxDistance = minDistance  + 5.0f;

                    distanceStep = 5.0f;
                }
                else
                {
                    maxDistance = sPlayerbotAIConfig.meleeDistance + 10.0f;
                    minDistance = sPlayerbotAIConfig.tooCloseDistance + 4.0f;
                    distanceStep = 3.0f;

                    if (bot->GetPlayerbotAI()->IsTank(bot))
                        frontonly = 1;
                    else backonly = 1;
                }
                break;
    }

	for (float distance = maxDistance; distance >= minDistance; distance -= distanceStep)
	{
        for (float angle = followAngle; angle < followAngle + 2 * M_PI; angle += M_PI / 4)
        {

            float x = botPosX + cos(angle) * distance;
            float y = botPosY + sin(angle) * distance;
            float z = botPosZ;

            bot->UpdateAllowedPositionZ(x, y, z);
            //need to investigate
            if (abs(z - botPosZ) > 10.0f)
                continue;

            if (!bot->IsWithinLOS(x, y, z))
                continue;

			if (master && !master->IsWithinLOS(x, y, z))
				continue;

            if (master && master->GetDistance(x, y, z) > sPlayerbotAIConfig.sightDistance)
                continue;

            if (master && master->GetDistance(x,y,z) < bot->GetMinMasterDistance())
                continue;

            if (target && target->GetDistance(x,y,z) < bot->GetMinTargetDistance())
                continue;

          //  if (backonly && bot->isInFront(target, M_PI / 3.0f))
          //      continue;

           // if (frontonly && !bot->isInFront(target, M_PI / 3.0f))
           //     continue;
		//	if (movePoint && abs(bot->GetDistance(x, y, z) - bot->GetDistance(oldPosX, oldPosY, oldPosX)) < 1.0f)
		//		continue;

            FleePoint *point = new FleePoint(x, y, z);
            calculateDistanceToPlayers(point);
            calculateDistanceToDestination(point);

            if (calculateDistanceToCreatures(point))
                points.push_back(point);
        }
	}
}

void FleeManager::cleanup(list<FleePoint*> &points)
{
	for (list<FleePoint*>::iterator i = points.begin(); i != points.end(); i++)
    {
		FleePoint* point = *i;
		delete point;
	}
	points.clear();
}

bool FleePoint::isReasonable()
{
	return toAllPlayers.max <= sPlayerbotAIConfig.sightDistance && toCreatures.min >= sPlayerbotAIConfig.aggroDistance;
}

bool FleePoint::isBetterByCreatures(FleePoint* other)
{
    return toCreatures.min > 0 && other->toCreatures.min > 0 &&
            (toCreatures.min - other->toCreatures.min) >= 0;
}

bool FleePoint::isBetterByDistance(FleePoint* other)
{
    return toDestination.max > 0 && other->toDestination.max > 0 &&
            (toDestination.max - other->toDestination.max) >= 0;
}

bool FleePoint::isBetterByAll(FleePoint* other)
{
    bool isFartherFromCreatures = isBetterByCreatures(other);
    bool isCloser = isBetterByDistance(other);
    bool isNearerToRangedPlayers = toRangedPlayers.max > 0 && other->toRangedPlayers.max > 0 &&
            (toRangedPlayers.max - other->toRangedPlayers.max) <= 0;
    bool isFartherFromMeleePlayers = toMeleePlayers.min > 0 && other->toMeleePlayers.min > 0 &&
            (toMeleePlayers.min - other->toMeleePlayers.min) >= 0;

    // return isFartherFromCreatures && (isNearerToRangedPlayers || isFartherFromMeleePlayers);
    return isFartherFromCreatures && isCloser;
}

FleePoint* FleeManager::selectOptimalDestination(list<FleePoint*> &points)
{
	FleePoint* byCreatures = NULL;
	for (list<FleePoint*>::iterator i = points.begin(); i != points.end(); i++)
    {
		FleePoint* point = *i;
		if (point->isReasonable() && (!byCreatures || point->isBetterByCreatures(byCreatures)))
			byCreatures = point;
	}

	FleePoint* byAll = NULL;
	for (list<FleePoint*>::iterator i = points.begin(); i != points.end(); i++)
    {
		FleePoint* point = *i;
		if (point->isReasonable() && (!byAll || point->isBetterByAll(byAll)))
		    byAll = point;
	}

	FleePoint* byDistance = NULL;

	for (list<FleePoint*>::iterator i = points.begin(); i != points.end(); i++)
    {
		FleePoint* point = *i;
		if (point->isReasonable() && (!byDistance || point->isBetterByDistance(byDistance)))
		    byDistance = point;
	}

	if (byAll && byCreatures)
	{
	    if (byAll->isBetterByDistance(byCreatures))
	        return byAll;
        else return byCreatures;
	}
    else
        return byDistance;
}

bool FleeManager::CalculateDestination(float* rx, float* ry, float* rz)
{
    LastMovement& lastMovement = *bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<LastMovement&>("last movement");
    if ((lastMovement.lastMoveToX || lastMovement.lastMoveToY) && !lastMovement.lastFollow)
    {
        FleePoint last(lastMovement.lastMoveToX, lastMovement.lastMoveToY, lastMovement.lastMoveToZ);
        if (last.isReasonable())
        {
            *rx = lastMovement.lastMoveToX;
            *ry = lastMovement.lastMoveToY;
            *rz = lastMovement.lastMoveToZ;
            return true;
        }
    }

	list<FleePoint*> points;
	calculatePossibleDestinations(points);

    FleePoint* point = selectOptimalDestination(points);
    if (!point)
    {
        cleanup(points);
        return false;
    }

	*rx = point->x;
	*ry = point->y;
	*rz = bot->GetPositionZ();

    cleanup(points);
	return true;
}
