#include "../pchdef.h"
#include "playerbot.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "RandomPlayerbotMgr.h"


class LoginQueryHolder;
class CharacterHandler;

PlayerbotHolder::PlayerbotHolder() : PlayerbotAIBase()
{
    for (uint32 spellId = 0; spellId < sSpellStore.GetNumRows(); spellId++)
        sSpellStore.LookupEntry(spellId);
}

PlayerbotHolder::~PlayerbotHolder()
{
    LogoutAllBots();
}


void PlayerbotHolder::UpdateAIInternal(uint32 elapsed)
{
}

void PlayerbotHolder::UpdateSessions(uint32 elapsed)
{
    for (PlayerBotMap::const_iterator itr = GetPlayerBotsBegin(); itr != GetPlayerBotsEnd(); ++itr)
    {
        Player* const bot = itr->second;
        if (bot->IsBeingTeleported())
        {
            bot->GetPlayerbotAI()->HandleTeleportAck();
        }
        else if (bot->IsInWorld())
        {
            bot->GetSession()->HandleBotPackets();
        }
    }
}

void PlayerbotHolder::LogoutAllBots()
{
    while (true)
    {
        PlayerBotMap::const_iterator itr = GetPlayerBotsBegin();
        if (itr == GetPlayerBotsEnd()) break;
        Player* bot= itr->second;
        LogoutPlayerBot(bot->GetGUID());
    }
	//debug
	//_CrtDumpMemoryLeaks();
}

void PlayerbotHolder::LogoutPlayerBot(uint64 guid)
{
    Player* bot = GetPlayerBot(guid);
    if (bot)
    {
        bot->GetPlayerbotAI()->TellMaster("Goodbye!");
        //bot->SaveToDB();

        WorldSession * botWorldSessionPtr = bot->GetSession();
        playerBots.erase(guid);    // deletes bot player ptr inside this WorldSession PlayerBotMap
        botWorldSessionPtr->LogoutPlayer(true); // this will delete the bot Player object and PlayerbotAI object
        delete botWorldSessionPtr;  // finally delete the bot's WorldSession
    }
}

Player* PlayerbotHolder::GetPlayerBot(uint64 playerGuid) const
{
    PlayerBotMap::const_iterator it = playerBots.find(playerGuid);
    return (it == playerBots.end()) ? 0 : it->second;
}

void PlayerbotHolder::OnBotLogin(Player * const bot)
{
	PlayerbotAI* ai = new PlayerbotAI(bot);
	bot->SetPlayerbotAI(ai);
	OnBotLoginInternal(bot);

    playerBots[bot->GetGUID()] = bot;

    Player* master = ai->GetMaster();
    if (master)
    {
        ObjectGuid masterGuid = master->GetGUID();
        if (master->GetGroup() &&
            ! master->GetGroup()->IsLeader(masterGuid))
            master->GetGroup()->ChangeLeader(masterGuid);
    }

    Group *group = bot->GetGroup();
    if (group)
    {
        bool groupValid = false;
        Group::MemberSlotList const& slots = group->GetMemberSlots();
        for (Group::MemberSlotList::const_iterator i = slots.begin(); i != slots.end(); ++i)
        {
            ObjectGuid member = i->guid;
            uint32 account = sObjectMgr->GetPlayerAccountIdByGUID(member);
            if (!sPlayerbotAIConfig.IsInRandomAccountList(account))
            {
                groupValid = true;
                break;
            }
        }

        if (!groupValid)
        {
            WorldPacket p;
            string member = bot->GetName();
            p << uint32(PARTY_OP_LEAVE) << member << uint32(0);
            bot->GetSession()->HandleGroupDisbandOpcode(p);
        }
    }

    ai->ResetStrategies();
    ai->TellMaster("Hello!");
}


string PlayerbotHolder::ProcessBotCommand(string cmd, ObjectGuid guid, bool admin, uint32 masterAccountId, uint32 masterGuildId)
{
    if (!sPlayerbotAIConfig.enabled || guid.IsEmpty())
        return "bot system is disabled";

    uint32 botAccount = sObjectMgr->GetPlayerAccountIdByGUID(guid);
    bool isRandomBot = sRandomPlayerbotMgr.IsRandomBot(guid);
    bool isRandomAccount = sPlayerbotAIConfig.IsInRandomAccountList(botAccount);
    bool isMasterAccount = (masterAccountId == botAccount);

    if (isRandomAccount && !isRandomBot && !admin)
    {
        Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
        if (bot->GetGuildId() != masterGuildId)
            return "not in your guild";
    }

    if (!isRandomAccount && !isMasterAccount && !admin)
        return "not in your account";

    if (cmd == "add" || cmd == "login")
    {
        if (sObjectMgr->GetPlayerByLowGUID(guid))
            return "player already logged in";

        AddPlayerBot(guid.GetRawValue(), masterAccountId);
        return "ok";
    }
    else if (cmd == "remove" || cmd == "logout" || cmd == "rm")
    {
        if (!sObjectMgr->GetPlayerByLowGUID(guid))
            return "player is offline";

        if (!GetPlayerBot(guid.GetRawValue()))
            return "not your bot";

        LogoutPlayerBot(guid.GetRawValue());
        return "ok";
    }

    if (admin)
    {
        Player* bot = GetPlayerBot(guid.GetRawValue());
        if (!bot)
            return "bot not found";

        Player* master = bot->GetPlayerbotAI()->GetMaster();
        if (master)
        {
            if (cmd == "init=white" || cmd == "init=common")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_NORMAL);
                factory.CleanRandomize();
                return "ok";
            }
            else if (cmd == "init=green" || cmd == "init=uncommon")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_UNCOMMON);
                factory.CleanRandomize();
                return "ok";
            }
            else if (cmd == "init=blue" || cmd == "init=rare")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_RARE);
                factory.CleanRandomize();
                return "ok";
            }
            else if (cmd == "init=epic" || cmd == "init=purple")
            {
                PlayerbotFactory factory(bot, master->getLevel(), ITEM_QUALITY_EPIC);
                factory.CleanRandomize();
                return "ok";
            }
             else if (cmd == "init=high80")
            {
                PlayerbotFactory factory(bot, 80, ITEM_QUALITY_EPIC);
                factory.CleanBuild();
                return "ok";
            }
        }

        if (cmd == "update")
        {
            PlayerbotFactory factory(bot, bot->getLevel());
            factory.Refresh();
            return "ok";
        }
        else if (cmd == "random")
        {
            sRandomPlayerbotMgr.Randomize(bot);
			sRandomPlayerbotMgr1.Randomize(bot);
            return "ok";
        }
        else if (cmd == "distance=none")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(.0f);
            bot->SetMinMasterDistance(.0f);
            return "ok";
        }
        else if (cmd == "distance=melee")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.meleeDistance);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.meleeDistance);
            return "ok";
        }
        else if (cmd == "distance=spell")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.spellDistance);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.spellDistance);
            return "ok";
        }
        else if (cmd == "distance=close")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.closeDistance);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.closeDistance);
            return "ok";
        }
        else if (cmd == "distance=medium")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.mediumDistance);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.mediumDistance);
            return "ok";
        }
        else if (cmd == "distance=far")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.farDistance);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.farDistance);
            return "ok";
        }
        else if (cmd == "distance=extreme")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.extremeDistance);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.extremeDistance);
            return "ok";
        }
        else if (cmd == "targetdistance=none")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(.0f);
            return "ok";
        }
        else if (cmd == "targetdistance=melee")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.meleeDistance);
            return "ok";
        }
        else if (cmd == "targetdistance=spell")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.spellDistance);
            return "ok";
        }
        else if (cmd == "targetdistance=close")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.closeDistance);
            return "ok";
        }
        else if (cmd == "targetdistance=medium")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.mediumDistance);
            return "ok";
        }
        else if (cmd == "targetdistance=far")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.farDistance);
            return "ok";
        }
        else if (cmd == "targetdistance=extreme")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinTargetDistance(sPlayerbotAIConfig.extremeDistance);
            return "ok";
        }
        else if (cmd == "masterdistance=none")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinMasterDistance(.0f);
            return "ok";
        }
        else if (cmd == "masterdistance=melee")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.meleeDistance);
            return "ok";
        }
        else if (cmd == "masterdistance=spell")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.spellDistance);
            return "ok";
        }
        else if (cmd == "masterdistance=close")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.closeDistance);
            return "ok";
        }
        else if (cmd == "masterdistance=medium")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.mediumDistance);
            return "ok";
        }
        else if (cmd == "masterdistance=far")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.farDistance);
            return "ok";
        }
        else if (cmd == "masterdistance=extreme")
        {
            Player* bot = sObjectMgr->GetPlayerByLowGUID(guid);
            bot->SetMinMasterDistance(sPlayerbotAIConfig.extremeDistance);
            return "ok";
        }
    }

    return "unknown command";
}

bool PlayerbotMgr::HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args)
{
	if (!sPlayerbotAIConfig.enabled)
	{
		handler->PSendSysMessage("|cffff0000Playerbot system is currently disabled!");
		handler->SetSentErrorMessage(true);
        return false;
	}

    WorldSession *m_session = handler->GetSession();

    if (!m_session)
    {
        handler->PSendSysMessage("You may only add bots from an active session");
        handler->SetSentErrorMessage(true);
        return false;
    }

    Player* player = m_session->GetPlayer();
    PlayerbotMgr* mgr = player->GetPlayerbotMgr();
    if (!mgr)
    {
        handler->PSendSysMessage("you cannot control bots yet");
        handler->SetSentErrorMessage(true);
        return false;
    }

    list<string> messages = mgr->HandlePlayerbotCommand(args, player);
    if (messages.empty())
        return true;

    for (list<string>::iterator i = messages.begin(); i != messages.end(); ++i)
    {
        handler->PSendSysMessage(i->c_str());
    }

    handler->SetSentErrorMessage(true);
    return false;
}

list<string> PlayerbotHolder::HandlePlayerbotCommand(char const* args, Player* master)
{
    list<string> messages;


	if (!*args)
	{
		messages.push_back("usage: list or add/init/remove PLAYERNAME");
		messages.push_back("  (OR) .bot lookup [CLASS] (without to see list of classes)");
		return messages;
	}

	char *cmd = strtok((char*)args, " ");
	char *charname = strtok(NULL, " ");

	//thesawolf - display lookup legend
	if ((cmd) && (!charname))
	{
		std::string cmdStr = cmd;
		if (cmdStr == "lookup" || cmdStr == "LOOKUP")
		{
			messages.push_back("Classes Available:");
			messages.push_back("|TInterface\\icons\\INV_Sword_27.png:25:25:0:-1|t Warrior");
			messages.push_back("|TInterface\\icons\\INV_Hammer_01.png:25:25:0:-1|t Paladin");
			messages.push_back("|TInterface\\icons\\INV_Weapon_Bow_07.png:25:25:0:-1|t Hunter");
			messages.push_back("|TInterface\\icons\\INV_ThrowingKnife_04.png:25:25:0:-1|t Rogue");
			messages.push_back("|TInterface\\icons\\INV_Staff_30.png:25:25:0:-1|t Priest");
			messages.push_back("|TInterface\\icons\\inv_jewelry_talisman_04.png:25:25:0:-1|t Shaman");
			messages.push_back("|TInterface\\icons\\INV_staff_30.png:25:25:0:-1|t Mage");
			messages.push_back("|TInterface\\icons\\INV_staff_30.png:25:25:0:-1|t Warlock");
			messages.push_back("|TInterface\\icons\\Ability_Druid_Maul.png:25:25:0:-1|t Druid");
			messages.push_back("(Usage: .bot lookup CLASS)");
			return messages;
		}
	}
	else if (!cmd)
	{
		messages.push_back("usage: list or add/init/remove PLAYERNAME");
		return messages;
		
	}
	
		if (!strcmp(cmd, "list"))
		{
			messages.push_back(ListBots(master));
			return messages;
		}
	
		if (!charname)
		{
			messages.push_back("usage: list or add/init/remove PLAYERNAME");
		return messages;
	}

    std::string cmdStr = cmd;
    std::string charnameStr = charname;


	//thesawolf - lookup routine.. you know ANY of those RANDOM names?
	if (cmdStr == "lookup" || cmdStr == "LOOKUP")
	{
		string bsearch1 = "Looking for bots of class: " + charnameStr + "...";
		messages.push_back(bsearch1);

		uint8 claz = 0;
		string icon = " ";
		if (charnameStr == "warrior" || charnameStr == "Warrior" || charnameStr == "WARRIOR")
		{
			claz = 1;
			icon = "|TInterface\\icons\\INV_Sword_27.png:25:25:0:-1|t ";
		}
		else if (charnameStr == "paladin" || charnameStr == "Paladin" || charnameStr == "PALADIN")
		{
			claz = 2;
			icon = "|TInterface\\icons\\INV_Hammer_01.png:25:25:0:-1|t ";
		}
		else if (charnameStr == "hunter" || charnameStr == "Hunter" || charnameStr == "HUNTER")
		{
			claz = 3;
			icon = "|TInterface\\icons\\INV_Weapon_Bow_07.png:25:25:0:-1|t ";
		}
		else if (charnameStr == "rogue" || charnameStr == "Rogue" || charnameStr == "ROGUE" || charnameStr == "rouge" || charnameStr == "Rouge" || charnameStr == "ROUGE") // for my friends that cannot spell
		{
			claz = 4;
			icon = "|TInterface\\icons\\INV_ThrowingKnife_04.png:25:25:0:-1|t ";
		}
		else if (charnameStr == "priest" || charnameStr == "Priest" || charnameStr == "PRIEST")
		{
			claz = 5;
			icon = "|TInterface\\icons\\INV_Staff_30.png:25:25:0:-1|t ";
		}
		else if (charnameStr == "shaman" || charnameStr == "Shaman" || charnameStr == "SHAMAN")
		{
			claz = 7;
			icon = "|TInterface\\icons\\inv_jewelry_talisman_04.png:25:25:0:-1|t ";
		}
		else if (charnameStr == "mage" || charnameStr == "Mage" || charnameStr == "MAGE")
		{
			claz = 8;
			icon = "|TInterface\\icons\\INV_staff_30.png:25:25:0:-1|t ";
		}
		else if (charnameStr == "warlock" || charnameStr == "Warlock" || charnameStr == "WARLOCK")
		{
			claz = 9;
			icon = "|TInterface\\icons\\INV_staff_30.png:25:25:0:-1|t ";
		}
		else if (charnameStr == "druid" || charnameStr == "Druid" || charnameStr == "DRUID")
		{
			claz = 11;
			icon = "|TInterface\\icons\\Ability_Druid_Maul.png:25:25:0:-1|t ";
		}
		else
		{
			messages.push_back("Error: Invalid Class. Try again.");
			return messages;
		}

		QueryResult lresults = CharacterDatabase.PQuery("SELECT * FROM characters WHERE class = '%u'", claz);
		if (lresults)
		{
			do
			{
				Field* fields = lresults->Fetch();
				string bName = fields[2].GetString();
				uint8 bRace = fields[3].GetUInt8();
				string cRace = " ";
				switch (bRace)
				{
				case 1: cRace = "Human";	break;
				case 2: cRace = "Orc";		break;
				case 3: cRace = "Dwarf";	break;
				case 4: cRace = "Nightelf";	break;
				case 5: cRace = "Undead";	break;
				case 6: cRace = "Tauren";	break;
				case 7: cRace = "Gnome";	break;
				case 8: cRace = "Troll";	break;
				case 10: cRace = "Bloodelf";	break;
				case 11: cRace = "Draenei";	break;
				}
				bool bGender = fields[5].GetBool();
				string cGender = "";
				if (bGender == 0)
					cGender = "Male";
				else
					cGender = "Female";
				bool bOnline = fields[25].GetBool();
				string cOnline = "";
				if (bOnline == 0)
					cOnline = "|cff00ff00Available|r";
				else
					cOnline = "|cffff0000Not Available|r";
				string bList = icon + "|TInterface\\icons\\Achievement_Character_" + cRace + "_" + cGender + ".png:25:25:0:-1|t " + bName + " - " + cRace + " " + cGender + " [" + cOnline + "]";
				messages.push_back(bList);

			} while (lresults->NextRow());
		}
		else
		{
			messages.push_back("Error: Listing class bots. Try again.");
			messages.push_back("Usage: .bot lookup (to see list of classes)");
			return messages;
		}
		messages.push_back("(Usage: .bot add PLAYERNAME)");
		return messages;
	}

    set<string> bots;
    if (charnameStr == "*" && master)
    {
        Group* group = master->GetGroup();
        if (!group)
        {
            messages.push_back("you must be in group");
            return messages;
        }

        Group::MemberSlotList slots = group->GetMemberSlots();
        for (Group::member_citerator i = slots.begin(); i != slots.end(); i++)
        {
			ObjectGuid member = i->guid;

			if (member == master->GetGUID())
				continue;

			string bot;
			if (sObjectMgr->GetPlayerNameByGUID(member, bot))
			    bots.insert(bot);
        }
    }

    if (charnameStr == "!" && master && master->GetSession()->GetSecurity() > SEC_GAMEMASTER)
    {
        for (PlayerBotMap::const_iterator i = GetPlayerBotsBegin(); i != GetPlayerBotsEnd(); ++i)
        {
            Player* bot = i->second;
            if (bot && bot->IsInWorld())
                bots.insert(bot->GetName());
        }
    }

    vector<string> chars = split(charnameStr, ',');
    for (vector<string>::iterator i = chars.begin(); i != chars.end(); i++)
    {
        string s = *i;

        uint32 accountId = GetAccountId(s);
        if (!accountId)
        {
            bots.insert(s);
            continue;
        }

        QueryResult results = CharacterDatabase.PQuery(
            "SELECT name FROM characters WHERE account = '%u'",
            accountId);
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                string charName = fields[0].GetString();
                bots.insert(charName);
            } while (results->NextRow());
        }
	}

    for (set<string>::iterator i = bots.begin(); i != bots.end(); ++i)
    {
        string bot = *i;
        ostringstream out;
        out << cmdStr << ": " << bot << " - ";

        ObjectGuid member = sObjectMgr->GetPlayerGUIDByName(bot);
        if (!member)
        {
            out << "character not found";
        }
        else if (master && member != master->GetGUID())
        {
            out << ProcessBotCommand(cmdStr, member,
                    master->GetSession()->GetSecurity() >= SEC_GAMEMASTER,
                    master->GetSession()->GetAccountId(),
                    master->GetGuildId());
        }
        else if (!master)
        {
            out << ProcessBotCommand(cmdStr, member, true, -1, -1);
        }

        messages.push_back(out.str());
    }

    return messages;
}

uint32 PlayerbotHolder::GetAccountId(string name)
{
    uint32 accountId = 0;

    QueryResult results = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '%s'", name.c_str());
    if(results)
    {
        Field* fields = results->Fetch();
        accountId = fields[0].GetUInt32();
    }

    return accountId;
}

string PlayerbotHolder::ListBots(Player* master)
{
	set<string> bots;
	map<uint8, string> classNames;
	classNames[CLASS_DRUID] = "Druid";
	classNames[CLASS_HUNTER] = "Hunter";
	classNames[CLASS_MAGE] = "Mage";
	classNames[CLASS_PALADIN] = "Paladin";
	classNames[CLASS_PRIEST] = "Priest";
	classNames[CLASS_ROGUE] = "Rogue";
	classNames[CLASS_SHAMAN] = "Shaman";
	classNames[CLASS_WARLOCK] = "Warlock";
	classNames[CLASS_WARRIOR] = "Warrior";
	ostringstream out;
	bool first = true;
	out << "Bot roster: ";
	for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
	{
		Player* const bot = it->second;
		string name = bot->GetName();
		bots.insert(name);

		if (first) first = false; else out << ", ";
		out << "+" << name << " " << classNames[bot->getClass()];
	}

	if (master)
	{
		QueryResult results = CharacterDatabase.PQuery("SELECT class,name FROM tc_characters_19.characters where account = '%u'",
			master->GetSession()->GetAccountId());
		if (results != NULL)
		{
			do
			{
				Field* fields = results->Fetch();
				uint8 cls = fields[0].GetUInt8();
				string name = fields[1].GetString();
				if (bots.find(name) == bots.end() && name != master->GetSession()->GetPlayerName())
				{
					if (first) first = false; else out << ", ";
					out << "-" << name << " " << classNames[cls];
				}
			} while (results->NextRow());
		}
	}

	return out.str();
}

PlayerbotMgr::PlayerbotMgr(Player* const master) : PlayerbotHolder(),  master(master)
{
}

PlayerbotMgr::~PlayerbotMgr()
{
}

void PlayerbotMgr::UpdateAIInternal(uint32 elapsed)
{
    SetNextCheckDelay(sPlayerbotAIConfig.reactDelay);
}

void PlayerbotMgr::HandleCommand(uint32 type, const string& text)
{
    Player *master = GetMaster();
    if (!master)
        return;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->GetPlayerbotAI()->HandleCommand(type, text, *master);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot->GetPlayerbotAI()->GetMaster() == master)
            bot->GetPlayerbotAI()->HandleCommand(type, text, *master);
    }
}

void PlayerbotMgr::HandleMasterIncomingPacket(const WorldPacket& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->GetPlayerbotAI()->HandleMasterIncomingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->GetPlayerbotAI()->HandleMasterIncomingPacket(packet);
    }

    switch (packet.GetOpcode())
    {
        // if master is logging out, log out all bots
        case CMSG_LOGOUT_REQUEST:
        {
            LogoutAllBots();
            return;
        }
    }
}
void PlayerbotMgr::HandleMasterOutgoingPacket(const WorldPacket& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->GetPlayerbotAI()->HandleMasterOutgoingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->GetPlayerbotAI()->HandleMasterOutgoingPacket(packet);
    }
}

void PlayerbotMgr::SaveToDB()
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->SaveToDB();
    }
    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->SaveToDB();
    }
}

void PlayerbotMgr::OnBotLoginInternal(Player * const bot)
{
    bot->GetPlayerbotAI()->SetMaster(master);
    bot->GetPlayerbotAI()->ResetStrategies();
}

