/*
 * Copyright 2009, 2010, The Pink Petal Development Team.
 * The Pink Petal Devloment Team are defined as the game's coders
 * who meet on http://pinkpetal.co.cc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "InterfaceProcesses.h"
#include "Brothel.hpp"
#include "cGangs.h"
#include "GangManager.hpp"
#include "InterfaceGlobals.h"
#include "BrothelManager.hpp"
#include "Helper.hpp"
#include "cGetStringScreenManager.h"
#include "cTariff.h"
#include "cScriptManager.h"
#include "cInterfaceWindow.h"
#include "cWindowManager.h"
#include "InterfaceIDs.h"
#include "cInterfaceEvent.h"
#include "cMessageBox.h"
#include "cChoiceMessage.h"
#include "DirPath.h"
#include "FileList.h"
#include "MasterFile.h"
#include "GameFlags.h"
#include "cTraits.h"
#include "cCustomers.h"
#include "cInventory.h"
#include "XmlMisc.h"
#include "cTriggers.h"
#include "cGirls.h"
#include "GirlManager.hpp"
#include "CLog.h"
#include "cRng.h"
#include "Girl.hpp"

#include <SDL.h>

namespace WhoreMasterRenewal
{

//used to store what files we have loaded
MasterFile loadedGirlsFiles;

void LoadGameScreen()
{
	DirPath location = DirPath() << "Saves";
	const char *pattern = "*.gam";
	FileList fl(location, pattern);

	if(g_InitWin)
	{
		g_LoadGame.Focused();
/*
 *		clear the list box with the save games
 */
		g_LoadGame.ClearListBox(g_interfaceid.LIST_LOADGSAVES);
/*
 *		loop through the files, adding them to the box
 */
		for(int i = 0; i < fl.size(); i++) {
			g_LoadGame.AddToListBox(g_interfaceid.LIST_LOADGSAVES, i, fl[i].leaf());
		}
		g_InitWin = false;
	}

/*
 *	no events process means we can go home early
 */
	if(g_InterfaceEvents.GetNumEvents() == 0) {
		return;
	}

/*
 *	the next simplest case is the "back" button
 */
	if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_LOADGBACK))
	{
		g_InitWin = true;
		g_WinManager.Pop();
		return;
	}
/*
 *	by this point, we're only interested if it's a click on the load game button or a double-click on a game in the list
 */
	if( !g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_LOADGLOAD)
		&& !g_LoadGame.ListDoubleClicked(g_interfaceid.LIST_LOADGSAVES) )
	{
		return;
	}
/*
 *	OK: So from this point onwards, we're loading the game
 */
	int selection = g_LoadGame.GetLastSelectedItemFromList(g_interfaceid.LIST_LOADGSAVES);
/*
 *	nothing selected means nothing more to do
 */
	if(selection == -1)
	{
		return;
	}
    std::string temp = fl[selection].leaf();
/*
 *	enable cheat mode for a test brothel
 */
	g_Cheats = (temp == "Test.gam");

	if(LoadGame(location, fl[selection].leaf()))
	{
		g_WinManager.Pop();
		g_WinManager.Push(BrothelScreen, &g_BrothelManagement);
		g_InitWin = true;
	}
	else
	{
		g_InitWin = true;
		g_WinManager.Pop();
	}
}

void MainMenu()
{
	if(g_InitWin)
	{
		g_MainMenu.Focused();
		g_InitWin = false;
		g_Girls.GetImgManager()->LoadList("Default");
	}

	if(g_InterfaceEvents.GetNumEvents() != 0)
	{
		if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_NEWGAME))
		{
			cGetStringScreenManager gssm;
			gssm.empty_allowed(false);
			gssm.set_handler(NewGame);
			g_WinManager.Push(GetString, &g_GetString);
			g_MessageQue.AddToQue("Enter a name for your first brothel.", 0);
			g_InitWin = true;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_LOADGAME))
		{
			g_WinManager.Push(LoadGameScreen, &g_LoadGame);
			g_InitWin = true;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_EXITGAME))
		{
			SDL_Event evn;
			evn.type = SDL_QUIT;
//			SDL_PushEvent(&evn);
		}
	}
}



void GetString()
{
	cGetStringScreenManager gssm;
	gssm.process();
}

void GetInt()
{
	if(g_InitWin)
	{
		g_GetInt.Focused();
		g_InitWin = false;
		g_IntReturn = 0;
		g_EnterKey = false;
	}

	if(g_InterfaceEvents.GetNumEvents() != 0 || g_EnterKey)
	{
		if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_OK) || g_EnterKey)
		{
			g_EnterKey = false;
			int number;
			g_ReturnText = g_GetInt.GetEditBoxText(g_interfaceid.EDITBOX_NAME);
			number = atoi(g_ReturnText.c_str());
			g_IntReturn = number;
			g_InitWin = true;
			g_WinManager.Pop();
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_CANCEL))
			g_WinManager.Pop();
	}
}

void NewGame()
{
	cConfig cfg;
	cScriptManager sm;

	g_Cheats = false;
	g_GenGirls = g_Cheats = g_WalkAround = false;	// for keeping track of weather have walked around town today
	g_TalkCount = 10;
	g_Brothels.Free();
	g_Gangs.Free();
	g_Customers.Free();
	g_Girls.Free();
	g_Traits.Free();
	g_GlobalTriggers.Free();
	g_Customers.Free();
	sm.Release();
	g_InvManager.Free();

	if(g_ReturnText == "Test")
		g_Cheats = true;
	else
		g_Cheats = false;

	// Load all the data
	LoadGameInfoFiles();
	loadedGirlsFiles.LoadXML(TiXmlHandle(nullptr));
	LoadGirlsFiles();

	g_Girls.LoadDefaultImages();	// load the default girl images

	// load the global triggers
	g_GlobalTriggers.LoadList(
		DirPath() << "Resources" << "Scripts" << "GlobalTriggers.xml"
	);

	g_CurrBrothel = 0;

	g_Gold.reset();

	g_Year = 1209;
	g_Month = 1;
	g_Day = 1;

	selected_girl = nullptr;
	for(int i=0; i<8; i++)
	{
		MarketSlaveGirls[i] = nullptr;
		MarketSlaveGirlsDel[i] = -1;
	}

	g_Brothels.NewBrothel(20);
	g_Brothels.SetName(0, g_ReturnText);
	for(int i=0; i<NUM_STATS; i++)
		g_Brothels.GetPlayer()->m_Stats[i] = 60;
	for(u_int i=0; i<NUM_SKILLS; i++)
		g_Brothels.GetPlayer()->m_Skills[i] = 10;
	g_Brothels.GetPlayer()->SetToZero();
	//g_Brothels.GetPlayer()->m_CustomerFear = 0;
	//g_Brothels.GetPlayer()->m_Disposition = 0;
	//g_Brothels.GetPlayer()->m_Suspicion = 0;
	//g_Brothels.GetPlayer()->m_WinGame = false;

	u_int start_random_gangs = cfg.gangs.start_random();
	u_int start_boosted_gangs = cfg.gangs.start_boosted();
	for(u_int i=0; i<start_random_gangs; i++)
		g_Gangs.AddNewGang(false);
	for(u_int i=0; i<start_boosted_gangs; i++)
		g_Gangs.AddNewGang(true);

	// update the shop inventory
	g_InvManager.UpdateShop();

/*
 *	two strings speparated only by white space are concatenated
 *	by the compiler. Which means you can split up long text
 *	passages, making them easier to read:
 */
#if 0

	g_MessageQue.AddToQue(
		"Welcome to Crossgate, a city in the realm of Mundiga, "
		"where criminals rule and space and time overlap with "
		"other worlds and dimensions. Once a powerful crime lord "
		"in the city, your father was recently assassinated and "
		"his assets looted by rivals. All that remains is the "
		"fire-gutted shell of an old brothel that served as your "
		"father's headquarters.\n"

		"However this building hides some interesting secrets. "
		"Still concealed in the cellars is the dungeon where "
		"your father conducted the less savory aspects of "
		"his business. Underneath, endless catacombs extend, "
		"constantly shifting in the dimensional flux, drawing "
		"in beings and plunder from a thousand different worlds.\n"

		"Your job now is to return the brothel to its former "
		"glories of exotic women and carnal pleasures. That will "
		"give you the income you need to avenge your father, "
		"and resume your rightful place as his successor.\n"

		"It is up to you if you will be as evil as your father "
		"or not, but in any case you start with very little gold "
		"and your first order of business should be to purchase "
		"some girls from the slave market, and to hire some goons "
		"to guard your headquarters.",
		0
	);
#endif
	// Add the begining rivals
	g_Brothels.GetRivalManager()->CreateRival(200, 5, 10000, 2, 0, 26, 2, 2);
	g_Brothels.GetRivalManager()->CreateRival(400, 10, 15000, 2, 1, 30, 2, 3);
	g_Brothels.GetRivalManager()->CreateRival(600, 15, 20000, 3, 1, 56, 3, 5);
	g_Brothels.GetRivalManager()->CreateRival(800, 20, 25000, 4, 2, 74, 4, 8);

	if(g_Cheats)
	{
		g_Gold.cheat();
		g_InvManager.GivePlayerAllItems();
		//g_Brothels.AddGirl(0, g_Girls.GetGirl(23));  // Adding girl to brothel (Ayanami Rei as it happens) for some reason?
	}

	g_WinManager.Push(BrothelScreen, &g_BrothelManagement);

	DirPath text = DirPath()
		<< "Saves"
		<< (g_Brothels.GetBrothel(0)->m_Name + ".gam").c_str()
	;
	sm.Load(ScriptPath("Intro.lua"), nullptr);
	SaveGameXML(text);
}

void BrothelScreen()
{
	if(g_InitWin)
	{
	    std::string brothel = "Current Brothel: ";
		brothel = g_Brothels.GetName(g_CurrBrothel);
		g_BrothelManagement.EditTextItem(brothel, g_interfaceid.TEXT_CURRENTBROTHEL);
		g_BrothelManagement.Focused();

		std::stringstream ss;
		ss << "Day: " << g_Day << " Month: " << g_Month << " Year: " << g_Year << ", Brothel: " << brothel;
		g_BrothelManagement.EditTextItem(ss.str(), g_interfaceid.TEXT_BROTHELNAME);
		g_BrothelManagement.EditTextItem(
			g_Brothels.GetBrothelString(g_CurrBrothel),
			g_interfaceid.TEXT_BMDETAILS
		);
		g_InitWin = false;
		selected_girl = nullptr;
	}

	if(g_InterfaceEvents.GetNumEvents() != 0)
	{
		if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_MANAGEGIRLS))
		{
			g_InitWin = true;
			g_WinManager.push("Girl Management");
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_MANAGESTAFF))
		{
			g_InitWin = true;
			g_WinManager.push("Gangs");
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_MANAGEBUILDING))
		{
			g_InitWin = true;
			g_WinManager.push("Building Setup");
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_VISITDUNGEON))
		{
			g_InitWin = true;
			g_WinManager.push("Dungeon");
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_VISITTOWN))
		{
			g_InitWin = true;
			g_WinManager.push("Town");
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_SAVEGAME))
		{
			g_MessageQue.AddToQue("Game Saved", 2);
			SaveGameXML(DirPath()
				<< "Saves"
				<< (g_Brothels.GetBrothel(0)->m_Name + ".gam").c_str()
			);
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_NEWWEEK))
		{
			g_InitWin = true;
			SaveGameXML(DirPath()
				<< "Saves"
				<< "autosave.gam"
			);
			NextWeek();
			g_WinManager.Push(Turnsummary, &g_Turnsummary);
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TURNSUMMARY))
		{
			g_InitWin = true;
			g_WinManager.Push(Turnsummary, &g_Turnsummary);
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_QUIT))
		{
			g_WinManager.Pop();
			g_InitWin = true;
			ResetInterface();
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_NEXTBROTHEL))
		{
			g_CurrBrothel++;
			if(g_CurrBrothel >= g_Brothels.GetNumBrothels())
				g_CurrBrothel=0;
			g_InitWin = true;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_PREVBROTHEL))
		{
			g_CurrBrothel--;
			if(g_CurrBrothel < 0)
				g_CurrBrothel =  g_Brothels.GetNumBrothels()-1;
			g_InitWin = true;
			return;
		}
	}

	if(g_LeftArrow)
	{
		g_CurrBrothel--;
		if(g_CurrBrothel < 0)
			g_CurrBrothel =  g_Brothels.GetNumBrothels()-1;
		g_InitWin = true;
		g_LeftArrow = false;
		return;
	}
	else if(g_RightArrow)
	{
		g_CurrBrothel++;
		if(g_CurrBrothel >= g_Brothels.GetNumBrothels())
			g_CurrBrothel=0;
		g_InitWin = true;
		g_RightArrow = false;
		return;
	}

	g_BrothelManagement.SetImage(g_interfaceid.IMAGE_BIMAGE, g_BrothelImages[g_CurrBrothel]);
}

static std::string clobber_extension(std::string s)
{
    size_t pos = s.rfind(".");
	g_LogFile.ss() << "clobber_extension: s = " << s << std::endl
                << "clobber_extension: pos = " << pos << std::endl;
    std::string base = s.substr(0, pos);
	g_LogFile.ss() << "clobber_extension: s = " << s << std::endl
                << "clobber_extension: base = " << base << std::endl;
	g_LogFile.ssend();
	return base;
}

/*
 * interim loader to load XML files, and then non-xml ones
 * if there was no xml version.
 */
static void load_items_temp(FileList &fl)
{
	std::map<std::string,std::string> lookup;

	g_LogFile.ss() << "itemsx files:" << std::endl;
	g_LogFile.ssend();
	fl.scan("*.itemsx");
	for(int i = 0; i < fl.size(); i++) {
	    std::string str = fl[i].full();
	    std::string key = clobber_extension(str);
		lookup[key] = str;
		g_LogFile.ss() << "	adding " << str << std::endl
                << "	under " << key << std::endl
                << "	result " << lookup[key] << std::endl;
		g_LogFile.ssend();
	}

	g_LogFile.ss() << "items files:" << std::endl;
	g_LogFile.ssend();
	fl.scan("*.items");
	for(int i = 0; i < fl.size(); i++) {
	    std::string str = fl[i].full();
	    std::string key = clobber_extension(str);
		g_LogFile.ss() << "	checking " << lookup[key] << std::endl;
		g_LogFile.ssend();
		if(lookup[key] != "") {
			continue;
		}
		lookup[key] = str;
		g_LogFile.ss() << "	adding " << str << std::endl
                    << "	under " << key << std::endl;
        g_LogFile.ssend();
	}
/*
 *	Iterate over the map and print out all key/value pairs.
 *	kudos: wikipedia
 */
	g_LogFile.ss() << "walking map..." << std::endl;
	g_LogFile.ssend();
	for(std::map<std::string,std::string>::const_iterator it = lookup.begin(); it != lookup.end(); ++it) {
	    std::string full_path = it->second;
		g_LogFile.ss() <<	"\tkey = " << it->first << std::endl
                    <<	"\tpath = " << full_path << std::endl;
        g_LogFile.ssend();
/*
 *		does it end in ".items" or ".itemsx"?
 */
		size_t len = full_path.length();
		char c = full_path.at(len-1);
		if(c == 'x') {
			g_LogFile.ss() << "\t\tloading xml" << std::endl;
			g_LogFile.ssend();
			g_InvManager.LoadItemsXML(full_path);
		}
		else {
			g_LogFile.ss() << "\t\tloading orig" << std::endl;
			g_LogFile.ssend();
			g_InvManager.LoadItems(full_path);
		}
	}
}

void LoadGameInfoFiles()
{
/*
 *	Load the traits: first build the path
 */
	DirPath location = DirPath() << "Resources" << "Data";
/*
 *	get a file list
 */
	FileList fl(location, "*.traits");
/*
 *	loop over the list, loading the files
 */
	for(int i = 0; i < fl.size(); i++) {
		g_Traits.LoadTraits(fl[i].full());

	}
/*
 *	scan and load items
 *	The "temp" function was a stopgap until
 *	I figured out XMLFileList. Needs replacing.
 */
	load_items_temp(fl);
}

void LoadGirlsFiles()
{
/*
 *	now get a list of all the file in the Characters folder
 *	start by building a path...
 */
	DirPath location = DirPath() << "Resources" << "Characters";
/*
 *	now scan for matching files. The XMLFileList
 *	will look for ".girls" and ".girlx" files
 *	with the XML versions shadowing the originals
 */
	XMLFileList girlfiles(location, "*.girls");
	XMLFileList rgirlfiles(location,"*.rgirls");
/*
 *	And we need to know which ".girls" files the saved game processed
 *	This information is stored in the master file - so we read that.
 */
	for(int i = 0; i < girlfiles.size(); i++)
	{
/*
 *		OK: if the current file is listed in the master file
 *		we don't need to load it. Unless the AllData flag is set
 *		and then we do. I think.
 */
		if(loadedGirlsFiles.exists(girlfiles[i].leaf()))
		{
			continue;
		}
/*
 *		add the file to the master list
 */
		loadedGirlsFiles.add(girlfiles[i].leaf());
/*
 *		load the file
 */
		g_Girls.LoadGirlsDecider(girlfiles[i].full());
	}
/*
 *	Load random girls
 *
 *	"girlfiles" is still an XMLFileList, so this will get
 *	XML format files in preference to original format ones
 */
	for(int i = 0; i < rgirlfiles.size(); i++)
	{
		g_Girls.LoadRandomGirl(rgirlfiles[i].full());
	}
}

void ChangeGirlJobs()
{
	Girl *girl = selected_girl;

	static int selection = -1;
	if(girl == nullptr && g_AllTogle == false)
	{
		selection = -1;
		g_WinManager.Pop();
		g_InitWin = true;
		g_LogFile.write("ERROR - change girl job, girl is null");
	}

	if(g_InitWin)
	{
		g_ChangeJobs.Focused();

		selection = -1;
		// clear the lists
		g_ChangeJobs.ClearListBox(g_interfaceid.LIST_CJDAYTIME);
		g_ChangeJobs.ClearListBox(g_interfaceid.LIST_CJNIGHTTIME);

		// Fill the list boxes and set the current selections
	    std::string text = "Freetime (";
		text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 0, true));
		text += ")";
		g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJDAYTIME, 0, text);
		text = "Freetime (";
		text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 0, false));
		text += ")";
		g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJNIGHTTIME, 0, text);

		text = "Work Brothel (";
		text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 3, true));
		text += ")";
		g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJDAYTIME, 3, text);
		text = "Work Brothel (";
		text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 3, false));
		text += ")";
		g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJNIGHTTIME, 3, text);

		text = "Work Streets (";
		text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 2, true));
		text += ")";
		g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJDAYTIME, 2, text);
		text = "Work Streets (";
		text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 2, false));
		text += ")";
		g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJNIGHTTIME, 2, text);

		text = "Produce Movie (";
		text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 5, true));
		text += ")";
		g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJDAYTIME, 5, text);
		text = "Produce Movie (";
		text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 5, false));
		text += ")";
		g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJNIGHTTIME, 5, text);

		if(g_Brothels.GetBrothel(g_CurrBrothel)->m_Bar == 1)
		{
			text = "Strip Bar (";
			text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 4, true));
			text += ")";
			g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJDAYTIME, 4, text);
			text = "Strip Bar (";
			text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 4, false));
			text += ")";
			g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJNIGHTTIME, 4, text);
		}

		if(g_Brothels.GetBrothel(g_CurrBrothel)->m_GamblingHall == 1)
		{
			text = "Gambling Hall (";
			text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 8, true));
			text += ")";
			g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJDAYTIME, 8, text);
			text = "Gambling Hall (";
			text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 8, false));
			text += ")";
			g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJNIGHTTIME, 8, text);
		}

		text = "Training (";
		text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 9, true));
		text += ")";
		g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJDAYTIME, 9, text);
		text = "Training (";
		text += toString(g_Brothels.GetNumGirlsOnJob(g_CurrBrothel, 9, false));
		text += ")";
		g_ChangeJobs.AddToListBox(g_interfaceid.LIST_CJNIGHTTIME, 9, text);

		if(girl)
		{
			g_ChangeJobs.SetSelectedItemInList(g_interfaceid.LIST_CJDAYTIME, static_cast<int>(girl->m_DayJob) );
			g_ChangeJobs.SetSelectedItemInList(g_interfaceid.LIST_CJNIGHTTIME, static_cast<int>(girl->m_NightJob) );
		}

		g_InitWin = false;
	}

	if(g_InterfaceEvents.GetNumEvents() != 0)
	{
		if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_CJOK))
		{
			if(!g_AllTogle && girl != nullptr)
			{
				selection = g_ChangeJobs.GetSelectedItemFromList(g_interfaceid.LIST_CJDAYTIME);
				if(selection != -1)
					girl->m_DayJob = selection;

				if(selection == 5)
				{
					g_LogFile.ss() << "Error: Movies should not be available...";
					g_LogFile.ssend();
				}

				selection = g_ChangeJobs.GetSelectedItemFromList(g_interfaceid.LIST_CJNIGHTTIME);
				if(selection != -1)
					girl->m_NightJob = selection;

				if(selection == 5)
				{
					g_LogFile.ss() << "Error: Movies should not be available...";
					g_LogFile.ssend();
				}
			}
			else
			{
				g_AllTogle = false;
				for(int i=0; i<g_Brothels.GetNumGirls(g_CurrBrothel); i++)
				{
					Girl* selGirl = g_Brothels.GetGirl(g_CurrBrothel, i);
					if(g_Girls.GetStat(selGirl, STAT_HEALTH) == 0)
						continue;

					selection = g_ChangeJobs.GetSelectedItemFromList(g_interfaceid.LIST_CJDAYTIME);
					if(selection != -1)
						selGirl->m_DayJob = selection;

					if(selection == 5)
					{
						g_LogFile.ss() << "Error: Movies should not be available...";
                        g_LogFile.ssend();
					}

					selection = g_ChangeJobs.GetSelectedItemFromList(g_interfaceid.LIST_CJNIGHTTIME);
					if(selection != -1)
						selGirl->m_NightJob = selection;

					if(selection == 5)
					{
						g_LogFile.ss() << "Error: Movies should not be available...";
                        g_LogFile.ssend();
					}
				}
			}

			g_InitWin = true;
			g_WinManager.Pop();
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_CJCANCEL))
		{
			g_InitWin = true;
			g_WinManager.Pop();
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_CJDAYTIME))
		{
			selection = g_ChangeJobs.GetSelectedItemFromList(g_interfaceid.LIST_CJDAYTIME);
			if(selection != -1)
			{
				if(selection == 0)
					g_ChangeJobs.EditTextItem("This makes a girl relax and recover and maybe do a little shopping.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 3)
					g_ChangeJobs.EditTextItem("She will work in the brothel.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 2)
					g_ChangeJobs.EditTextItem("She will work on the streets surrounding and for a short distance from the brothel. It can be dangerous.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 5)
					g_ChangeJobs.EditTextItem("Error: You shouldn't see this, since Movies were removed from game.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 4)
					g_ChangeJobs.EditTextItem("Work in your strip bar. Bar requires 3 girls or to have hired bar staff to run. More girls adds customers.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 8)
					g_ChangeJobs.EditTextItem("Work in the gambling hall. Gambling hall requires 5 girls or to have hired bar staff to run. More girls add customers.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 9)
					g_ChangeJobs.EditTextItem("Train skills for a shift, the more girls that training together the better they will train. Highly experienced girls will give extra boost.", g_interfaceid.TEXT_CJDESC);
			}
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_CJNIGHTTIME))
		{
			selection = g_ChangeJobs.GetSelectedItemFromList(g_interfaceid.LIST_CJNIGHTTIME);
			if(selection != -1)
			{
				if(selection == 0)
					g_ChangeJobs.EditTextItem("This makes a girl relax and recover and maybe do a little shopping.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 3)
					g_ChangeJobs.EditTextItem("She will work in the brothel.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 2)
					g_ChangeJobs.EditTextItem("She will work on the streets surrounding and for a short distance from the brothel. It can be dangerous.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 5)
					g_ChangeJobs.EditTextItem("Error: You shouldn't see this, since Movies were removed from game.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 4)
					g_ChangeJobs.EditTextItem("Work in your strip bar. Bar requires 3 girls or to have hired bar staff to run. More girls adds customers.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 8)
					g_ChangeJobs.EditTextItem("Work in the gambling hall. Gambling hall requires 5 girls or to have hired bar staff to run. More girls add customers.", g_interfaceid.TEXT_CJDESC);
				else if(selection == 9)
					g_ChangeJobs.EditTextItem("Train skills for a shift, the more girls that training together the better they will train. Highly experienced girls will give extra boost.", g_interfaceid.TEXT_CJDESC);
			}
		}
	}
}

// MYR: Reordered the listing of messages: Critical (red) first, then important (dark blue)
//      then everything else.
// WD:	Copy sort code to Dungeon Girls

void Turnsummary()

{
	static int ImageType = -1;
	static int lastNum = -1;
	static int ImageNum = -1;
	static int LastType = -1;
	static int category = 0;
	static int category_last = 0;
	static int Item = 0;

	Girl *girl;

	if(g_InitWin)
	{
		g_Turnsummary.Focused();

	    std::string brothel = "Current Brothel: ";
		brothel += g_Brothels.GetName(g_CurrBrothel);
		g_Turnsummary.EditTextItem(brothel, g_interfaceid.TEXT_CURRENTBROTHEL);

		if(category_last == category)
			Item = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM);
		else
		{
			Item = 0;
			category_last = category;
		}

		// Clear the lists
		g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSCATEGORY);
		g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSITEM);
		g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);

/*
 *		CATEGORY Listbox
 */
		// list the categories of events
		g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 0, "GIRLS");
		g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 1, "GANGS");
		g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 2, "BROTHELS");
		g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 3, "DUNGEON");
		//g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 4, "RIVALS");
		//g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 5, "GLOBAL"); */
		g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSCATEGORY, category, false);

/*
 *		ITEM Listbox
 */
		// 0. List girls
		if(category == 0)
		{
			int nNumGirls	= g_Brothels.GetNumGirls(g_CurrBrothel);
			int ID			= 0;

			// MYR: Girls with danger events first
			for(int i=0; i<nNumGirls; i++)
			{
				Girl* pTmpGirl = g_Brothels.GetGirl(g_CurrBrothel, i);
				if(pTmpGirl->m_Events.HasDanger())
				{
				    std::string tname = pTmpGirl->m_Realname;
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, ID, tname, LISTBOX_RED);
					if (selected_girl == pTmpGirl)
						Item = ID;
					ID++;
				}
			}

			// Girls with warning events next
			for(int j=0; j<nNumGirls; j++)
			{
				Girl* pTmpGirl = g_Brothels.GetGirl(g_CurrBrothel, j);
				if( pTmpGirl->m_Events.HasWarning() && !pTmpGirl->m_Events.HasDanger())
				{
				    std::string temp = pTmpGirl->m_Realname;
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, ID, temp, LISTBOX_DARKBLUE);
					if (selected_girl == pTmpGirl)
						Item = ID;
					ID++;
				}
			}

			// Then every other girl
			for(int k=0; k<nNumGirls; k++)
			{
				Girl* pTmpGirl = g_Brothels.GetGirl(g_CurrBrothel, k);
				if(!pTmpGirl->m_Events.HasUrgent())
				{
				    std::string temp = pTmpGirl->m_Realname;
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, ID, temp);
					if (selected_girl == pTmpGirl)
						Item = ID;
					ID++;
				}
			}
		}

		// 1. Gangs
		else if(category == 1)
		{
			for(int i=0; i<g_Gangs.GetNumGangs(); i++)
				g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, g_Gangs.GetGang(i)->m_Name);
		}

		// 2. Brothels
		else if(category == 2)
		{
			for(int i=0; i<g_Brothels.GetNumBrothels(); i++)
				g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, g_Brothels.GetBrothel(i)->m_Name);
		}

		// 3. Dungeon
		else if(category == 3)
		{

			// Fill the list box
			cDungeon* pDungeon	= g_Brothels.GetDungeon();
			int nNumGirls		= pDungeon->GetNumGirls();
#if 0
			for(int i = 0; i < nNumGirls; i++)	// add girls
			{
				int col = LISTBOX_BLUE;
				Girl *girl = pDungeon->GetGirl(i)->m_Girl;
				if (selected_girl == girl)
					Item = i;
				if(girl->health() <= 30) {
					col = LISTBOX_RED;
				}
				g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, girl->m_Realname, col);
			}
#else
			int ID	= 0;

			// WD:	Copied sort to dungeon girls.  Added warnings to cDungeon::Update() to sort
			// MYR: Girls with danger events first
			for(int i=0; i<nNumGirls; i++)
			{
				Girl* pTmpGirl = pDungeon->GetGirl(i)->m_Girl;
				if(pTmpGirl->m_Events.HasDanger())
				{
				    std::string tname = pTmpGirl->m_Realname;
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, ID, tname, LISTBOX_RED);
					if (selected_girl == pTmpGirl)
						Item = ID;
					ID++;
				}
			}

			// Girls with warning events next
			for(int j=0; j<nNumGirls; j++)
			{
				Girl* pTmpGirl = pDungeon->GetGirl(j)->m_Girl;
				if( pTmpGirl->m_Events.HasWarning() && !pTmpGirl->m_Events.HasDanger())
				{
				    std::string temp = pTmpGirl->m_Realname;
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, ID, temp, LISTBOX_DARKBLUE);
					if (selected_girl == pTmpGirl)
						Item = ID;
					ID++;
				}
			}

			// Then every other girl
			for(int k=0; k<nNumGirls; k++)
			{
				Girl* pTmpGirl = pDungeon->GetGirl(k)->m_Girl;
				if(!pTmpGirl->m_Events.HasUrgent())
				{
				    std::string temp = pTmpGirl->m_Realname;
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, ID, temp);
					if (selected_girl == pTmpGirl)
						Item = ID;
					ID++;
				}
			}
#endif
		} // End of dungeon

        if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSITEM) > 0)
		  g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSITEM, 0, true);

		if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSITEM) >= Item)
			g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSITEM, Item, true);
		else if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSITEM) > 0)
		{
			Item = 0;
			g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSITEM, Item, true);
		}

		// Set the brothels name
		g_Turnsummary.EditTextItem("", g_interfaceid.TEXT_TSEVENTDESC);

		g_InitWin = false;
		g_Turnsummary.DisableButton(g_interfaceid.BUTTON_TSGOTO, true);
	}  // End of if initwin

/*
 *		EVENTS Listbox
 */

	if(g_InterfaceEvents.GetNumEvents() != 0)
	{
		// Change category
		if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_TSCATEGORY))
		{
			if((category = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSCATEGORY)) == -1)	// if a girl is selected then
				category = 0;
			else
				Item = 0;
			g_InitWin = true;
		}

		// Change item
		else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_TSITEM))
		{
			int selected = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM);
			if(selected == -1)
				g_Turnsummary.DisableButton(g_interfaceid.BUTTON_TSGOTO, true);
			else
				g_Turnsummary.DisableButton(g_interfaceid.BUTTON_TSGOTO, false);


			g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);
			g_Turnsummary.EditTextItem("", g_interfaceid.TEXT_TSEVENTDESC);
			// ------------ Girl Event display
			if(category == 0)
			{
				if(selected != -1)
				{
				    std::string selectedName = g_Turnsummary.GetSelectedTextFromList(g_interfaceid.LIST_TSITEM);
					girl = selected_girl = g_Brothels.GetGirlByName(g_CurrBrothel, selectedName);

					if(girl == nullptr)
						return;

					// change the picture to profile (no event selected)
					ImageType = IMGTYPE_PROFILE;

#if 1
					if (!girl->m_Events.IsEmpty())
					{
						girl->m_Events.DoSort();						// Sort Events to put Warnings & Dangers first.

						for(int l=0; l<girl->m_Events.GetNumEvents(); l++)
						{
						    std::string 		sTitle			= girl->m_Events.GetMessage(l).TitleText();
							unsigned int	uiListboxColour	= girl->m_Events.GetMessage(l).ListboxColour();
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, l, sTitle, uiListboxColour);
						}
					}
#else
					// Important texts at the top
				    std::string text = "";
					bool selected = false, FoundFirstMessage = false; // Priority is given to selecting danger over warning over standard
					int ID2 = 0, FirstMessage = -1;

					// DANGER first
                    for(int l=0; l<girl->m_Events.GetNumEvents(); l++)
					{
						//if(girl->m_Events.GetMessage(l).m_Event == EVENT_DANGER)
						{
							text = "!!!DANGER!!!";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, l, text, LISTBOX_RED);
							ID2++;
							if (!FoundFirstMessage)
							{
							  FirstMessage = l;
							  FoundFirstMessage = true;
							}
						}
					}

					// Warnings
                    for(int m=0; m<girl->m_Events.GetNumEvents(); m++)
					{
						if(girl->m_Events.GetMessage(m).m_Event == EVENT_WARNING)
						{
							text = "Warning";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, m, text, LISTBOX_DARKBLUE);
							ID2++;
                            if (!FoundFirstMessage)
							{
							  FirstMessage = m;
							  FoundFirstMessage = true;
							}
						}
					}

					// Everything else
					for(int p=0; p<girl->m_Events.GetNumEvents(); p++)
					{
						if(girl->m_Events.GetMessage(p).m_Event == EVENT_DAYSHIFT)
						{
							text = "Day Shift";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, p, text);
							ID2++;
							if (!FoundFirstMessage)
							{
							  FirstMessage = p;
							  FoundFirstMessage = true;
							}
						}
						else if(girl->m_Events.GetMessage(p).m_Event == EVENT_NIGHTSHIFT)
						{
							text = "Night Shift";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, p, text);
							ID2++;
							if (!FoundFirstMessage)
							{
							  FirstMessage = p;
							  FoundFirstMessage = true;
							}
						}
						else if(girl->m_Events.GetMessage(p).m_Event == EVENT_SUMMARY)
						{
							text = "summary";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, p, text);
							ID2++;
							if (!FoundFirstMessage)
							{
							  FirstMessage = p;
							  FoundFirstMessage = true;
							}
						}
					}

#endif
					if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSEVENTS) > 0)
						g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSEVENTS, 0, true);
				}
			}

			// ------------ Gangs item display
			else if(category == 1)
			{
				if(selected != -1)
				{
					// Get the gang
					sGang* gang = g_Gangs.GetGang(selected);

					if(gang == nullptr)
						return;

#if 1
					if (!gang->m_Events.IsEmpty())
					{
						gang->m_Events.DoSort();						// Sort Events to put Warnings & Dangers first.

						for(int l=0; l<gang->m_Events.GetNumEvents(); l++)
						{
						    std::string 		sTitle			= gang->m_Events.GetMessage(l).TitleText();
							unsigned int	uiListboxColour	= gang->m_Events.GetMessage(l).ListboxColour();
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, l, sTitle, uiListboxColour);
						}
					}
#else
					// add the newly selected gangs events
					for(int i=0; i<gang->m_Events.GetNumEvents(); i++)
					{
					    std::string text = "Event";
						g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
					}
#endif
					if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSEVENTS) > 0)
						g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSEVENTS, 0, true);
				}
			}

			// ------------- Brothels item display
			else if(category == 2)
			{
				if(selected != -1)
				{
#if 1
					Brothel	*pSelectedBrothel = g_Brothels.GetBrothel(selected);
					if (!pSelectedBrothel->m_Events.IsEmpty())
					{
						pSelectedBrothel->m_Events.DoSort();						// Sort Events to put Warnings & Dangers first.

						for(int l=0; l<g_Brothels.GetBrothel(selected)->m_Events.GetNumEvents(); l++)
						{
						    std::string 		sTitle			= pSelectedBrothel->m_Events.GetMessage(l).TitleText();
							unsigned int	uiListboxColour	= pSelectedBrothel->m_Events.GetMessage(l).ListboxColour();
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, l, sTitle, uiListboxColour);
						}
					}

#else
					// add the events
					for(int i=0; i<g_Brothels.GetBrothel(selected)->m_Events.GetNumEvents(); i++)
					{
					    std::string text = "Event";
						g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
					}
#endif
					if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSEVENTS) > 0)
						g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSEVENTS, 0, true);
				}
			}

			// -------- Dungeons item display
			else if (category == 3) {
				if(selected != -1)
				{
#if 1
					// list the events
					// Get the girl
				    std::string selectedName		= g_Turnsummary.GetSelectedTextFromList(g_interfaceid.LIST_TSITEM);
					girl =	selected_girl	= g_Brothels.GetDungeon()->GetGirlByName(selectedName)->m_Girl;

					if(girl == nullptr)
						return;

					// change the picture to profile (no event selected)
					ImageType = IMGTYPE_PROFILE;

					if (!girl->m_Events.IsEmpty())
					{
						girl->m_Events.DoSort();						// Sort Events to put Warnings & Dangers first.

						for(int l=0; l<girl->m_Events.GetNumEvents(); l++)
						{
						    std::string 		sTitle			= girl->m_Events.GetMessage(l).TitleText();
							unsigned int	uiListboxColour	= girl->m_Events.GetMessage(l).ListboxColour();
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, l, sTitle, uiListboxColour);
						}
					}

#else
					g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);

					// list the events
					// Get the girl
					selected_girl = girl = g_Brothels.GetDungeon()->GetGirl(selected)->m_Girl;

					if(girl == 0)
						return;

					// change the picture to profile (no event selected)
					ImageType = IMGTYPE_PROFILE;

					// add the newly selected girls events
				    std::string text = "";

					for(int i=0; i<girl->m_Events.GetNumEvents(); i++)
					{
						if(girl->m_Events.GetMessage(i).m_Event == EVENT_DANGER)
						{
							text = "!!!DANGER!!!";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_RED);
						}
					}

					for(int i=0; i<girl->m_Events.GetNumEvents(); i++)
					{
						if(girl->m_Events.GetMessage(i).m_Event == EVENT_WARNING)
						{
							text = "Warning";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_DARKBLUE);
						}
					}

					for(int i=0; i<girl->m_Events.GetNumEvents(); i++)
					{
						if(girl->m_Events.GetMessage(i).m_Event == EVENT_DAYSHIFT)
						{
							text = "Day Shift";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_NIGHTSHIFT)
						{
							text = "Night Shift";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_SUMMARY)
						{
							text = "summary";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
					}
#endif
					if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSEVENTS) > 0)
						g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSEVENTS, 0, true);
				}
			}
		}

		// Girl event selection
		else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_TSEVENTS))
		{
			if(category == 0)
			{
				int SelEvent = -1;
				// ****************************
				// When warnings and problems are displayed first, the ordering of events is messed up
				// A warning is at ID 0 in the list, and is ID X in the message queue
				// ****************************
				//
				//	WD: The message queue is now sorted in cEvents::DoSort()
				//		so ID's will match
				// ****************************
				if((SelEvent = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSEVENTS)) != -1)
				{
					int SelGirl; // TODO check
					// if a girl is selected then
					if((SelGirl = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM)) != -1)
					{
						Girl* girl = nullptr;

						// MYR
					    std::string selectedName	= g_Turnsummary.GetSelectedTextFromList(g_interfaceid.LIST_TSITEM);
						girl = selected_girl = g_Brothels.GetGirlByName(g_CurrBrothel, selectedName);

						// Set the event desc text
						g_Turnsummary.EditTextItem(girl->m_Events.GetMessage(SelEvent).m_Message, g_interfaceid.TEXT_TSEVENTDESC);

						// Change the picture
						ImageType = girl->m_Events.GetMessage(SelEvent).m_MessageType;

						lastNum = -1;
					}
				}
			}

			// Gang event selection
			else if(category == 1)
			{
				int SelEvent = -1;
				// if a event is selected then
				if((SelEvent = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSEVENTS)) != -1)
				{
					int SelGang;
					// if a gang is selected then
					if((SelGang = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM)) != -1)
					{
						sGang* gang = g_Gangs.GetGang(SelGang);

						// Set the event desc text
						g_Turnsummary.EditTextItem(gang->m_Events.GetMessage(SelEvent).m_Message, g_interfaceid.TEXT_TSEVENTDESC);

						// Change the picture
						ImageType = gang->m_Events.GetMessage(SelEvent).m_MessageType;

						lastNum = -1;
					}
				}
			}

			// Brothel event selection
			else if(category == 2)
			{
				int SelEvent = -1;
				if((SelEvent = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSEVENTS)) != -1)	// if a event is selected then
				{
					int SelBrothel;
					if((SelBrothel = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM)) != -1)		// if a brothel is selected then
					{
						// Set the event desc text
						g_Turnsummary.EditTextItem(g_Brothels.GetBrothel(SelBrothel)->m_Events.GetMessage(SelEvent).m_Message, g_interfaceid.TEXT_TSEVENTDESC);

						// Change the picture
						ImageType = g_Brothels.GetBrothel(SelBrothel)->m_Events.GetMessage(SelEvent).m_MessageType;

						lastNum = -1;
					}
				}
			}

			// Dungeon event selection
			else if (category == 3)
			{
				int SelEvent = -1;
				if((SelEvent = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSEVENTS)) != -1)	// if a event is selected then
				{
					int SelGirl; // TODO check
					if((SelGirl = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM)) != -1)	// if a dungeon is selected then
					{
						// WD	Get girl by name
					    std::string selectedName	= g_Turnsummary.GetSelectedTextFromList(g_interfaceid.LIST_TSITEM);
						Girl* girl			= g_Brothels.GetDungeon()->GetGirlByName(selectedName)->m_Girl;

						if(girl == nullptr)
							return;

						// Set the event desc text
						g_Turnsummary.EditTextItem(girl->m_Events.GetMessage(SelEvent).m_Message, g_interfaceid.TEXT_TSEVENTDESC);

						// Change the picture
						ImageType = girl->m_Events.GetMessage(SelEvent).m_MessageType;

						lastNum = -1;
					}
				}
			}
		}

		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TSCLOSE))
		{
			g_WinManager.Pop();
			g_InitWin = true;
			return;
		}

		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TSNEWWEEK))
		{
			g_InitWin = true;
			SaveGameXML( DirPath() << "Saves" << "autosave.gam" );
			NextWeek();
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TSNEXTBROTHEL))
		{
			g_CurrBrothel++;
			if(g_CurrBrothel >= g_Brothels.GetNumBrothels())
				g_CurrBrothel=0;
			g_InitWin = true;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TSPREVBROTHEL))
		{
			g_CurrBrothel--;
			if(g_CurrBrothel < 0)
				g_CurrBrothel =  g_Brothels.GetNumBrothels()-1;
			g_InitWin = true;
			return;
		}
		/*
		 *	GOTO Sleclected Girl or Gang
		 */
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TSGOTO))
		{
			int selected = 0;
			selected = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM);
			if(selected != -1)
			{
				if(category == 0)
				{
					// MYR
				    std::string selectedName		= g_Turnsummary.GetSelectedTextFromList(g_interfaceid.LIST_TSITEM);
					girl	= selected_girl	= g_Brothels.GetGirlByName(g_CurrBrothel, selectedName);
					g_WinManager.push("Girl Details");
				}
				else if(category == 1)
				{
					g_WinManager.push("Gangs");
				}
				else if (category == 3)		// Dungeon
				{
					// WD: Add Dungeon Girls Goto
				    std::string selectedName		= g_Turnsummary.GetSelectedTextFromList(g_interfaceid.LIST_TSITEM);
					selected_girl	= girl	= g_Brothels.GetDungeon()->GetGirlByName(selectedName)->m_Girl;
					g_WinManager.push("Girl Details");

				}
				g_InitWin = true;
			}
			return;
		}
	}

	if(g_UpArrow)
	{
		Item = g_Turnsummary.ArrowUpListBox(g_interfaceid.LIST_TSITEM);
		g_UpArrow = false;
		selected_girl = nullptr;
		g_InitWin = true;
		return;
	}
	else if(g_DownArrow)
	{
		Item = g_Turnsummary.ArrowDownListBox(g_interfaceid.LIST_TSITEM);
		g_DownArrow = false;
		selected_girl = nullptr;
		g_InitWin = true;
		return;
	}

	if(g_LeftArrow)
	{
		g_CurrBrothel--;
		if(g_CurrBrothel < 0)
			g_CurrBrothel =  g_Brothels.GetNumBrothels()-1;
		g_InitWin = true;
		g_LeftArrow = false;
		category = Item = 0;
		return;
	}
	else if(g_RightArrow)
	{
		g_CurrBrothel++;
		if(g_CurrBrothel >= g_Brothels.GetNumBrothels())
			g_CurrBrothel=0;
		g_InitWin = true;
		g_RightArrow = false;
		category = Item = 0;
		return;
	}
/*
 *	Draw a girls picture and description when selected
 *	Category 1 is easier, so let's get that out of the way first
 */
	if(category == 1) {
		g_Turnsummary.SetImage(g_interfaceid.IMAGE_TSIMAGE, g_BrothelImages[g_CurrBrothel]);
		return;
	}
/*
 *	make sure we don't trip over any other categories that were being
 *	silently ignored
 */
	if(category != 0 && category != 3) {
		return;
	}
/*
 *	anythign hereafter is category zero - so we can lose the "if"
 */
	int GirlNum = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM);
/*
 *	again, no selected item means "nothing to do"
 *	so let's do that first :)
 */
	if(GirlNum == -1) {
		return;
	}

/*
 *	both cases need the selected girl data - so let's get that
 */
	Girl* selGirl = nullptr;
	if (category == 0)
	{
		// MYR
	    std::string selectedName = g_Turnsummary.GetSelectedTextFromList(g_interfaceid.LIST_TSITEM);
		selGirl = selected_girl = g_Brothels.GetGirlByName(g_CurrBrothel, selectedName);
		//selGirl = g_Brothels.GetGirl(g_CurrBrothel, num);
	}
	else if (category == 3)
	{
		cDungeon* pDungeon = g_Brothels.GetDungeon();
		if (pDungeon->GetNumGirls() > 0)
		{
			sDungeonGirl *selDGirl = pDungeon->GetGirl(GirlNum);
			if(selDGirl != nullptr)
				selGirl = selDGirl->m_Girl;
			else
				selGirl = nullptr;
		}
	}
/*
 *	not really sure what's going on here. lastNum is static so
 *	persists from one invocation to the next. Ditto lastType.
 *
 *	num is the selection in the list box
 *
 *	so we're saying "if this is a different girl, or if we didn't
 *	display a girl last time - do this:"
 *
 *	let's set a flag with that
 */
	bool image_changed = (lastNum != GirlNum || LastType != ImageType);
/*
 *	now: the setImage happens regardless of whether the image changed or not
 *	assuming selGirl is set, anyhow. The only difference is that the
 *	random parameter passed is true if the "image_changed" is true:
 *
 *	that makes sense - this is a new girl, so go find a random image of her
 */
	if(selGirl)
	{
		g_Turnsummary.SetImage(g_interfaceid.IMAGE_TSIMAGE, g_Girls.GetImageSurface(selGirl, ImageType, image_changed, ImageNum));
		if(g_Girls.IsAnimatedSurface(selGirl, ImageType, ImageNum))
			g_Turnsummary.SetImage(g_interfaceid.IMAGE_TSIMAGE, g_Girls.GetAnimatedSurface(selGirl, ImageType, ImageNum));
	}
/*
 *	now we need to update the "last" variables
 *
 *	this is cheap enough we can do it whether there's been a change
 *	or not
 */
	LastType = ImageType;
	lastNum = GirlNum;
}
#if 0
{
	static int ImageType = -1;
	static int lastNum = -1;
	static int ImageNum = -1;
	static int LastType = -1;
	static int category = 0;
	static int Item = 0;

	Girl *girl;

	if(g_InitWin)
	{
		g_Turnsummary.Focused();

	    std::string brothel = "Current Brothel: ";
		brothel += g_Brothels.GetName(g_CurrBrothel);
		g_Turnsummary.EditTextItem(brothel, g_interfaceid.TEXT_CURRENTBROTHEL);

		Item = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM);

		// Clear the lists
		g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSCATEGORY);
		g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);
		g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSITEM);

		// list the categories of events
		g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 0, "GIRLS");
		g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 1, "GANGS");
		g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 2, "BROTHELS");
		g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 3, "DUNGEON");
//		g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 4, "RIVALS");
//		g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSCATEGORY, 5, "GLOBAL");
		g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSCATEGORY, category, false);

		// List the Items in the category
		if(category == 0)
		{
			for(int i=0; i<g_Brothels.GetNumGirls(g_CurrBrothel); i++)
			{
				Girl* b_girl = g_Brothels.GetGirl(g_CurrBrothel, i);
				if (selected_girl == b_girl)
					Item = i;

			    std::string temp = b_girl->m_Realname;

				if(b_girl->health() <= 10 || b_girl->m_Events.HasDanger())
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, temp, LISTBOX_RED);
				else if(b_girl->health() <= 40 || b_girl->m_Events.HasWarning())
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, temp, LISTBOX_DARKBLUE);
				else
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, temp);
			}

		}
		else if(category == 1)
		{
			for(int i=0; i<g_Gangs.GetNumGangs(); i++)
			{
				if(g_Gangs.GetGang(i)->m_Num <= 4 || g_Gangs.GetGang(i)->m_Events.HasDanger())
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, g_Gangs.GetGang(i)->m_Name, LISTBOX_RED);
				else if(g_Gangs.GetGang(i)->m_Num <= 8 || g_Gangs.GetGang(i)->m_Events.HasWarning())
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, g_Gangs.GetGang(i)->m_Name, LISTBOX_DARKBLUE);
				else
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, g_Gangs.GetGang(i)->m_Name);
			}
		}

		else if(category == 2)
		{
			for(int i=0; i<g_Brothels.GetNumBrothels(); i++)
			{
				if(g_Brothels.GetBrothel(i)->m_Events.HasDanger())
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, g_Brothels.GetBrothel(i)->m_Name, LISTBOX_RED);
				else if(g_Brothels.GetBrothel(i)->m_Events.HasWarning())
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, g_Brothels.GetBrothel(i)->m_Name, LISTBOX_DARKBLUE);
				else

				g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, g_Brothels.GetBrothel(i)->m_Name);
			}
		}
		else if(category == 3)
		{
			cDungeon* pDungeon = g_Brothels.GetDungeon();
			// Fill the list box
			for(int i = 0; i < pDungeon->GetNumGirls(); i++)	// add girls
			{
				//int col = LISTBOX_BLUE;
				//Girl *girl = pDungeon->GetGirl(i)->m_Girl;
				//if (selected_girl == girl)
				//	Item = i;
				//if(girl->health() <= 30) {
				//	col = LISTBOX_RED;
				//}
				//g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, girl->m_Realname, col);

				Girl* d_girl = pDungeon->GetGirl(i)->m_Girl;
				if (selected_girl == d_girl)
					Item = i;

			    std::string temp = d_girl->m_Realname;

				if(d_girl->health() <= 10 || d_girl->m_Events.HasDanger())
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, temp, LISTBOX_RED);
				else if(d_girl->health() <= 40 || d_girl->m_Events.HasWarning())
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, temp, LISTBOX_DARKBLUE);
				else
					g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSITEM, i, temp);
			}
		}

		if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSITEM) >= Item)
			g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSITEM, Item, true);
		else if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSITEM) > 0)
		{
			Item = 0;
			g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSITEM, Item, true);
		}

		// Set the brothels name
		g_Turnsummary.EditTextItem("", g_interfaceid.TEXT_TSEVENTDESC);

		g_InitWin = false;
		g_Turnsummary.DisableButton(g_interfaceid.BUTTON_TSGOTO, true);
	}

	if(g_InterfaceEvents.GetNumEvents() != 0)
	{
		if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_TSCATEGORY))
		{
			if((category = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSCATEGORY)) == -1)	// if a girl is selected then
				category = 0;
			else
				Item = 0;
			g_InitWin = true;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_TSITEM))
		{
			int selection = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM);
			if(selection == -1)
				g_Turnsummary.DisableButton(g_interfaceid.BUTTON_TSGOTO, true);
			else
				g_Turnsummary.DisableButton(g_interfaceid.BUTTON_TSGOTO, false);

			if(category == 0)
			{
				if(selection != -1)
				{
					g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);
					// list the events
					// Get the girl
					selected_girl = girl = g_Brothels.GetGirl(g_CurrBrothel, selection);

					if(girl == 0)
						return;

					// change the picture to profile (no event selected)
					ImageType = IMGTYPE_PROFILE;

					// add the newly selected girls events
					for(int i=0; i<girl->m_Events.GetNumEvents(); i++)
					{
					    std::string text = "";
						if(girl->m_Events.GetMessage(i).m_Event == EVENT_DAYSHIFT)
						{
							text = "Day Shift";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_NIGHTSHIFT)
						{
							text = "Night Shift";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_WARNING)
						{
							text = "Warning";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_DARKBLUE);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_DANGER)
						{
							text = "!!!DANGER!!!";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_RED);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_SUMMARY)
						{
							text = "Summary";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_DUNGEON)
						{
							text = "Dungeon";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_MATRON)
						{
							text = "Matron";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_DEBUG)
						{
							text = "Debug";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_DARKBLUE);
						}
						else	// catch any unknown event
						{
							text = "Event";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
					}

					if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSEVENTS) > 0)
						g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSEVENTS, 0, true);
				}
				else
				{
					g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);
					g_Turnsummary.EditTextItem("", g_interfaceid.TEXT_TSEVENTDESC);
				}
			}
			else if(category == 1)
			{
				if(selection != -1)
				{
					g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);
					// Get the girl
					sGang* gang = g_Gangs.GetGang(selection);

					if(gang == 0)
						return;

					// add the newly selected girls events
					for(int i=0; i<gang->m_Events.GetNumEvents(); i++)
					{
					    std::string text = "";
						if(gang->m_Events.GetMessage(i).m_Event == EVENT_WARNING)
						{
							text = "Warning";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_DARKBLUE);
						}
						else if(gang->m_Events.GetMessage(i).m_Event == EVENT_DEBUG)
						{
							text = "!!!DANGER!!!";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_RED);
						}
						else if(gang->m_Events.GetMessage(i).m_Event == EVENT_DEBUG)
						{
							text = "Debug";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_DARKBLUE);
						}
						else
						{
							text = "Event";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
					}

					if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSEVENTS) > 0)
						g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSEVENTS, 0, true);
				}
				else
				{
					g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);
					g_Turnsummary.EditTextItem("", g_interfaceid.TEXT_TSEVENTDESC);
				}
			}
			else if(category == 2)
			{
				if(selection != -1)
				{
					g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);

					// add the events
					for(int i=0; i<g_Brothels.GetBrothel(selection)->m_Events.GetNumEvents(); i++)
					{
					    std::string text = "";
						if(g_Brothels.GetBrothel(selection)->m_Events.GetMessage(i).m_Event == EVENT_DEBUG)
						{
							text = "Debug";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_DARKBLUE);
						}
						if(g_Brothels.GetBrothel(selection)->m_Events.GetMessage(i).m_Event == EVENT_WARNING)
						{
							text = "Warning";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_DARKBLUE);
						}
						else if(g_Brothels.GetBrothel(selection)->m_Events.GetMessage(i).m_Event == EVENT_DEBUG)
						{
							text = "!!!DANGER!!!";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_RED);
						}
						else
						{
							text = "Event";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
					}

					if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSEVENTS) > 0)
						g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSEVENTS, 0, true);
				}
				else
				{
					g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);
					g_Turnsummary.EditTextItem("", g_interfaceid.TEXT_TSEVENTDESC);
				}
			}
			else if (category == 3) {
				if(selection != -1)
				{
					g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);
					// list the events
					// Get the girl
					cDungeon* pDungeon = g_Brothels.GetDungeon();
					selected_girl = girl = pDungeon->GetGirl(selection)->m_Girl;

					if(girl == 0)
						return;

					// change the picture to profile (no event selected)
					ImageType = IMGTYPE_PROFILE;

					// add the newly selected girls events
					for(int i=0; i<girl->m_Events.GetNumEvents(); i++)
					{
					    std::string text = "";
						if(girl->m_Events.GetMessage(i).m_Event == EVENT_DAYSHIFT)
						{
							text = "Day Shift";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_NIGHTSHIFT)
						{
							text = "Night Shift";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_WARNING)
						{
							text = "Warning";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_DARKBLUE);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_DANGER)
						{
							text = "!!!DANGER!!!";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_RED);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_SUMMARY)
						{
							text = "Summary";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_DUNGEON)
						{
							text = "Dungeon";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_MATRON)
						{
							text = "Matron";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
						else if(girl->m_Events.GetMessage(i).m_Event == EVENT_DEBUG)
						{
							text = "Debug";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text, LISTBOX_DARKBLUE);
						}
						else	// catch any unknown event
						{
							text = "Event";
							g_Turnsummary.AddToListBox(g_interfaceid.LIST_TSEVENTS, i, text);
						}
					}

					if(g_Turnsummary.GetListBoxSize(g_interfaceid.LIST_TSEVENTS) > 0)
						g_Turnsummary.SetSelectedItemInList(g_interfaceid.LIST_TSEVENTS, 0, true);
				}
				else
				{
					g_Turnsummary.ClearListBox(g_interfaceid.LIST_TSEVENTS);
					g_Turnsummary.EditTextItem("", g_interfaceid.TEXT_TSEVENTDESC);
				}
			}
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_TSEVENTS))
		{
			if(category == 0)
			{
				int ret1 = -1;
				if((ret1 = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSEVENTS)) != -1)	// if a event is selected then
				{
					int ret2;
					if((ret2 = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM)) != -1)	// if a girl is selected then
					{
						Girl* girl = g_Brothels.GetGirl(g_CurrBrothel, ret2);

						// Set the event desc text
						g_Turnsummary.EditTextItem(girl->m_Events.GetMessage(ret1).m_Message, g_interfaceid.TEXT_TSEVENTDESC);

						// Change the picture
						ImageType = girl->m_Events.GetMessage(ret1).m_MessageType;

						lastNum = -1;
					}
				}
			}
			else if(category == 1)
			{
				int ret1 = -1;
				if((ret1 = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSEVENTS)) != -1)	// if a event is selected then
				{
					int ret2;
					if((ret2 = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM)) != -1)	// if a gang is selected then
					{
						sGang* gang = g_Gangs.GetGang(ret2);

						// Set the event desc text
						g_Turnsummary.EditTextItem(gang->m_Events.GetMessage(ret1).m_Message, g_interfaceid.TEXT_TSEVENTDESC);

						// Change the picture
						ImageType = gang->m_Events.GetMessage(ret1).m_MessageType;

						lastNum = -1;
					}
				}
			}
			else if(category == 2)
			{
				int ret1 = -1;
				if((ret1 = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSEVENTS)) != -1)	// if a event is selected then
				{
					int ret2;
					if((ret2 = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM)) != -1)	// if a gang is selected then
					{
						// Set the event desc text
						g_Turnsummary.EditTextItem(g_Brothels.GetBrothel(ret2)->m_Events.GetMessage(ret1).m_Message, g_interfaceid.TEXT_TSEVENTDESC);

						// Change the picture
						ImageType = g_Brothels.GetBrothel(ret2)->m_Events.GetMessage(ret1).m_MessageType;

						lastNum = -1;
					}
				}
			}
			else if (category == 3)
			{
				int ret1 = -1;
				if((ret1 = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSEVENTS)) != -1)	// if a event is selected then
				{
					int ret2;
					if((ret2 = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM)) != -1)	// if a girl is selected then
					{
						cDungeon* pDungeon = g_Brothels.GetDungeon();
						Girl* girl = pDungeon->GetGirl(ret2)->m_Girl;

						if(girl == 0)
							return;

						// Set the event desc text
						g_Turnsummary.EditTextItem(girl->m_Events.GetMessage(ret1).m_Message, g_interfaceid.TEXT_TSEVENTDESC);

						// Change the picture
						ImageType = girl->m_Events.GetMessage(ret1).m_MessageType;

						lastNum = -1;
					}
				}
			}
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TSCLOSE))
		{
			g_WinManager.Pop();
			g_InitWin = true;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TSNEWWEEK))
		{
			g_InitWin = true;
			SaveGame( DirPath() << "Saves" << "autosave.gam" );
			NextWeek();
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TSNEXTBROTHEL))
		{
			g_CurrBrothel++;
			if(g_CurrBrothel >= g_Brothels.GetNumBrothels())
				g_CurrBrothel=0;
			g_InitWin = true;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TSPREVBROTHEL))
		{
			g_CurrBrothel--;
			if(g_CurrBrothel < 0)
				g_CurrBrothel =  g_Brothels.GetNumBrothels()-1;
			g_InitWin = true;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TSGOTO))
		{
			int selection = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM);
			if(selection != -1)
			{
				if(category == 0)
				{
					girl = g_Brothels.GetGirl(g_CurrBrothel, selection);
					if(girl->health() <= 0)
						g_MessageQue.AddToQue("This girl is dead. Her body will be removed by the end of the week.", 1);
					else
					{
						//load up the cycle_girls vector with the ordered list of girl IDs
						g_Turnsummary.FillSortedIDList(g_interfaceid.LIST_TSITEM, &cycle_girls, &cycle_pos);
						for(int i = cycle_girls.size(); i --> 0; )
						{  // no viewing dead girls
							if(g_Brothels.GetGirl(g_CurrBrothel, cycle_girls[i])->health() <= 0)
								cycle_girls.erase(cycle_girls.begin()+i);
						}
						selected_girl = girl;
						g_WinManager.push("Girl Details");
					}
				}
				else if(category == 1)
				{
					g_WinManager.push("Gangs");
				}
				else if (category == 3)
				{
					girl = g_Brothels.GetDungeon()->GetGirl(selection)->m_Girl;
					if(girl->health() <= 0)
						g_MessageQue.AddToQue("This girl is dead. Her body will be removed by the end of the week.", 1);
					else
					{
						//load up the cycle_girls vector with the ordered list of girl IDs
						g_Turnsummary.FillSortedIDList(g_interfaceid.LIST_TSITEM, &cycle_girls, &cycle_pos);
						for(int i = cycle_girls.size(); i --> 0; )
						{  // no viewing dead girls
							if(g_Brothels.GetDungeon()->GetGirl(cycle_girls[i])->m_Girl->health() <= 0)
								cycle_girls.erase(cycle_girls.begin()+i);
						}
						selected_girl = girl;
						g_WinManager.push("Girl Details");
					}
				}
				g_InitWin = true;
			}
			return;
		}
	}

	if(g_UpArrow)
	{
		Item = g_Turnsummary.ArrowUpListBox(g_interfaceid.LIST_TSITEM);
		g_UpArrow = false;
		selected_girl = girl = 0;
		g_InitWin = true;
		return;
	}
	else if(g_DownArrow)
	{
		Item = g_Turnsummary.ArrowDownListBox(g_interfaceid.LIST_TSITEM);
		g_DownArrow = false;
		selected_girl = girl = 0;
		g_InitWin = true;
		return;
	}

	if(g_LeftArrow)
	{
		g_CurrBrothel--;
		if(g_CurrBrothel < 0)
			g_CurrBrothel =  g_Brothels.GetNumBrothels()-1;
		g_InitWin = true;
		g_LeftArrow = false;
		category = Item = 0;
		return;
	}
	else if(g_RightArrow)
	{
		g_CurrBrothel++;
		if(g_CurrBrothel >= g_Brothels.GetNumBrothels())
			g_CurrBrothel=0;
		g_InitWin = true;
		g_RightArrow = false;
		category = Item = 0;
		return;
	}
/*
 *	Draw a girls picture and description when selected
 *	Category 1 is easier, so let's get that out of the way first
 */
	if(category == 1) {
		g_Turnsummary.SetImage(g_interfaceid.IMAGE_TSIMAGE, g_BrothelImages[g_CurrBrothel]);
		return;
	}
/*
 *	make sure we don't trip over any other categories that were being
 *	silently ignored
 */
	if(category != 0 && category != 3) {
		return;
	}
/*
 *	anythign hereafter is category zero - so we can lose the "if"
 */
	int num = g_Turnsummary.GetSelectedItemFromList(g_interfaceid.LIST_TSITEM);
/*
 *	again, no selected item means "nothing to do"
 *	so let's do that first :)
 */
	if(num == -1) {
		return;
	}

/*
 *	both cases need the selected girl data - so let's get that
 */
	Girl* selGirl = NULL;
	if (category == 0)
	{
		selGirl = g_Brothels.GetGirl(g_CurrBrothel, num);
	}
	else if (category == 3)
	{
		cDungeon* pDungeon = g_Brothels.GetDungeon();
		if (pDungeon->GetNumGirls() > 0)
		{
			sDungeonGirl *selDGirl = pDungeon->GetGirl(num);
			if(selDGirl != 0)
				selGirl = selDGirl->m_Girl;
			else
				selGirl = 0;
		}
	}
/*
 *	not really sure what's going on here. lastNum is static so
 *	persists from one invocation to the next. Ditto lastType.
 *
 *	num is the selection in the list box
 *
 *	so we're saying "if this is a different girl, or if we didn't
 *	display a girl last time - do this:"
 *
 *	let's set a flag with that
 */
	bool image_changed = (lastNum != num || LastType != ImageType);
/*
 *	now: the setImage happens regardless of whether the image changed or not
 *	assuming selGirl is set, anyhow. The only difference is that the
 *	random parameter passed is true if the "image_changed" is true:
 *
 *	that makes sense - this is a new girl, so go find a random image of her
 */
	if(selGirl)
	{
		g_Turnsummary.SetImage(g_interfaceid.IMAGE_TSIMAGE, g_Girls.GetImageSurface(selGirl, ImageType, image_changed, ImageNum));
		if(g_Girls.IsAnimatedSurface(selGirl, ImageType, ImageNum))
			g_Turnsummary.SetImage(g_interfaceid.IMAGE_TSIMAGE, g_Girls.GetAnimatedSurface(selGirl, ImageType, ImageNum));
	}
/*
 *	now we need to update the "last" variables
 *
 *	this is cheap enough we can do it whether there's been a change
 *	or not
 */
	LastType = ImageType;
	lastNum = num;
}
#endif

void NextWeek()
{
	g_GenGirls = g_WalkAround = false;
	g_TalkCount = 10;

	if(g_Cheats)
	{
		g_Gold.cheat();
	}

	// Clear choice dialog
	g_ChoiceManager.Free();

	// update the shop inventory
	g_InvManager.UpdateShop();

	// Clear the interface events
	g_InterfaceEvents.ClearEvents();

	// go through and update all the brothels (this updates the girls each brothel has and calculates sex and stuff)
	g_Brothels.UpdateBrothels();

	// go ahead and handle pregnancies for girls not controlled by player
	g_Girls.UncontrolledPregnancies();

	// go through and update all the gang-related data (send them on missions, etc.)
	g_Gangs.UpdateGangs();

	// go through and update the population base
	g_Customers.ChangeCustomerBase();

	// Free customers
	g_Customers.Free();

	// Update the time
	g_Day+=7;
	if(g_Day > 30)
	{
		g_Day = g_Day-30;
		g_Month++;
		if(g_Month > 12)
		{
			g_Month = 1;
			g_Year++;
		}
	}

	// update the players gold
	g_Gold.week_end();

	// Process Triggers
	g_GlobalTriggers.ProcessTriggers();

	GameEvents();
}

void GameEvents()
{
	cScriptManager sm;
/*
 *	The "" on the end forces it to end in a / or \
 */
	DirPath dp = DirPath() << "Resources" << "Scripts" << "";

	if(!sm.IsActive())
		eventrunning = false;

	// process global triggers here
	if(g_GlobalTriggers.GetNextQueItem() && !eventrunning)
	{
		g_GlobalTriggers.ProcessNextQueItem( dp );
		eventrunning = true;
	}
	else if(!eventrunning)	// check girl scripts
	{
		if(g_Brothels.CheckScripts())
			eventrunning = true;
	}

	if(eventrunning)
	{
		sm.RunScript();
	}

	if(CheckGameFlag(FLAG_DUNGEONGIRLDIE))	// a girl has died int the dungeon
	{
		g_MessageQue.AddToQue("A girl has died in the dungeon.\nHer body will be removed by the end of the week.", 1);

		if(g_Dice%100 < 10)	// only 10% of being discovered
		{
			g_Brothels.GetPlayer()->suspicion(1);
			//g_Brothels.GetPlayer()->m_Suspicion++;
			//if(g_Brothels.GetPlayer()->m_Suspicion > 100)
			//	g_Brothels.GetPlayer()->m_Suspicion = 100;
		}

		g_Brothels.GetPlayer()->disposition(-1);
		//g_Brothels.GetPlayer()->m_Disposition--;
		//if(g_Brothels.GetPlayer()->m_Disposition < -100)
		//	g_Brothels.GetPlayer()->m_Disposition = -100;

		g_Brothels.UpdateAllGirlsStat(nullptr, STAT_PCFEAR, 2);

		ClearGameFlag(FLAG_DUNGEONGIRLDIE);
	}
	else if(CheckGameFlag(FLAG_DUNGEONCUSTDIE))	// a customer has died in the dungeon
	{
		g_MessageQue.AddToQue("A customer has died in the dungeon.\nTheir body will be removed by the end of the week.", 1);

		if(g_Dice%100 < 10)	// only 10% chance of being found out
		{
			// WD: Use acessor methods
			//g_Brothels.GetPlayer()->m_Suspicion++;
			//if(g_Brothels.GetPlayer()->m_Suspicion > 100)
			//	g_Brothels.GetPlayer()->m_Suspicion = 100;
			g_Brothels.GetPlayer()->suspicion(1);

		}

		// WD: Use acessor methods
		//g_Brothels.GetPlayer()->m_Disposition--;
		//if(g_Brothels.GetPlayer()->m_Disposition < -100)
		//	g_Brothels.GetPlayer()->m_Disposition = -100;
		g_Brothels.GetPlayer()->disposition(-1);

		// WD: Use acessor methods
		//g_Brothels.GetPlayer()->m_CustomerFear++;
		//if(g_Brothels.GetPlayer()->m_CustomerFear > 100)
		//	g_Brothels.GetPlayer()->m_CustomerFear = 100;
		g_Brothels.GetPlayer()->customerfear(1);

		ClearGameFlag(FLAG_DUNGEONCUSTDIE);
	}
}

void Gallery()
{
	static int Mode = IMGTYPE_ANAL;
	static int Img = 0;	// what image currently drawing
	Girl *girl = selected_girl;

	if(g_InitWin)
	{
		if(girl == nullptr)
		{
			g_InitWin = true;
			g_MessageQue.AddToQue("ERROR: No girl selected", 1);
			g_WinManager.Pop();
			return;
		}
		g_Gallery.Focused();

		if(girl->m_GirlImages->m_Images[IMGTYPE_ANAL].m_NumImages == 0)
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYANAL, true);
		else
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYANAL, false);
		if(girl->m_GirlImages->m_Images[IMGTYPE_BDSM].m_NumImages == 0)
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYBDSM, true);
		else
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYBDSM, false);
		if(girl->m_GirlImages->m_Images[IMGTYPE_SEX].m_NumImages == 0)
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYSEX, true);
		else
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYSEX, false);
		if(girl->m_GirlImages->m_Images[IMGTYPE_BEAST].m_NumImages == 0)
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYBEAST, true);
		else
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYBEAST, false);
		if(girl->m_GirlImages->m_Images[IMGTYPE_GROUP].m_NumImages == 0)
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYGROUP, true);
		else
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYGROUP, false);
		if(girl->m_GirlImages->m_Images[IMGTYPE_LESBIAN].m_NumImages == 0)
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYLESBIAN, true);
		else
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYLESBIAN, false);
		if(girl->m_GirlImages->m_Images[IMGTYPE_PREGNANT].m_NumImages == 0)
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYPREGNANT, true);
		else
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYPREGNANT, false);
		if(girl->m_GirlImages->m_Images[IMGTYPE_DEATH].m_NumImages == 0)
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYDEATH, true);
		else
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYDEATH, false);
		if(girl->m_GirlImages->m_Images[IMGTYPE_PROFILE].m_NumImages == 0)
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYPROFILE, true);
		else
			g_Gallery.DisableButton(g_interfaceid.BUTTON_GALLERYPROFILE, false);

		while(girl->m_GirlImages->m_Images[Mode].m_NumImages == 0 && Mode < NUM_IMGTYPES)
		{
			Mode++;
		}

		if(Img >= girl->m_GirlImages->m_Images[Mode].m_NumImages)
			Img = 0;
		else if(Img < 0)
			Img = girl->m_GirlImages->m_Images[Mode].m_NumImages-1;

		g_InitWin = false;
	}

	if(g_InterfaceEvents.GetNumEvents() != 0)
	{
		if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYBACK))
		{
			g_WinManager.Pop();
			g_InitWin = true;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYANAL))
		{
			Mode=0;
			Img=0;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYBDSM))
		{
			Mode=1;
			Img=0;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYSEX))
		{
			Mode=2;
			Img=0;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYBEAST))
		{
			Mode=3;
			Img=0;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYGROUP))
		{
			Mode=4;
			Img=0;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYLESBIAN))
		{
			Mode=5;
			Img=0;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYPREGNANT))
		{
			Mode=6;
			Img=0;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYDEATH))
		{
			Mode=7;
			Img=0;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYPROFILE))
		{
			Mode=8;
			Img=0;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYPREV))
		{
			Img--;
			if(Img < 0)
				Img = girl->m_GirlImages->m_Images[Mode].m_NumImages-1;
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_GALLERYNEXT))
		{
			Img++;
			if(Img == girl->m_GirlImages->m_Images[Mode].m_NumImages)
				Img = 0;
			return;
		}
	}

	if(g_LeftArrow)
	{
		g_LeftArrow = false;
		Img--;
		if(Img < 0)
			Img = girl->m_GirlImages->m_Images[Mode].m_NumImages-1;
		return;
	}
	else if(g_RightArrow)
	{
		g_RightArrow = false;
		Img++;
		if(Img == girl->m_GirlImages->m_Images[Mode].m_NumImages)
			Img = 0;
		return;
	}

	if (Mode == NUM_IMGTYPES)
	{
		//we've gone through all categories and could not find a single image!
		return;
	}

	// Draw the image
	if(girl)
	{
		g_Gallery.SetImage(g_interfaceid.IMAGE_TSIMAGE, g_Girls.GetImageSurface(girl, Mode, false, Img, true));
		if(g_Girls.IsAnimatedSurface(girl, Mode, Img))
			g_Gallery.SetImage(g_interfaceid.IMAGE_TSIMAGE, g_Girls.GetAnimatedSurface(girl, Mode, Img));
	}
}

/*
void SaveMasterFile(std::string filename)
{
 *
 *	I think this is outdated now. Which is to say
 *	we re-write the masterfile after we finish loading
 *	and I don't think we can pick up any new .girls files
 *	in mid play. So this should already be up to date.
 *
 *	I'm just commenting it out in case I'm wrong
 *	(Do we need this the first time we save?)
 *
    std::string savedFiles[400];
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	int numSaved = 0;

	// first load the masterlist to see what has been loaded before
	std::ifstream load;
    std::string mastfile = ".\\Saves\\";
	mastfile += filename;
	mastfile += ".mast";
	load.open(mastfile.c_str());

	numSaved = 0;
	if(load)
	{
		while(load.good())
		{
			load.getline(buffer, 1000, '\n');
			savedFiles[numSaved] = buffer;
			numSaved++;
		}
	}
	load.close();

	// Load Girls
	DirPath location = DirPath() << "Resources" << "Characters\\*.girls";
	hFind = FindFirstFileA(location.c_str(), &FindFileData);
	do
	{
		if(hFind != INVALID_HANDLE_VALUE)
		{
			bool match = false;
			for(int i=0; i<numSaved; i++)
			{
				if(savedFiles[i] == FindFileData.cFileName)
					match = true;
			}

			if(!match)
			{
				savedFiles[numSaved] = FindFileData.cFileName;
				numSaved++;
			}
		}
		else
			break;
	}
	while(FindNextFileA(hFind, &FindFileData) != 0);
	FindClose(hFind);

	std::ofstream save;
	mastfile = filename;
	mastfile += ".mast";
	save.open(mastfile.c_str());
	for(int j=0; j<numSaved; j++)
	{
		if(j==(numSaved-1))
			save<<savedFiles[j].c_str();
		else
			save<<savedFiles[j].c_str()<<std::endl;
	}
	save.close();

}
*/

void SaveGameXML(std::string filename)
{
	TiXmlDocument doc(filename);
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "yes");
	doc.LinkEndChild(decl);
	TiXmlElement* pRoot = new TiXmlElement("Root");
	doc.LinkEndChild(pRoot);

	//this replaces the "master file"
	loadedGirlsFiles.SaveXML(pRoot);

	// output game version
	pRoot->SetAttribute("MajorVersion", g_MajorVersion);
	pRoot->SetAttribute("MinorVersion", g_MinorVersion);
	pRoot->SetAttribute("PatchVersion", g_PatchVersion);
	pRoot->SetAttribute("MetadataVersion", g_MetadataVersion);

	//if a user mods the exe, he can tell us that here
	//by changing it to anything besides official
	pRoot->SetAttribute("ExeVersion", "official");

	// output interface variables
	pRoot->SetAttribute("WalkAround", g_WalkAround);
	pRoot->SetAttribute("TalkCount", g_TalkCount);

	// output player gold
	g_Gold.saveGoldXML(pRoot);

	// output year, month and day
	pRoot->SetAttribute("Year", g_Year);
	pRoot->SetAttribute("Month", g_Month);
	pRoot->SetAttribute("Day", g_Day);

	// output girls
	g_Girls.SaveGirlsXML(pRoot);

	// output gangs
	g_Gangs.SaveGangsXML(pRoot);

	// output brothels
	g_Brothels.SaveDataXML(pRoot);

	// output global triggers
	g_GlobalTriggers.SaveTriggersXML(pRoot);
	doc.SaveFile();


	//ADB TODO
#if 0
	//this stuff is not saved, save it and load it
	selected_girl = 0;
	for(int i=0; i<8; i++)
	{
		MarketSlaveGirls[i] = 0;
		MarketSlaveGirlsDel[i] = -1;
	}

	// update the shop inventory
	g_InvManager.UpdateShop();
#endif

}

bool LoadGame(std::string directory, std::string filename)
{
	// FREE ANYTHING
	//other stuff will be freed as it is loaded
	g_Traits.Free();
	g_Girls.Free();
	g_InvManager.Free();

	//load items database, traits info, etc
	LoadGameInfoFiles();

	DirPath thefile;
	thefile<<directory<<filename;
	TiXmlDocument doc(thefile.c_str());
	if (doc.LoadFile() == false)
	{
		g_LogFile.write("Tried and failed to parse savegame as XML file, error as follows");
		g_LogFile.write("(If it says 'Error document empty.' it's probably old format)");
		g_LogFile.write(doc.ErrorDesc());
		g_LogFile.write("Attempting to load savegame as old format");
		bool success = LoadGameLegacy(directory, filename);
		if (success == true)
		{
			g_LogFile.write("Successfully loaded from old savegame format");
		}
		else
		{
			g_LogFile.write("Unknown error, did not load any savegame format");
		}
		return success;
	}
	else
	{
		return LoadGameXML(TiXmlHandle(&doc));
	}
}

bool LoadGameXML(TiXmlHandle hDoc)
{
	TiXmlHandle hRoot = hDoc.FirstChildElement("Root");
	TiXmlElement* pRoot = hRoot.ToElement();
	if (pRoot == nullptr)
	{
		return false;
	}

	// load the version
	int minorA = -1;
	pRoot->QueryIntAttribute("MinorVersionA", &minorA);

	if(minorA != 3)
	{
		g_MessageQue.AddToQue("You must start a new game with this version", 2);
		return false;
	}

    std::string version("<blank>");
	if (pRoot->Attribute("ExeVersion"))
	{
		version = pRoot->Attribute("ExeVersion");
	}
	if (version != "official")
	{
		g_MessageQue.AddToQue("Warning, the exe was not detected as official, it was detected as "
			+ version + ".  Attempting to load anyways.", 1);
	}
/*
 *	We need to load the .girls/.girlsx files
 *
 *	To do that, we need to know which we might have already loaded
 *	that information is kept in the master file
 *	so we load that first
 */
	loadedGirlsFiles.LoadXML(hRoot.FirstChild("Loaded_Files"));
	LoadGirlsFiles();

	g_CurrBrothel = 0;

	// load interface variables
	g_WalkAround = false;
	pRoot->QueryValueAttribute<bool>("WalkAround", &g_WalkAround);
	g_TalkCount = 0;
	pRoot->QueryIntAttribute("TalkCount", &g_TalkCount);


	// load player gold
	g_Gold.loadGoldXML(hRoot.FirstChildElement("Gold"));

	// load year, month and day
	//if these aren't loaded, they default to whatever numbers they were before
	pRoot->QueryValueAttribute<unsigned long>("Year", &g_Year);
	pRoot->QueryValueAttribute<unsigned long>("Month", &g_Month);
	pRoot->QueryValueAttribute<unsigned long>("Day", &g_Day);

	// load main girls
	g_LogFile.write("Loading girls");
	g_Girls.LoadGirlsXML(hRoot.FirstChildElement("Girls"));

	// load gangs
	g_LogFile.write("Loading gang Data");
	g_Gangs.LoadGangsXML(hRoot.FirstChildElement("Gang_Manager"));

	// load brothels
	g_LogFile.write("Loading brothel Data");
	g_Brothels.LoadDataXML(hRoot.FirstChildElement("Brothel_Manager"));

	// load global triggers
	g_LogFile.write("Loading global triggers");
	g_GlobalTriggers.LoadTriggersXML(hRoot.FirstChildElement("Triggers"));

	g_LogFile.write("Loading default images");
	g_Girls.LoadDefaultImages();

	if(g_Cheats)
	{
		g_WalkAround = false;
		g_TalkCount = 10;
	}

	selected_girl = nullptr;
	for(int i=0; i<8; i++)
	{
		MarketSlaveGirls[i] = nullptr;
		MarketSlaveGirlsDel[i] = -1;
	}

	// update the shop inventory
	g_InvManager.UpdateShop();
	return true;
}

bool LoadGameLegacy(std::string directory, std::string filename)
{
	// load templates
	g_LogFile.write("Loading what used to be the master file");
/*
 *	We need to load the .girls/.girlsx files
 *
 *	To do that, we need to know which we might have already loaded
 *	that information is kept in the master file
 *	so we load that first
 */
	loadedGirlsFiles.LoadLegacy(filename);
	LoadGirlsFiles();

	int major = 0;
	int minorA = 0;
	int minorB = 0;
	int temp = 0;

	std::ifstream ifs;
	DirPath thefile;
	thefile<<directory<<filename;
	ifs.open(thefile.c_str());

	// load the version
	ifs>>major>>minorA>>minorB;

	if(minorA != 3)
	{
		g_MessageQue.AddToQue("You must start a new game with this version", 2);
		return false;
	}

	g_CurrBrothel = 0;

	// load interface variables
	if (ifs.peek()=='\n') ifs.ignore(1,'\n');
	ifs>>temp;
	if(temp == 1)
		g_WalkAround = true;
	else
		g_WalkAround = false;
	if (ifs.peek()=='\n') ifs.ignore(1,'\n');
	ifs>>g_TalkCount;


	// load player gold
	if (ifs.peek()=='\n') ifs.ignore(1,'\n');
	g_Gold.loadGoldLegacy(ifs);

	// load year, month and day
	if (ifs.peek()=='\n') ifs.ignore(1,'\n');
	ifs>>g_Year>>g_Month>>g_Day;

	// load main girls
	g_LogFile.write("Loading girls");
	g_Girls.LoadGirlsLegacy(ifs);

	// load gangs
	g_LogFile.write("Loading gang Data");
	g_Gangs.LoadGangsLegacy(ifs);

	// load brothels
	g_LogFile.write("Loading brothel Data");
	g_Brothels.LoadDataLegacy(ifs);

	// load global triggers
	g_LogFile.write("Loading global triggers");
	g_GlobalTriggers.LoadTriggersLegacy(ifs);

	g_LogFile.write("Loading default images");
	g_Girls.LoadDefaultImages();

	ifs.close();

	if(g_Cheats)
	{
		g_WalkAround = false;
		g_TalkCount = 10;
	}

	selected_girl = nullptr;
	for(int i=0; i<8; i++)
	{
		MarketSlaveGirls[i] = nullptr;
		MarketSlaveGirlsDel[i] = -1;
	}

	// update the shop inventory
	g_InvManager.UpdateShop();
	return true;
}

void TransferGirls()
{
	static int leftBrothel = 0;
	static int rightBrothel = 0;

	if(g_InitWin)
	{
		leftBrothel = rightBrothel = 0;
		// clear list boxes
		g_TransferGirls.ClearListBox(g_interfaceid.LIST_TRANSGLEFTBROTHEL);
		g_TransferGirls.ClearListBox(g_interfaceid.LIST_TRANSGRIGHTBROTHEL);
		g_TransferGirls.ClearListBox(g_interfaceid.LIST_TRANSGLEFTGIRLS);
		g_TransferGirls.ClearListBox(g_interfaceid.LIST_TRANSGRIGHTGIRLS);

		// list all the brothels
		Brothel* current = g_Brothels.GetBrothel(0);
		int i=0;
		while(current)
		{
			g_TransferGirls.AddToListBox(g_interfaceid.LIST_TRANSGLEFTBROTHEL, i, current->m_Name);
			g_TransferGirls.AddToListBox(g_interfaceid.LIST_TRANSGRIGHTBROTHEL, i, current->m_Name);
			i++;
			current = current->m_Next;
		}

		g_TransferGirls.SetSelectedItemInList(g_interfaceid.LIST_TRANSGLEFTBROTHEL, 0);
		g_TransferGirls.SetSelectedItemInList(g_interfaceid.LIST_TRANSGRIGHTBROTHEL, 0);

		g_InitWin = false;
	}

	if(g_InterfaceEvents.GetNumEvents() != 0)
	{
		if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TRANSGBACK))
		{
			g_InitWin = true;
			g_WinManager.Pop();
			return;
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TRANSGSHIFTR))
		{
			if((rightBrothel != -1 && leftBrothel != -1))
			{
				Brothel* brothel = g_Brothels.GetBrothel(rightBrothel);
				if(brothel->m_NumGirls == brothel->m_NumRooms)
					g_MessageQue.AddToQue("Right side brothel is full", 1);
				else
				{
					int pos = 0;
					int NumRemoved = 0;
					int girlSelection = g_TransferGirls.GetNextSelectedItemFromList(g_interfaceid.LIST_TRANSGLEFTGIRLS, 0, pos);
					while(girlSelection != -1)
					{
						// get the girl
						Girl* temp = g_Brothels.GetGirl(leftBrothel, girlSelection-NumRemoved);

						// check there is still room
						if(brothel->m_NumGirls+1 > brothel->m_NumRooms)
						{
							g_MessageQue.AddToQue("Right side brothel is full", 1);
							break;
						}

						// remove girl from left side
						NumRemoved++;
						g_Brothels.RemoveGirl(leftBrothel, temp, false);

						// add to right side
						g_Brothels.AddGirl(rightBrothel, temp);

						// get next girl
						girlSelection = g_TransferGirls.GetNextSelectedItemFromList(g_interfaceid.LIST_TRANSGLEFTGIRLS, pos+1, pos);
					}

					// update the girl lists
					g_TransferGirls.SetSelectedItemInList(g_interfaceid.LIST_TRANSGLEFTBROTHEL, leftBrothel);
					g_TransferGirls.SetSelectedItemInList(g_interfaceid.LIST_TRANSGRIGHTBROTHEL, rightBrothel);
				}
			}
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_BUTTONCLICKED, g_interfaceid.BUTTON_TRANSGSHIFTL))
		{
			if((rightBrothel != -1 && leftBrothel != -1))
			{
				Brothel* brothel = g_Brothels.GetBrothel(leftBrothel);
				if(brothel->m_NumGirls == brothel->m_NumRooms)
					g_MessageQue.AddToQue("Left side brothel is full", 1);
				else
				{
					int pos = 0;
					int NumRemoved = 0;
					int girlSelection = g_TransferGirls.GetNextSelectedItemFromList(g_interfaceid.LIST_TRANSGRIGHTGIRLS, 0, pos);
					while(girlSelection != -1)
					{
						// get the girl
						Girl* temp = g_Brothels.GetGirl(rightBrothel, girlSelection-NumRemoved);

						// check there is still room
						if(brothel->m_NumGirls+1 > brothel->m_NumRooms)
						{
							g_MessageQue.AddToQue("Left side brothel is full", 1);
							break;
						}

						// remove girl from right side
						NumRemoved++;
						g_Brothels.RemoveGirl(rightBrothel, temp, false);

						// add to left side
						g_Brothels.AddGirl(leftBrothel, temp);

						// get next girl
						girlSelection = g_TransferGirls.GetNextSelectedItemFromList(g_interfaceid.LIST_TRANSGLEFTGIRLS, pos+1, pos);
					}

					// update the girl lists
					g_TransferGirls.SetSelectedItemInList(g_interfaceid.LIST_TRANSGLEFTBROTHEL, leftBrothel);
					g_TransferGirls.SetSelectedItemInList(g_interfaceid.LIST_TRANSGRIGHTBROTHEL, rightBrothel);
				}
			}
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_TRANSGLEFTBROTHEL))
		{
			g_TransferGirls.ClearListBox(g_interfaceid.LIST_TRANSGLEFTGIRLS);
			leftBrothel = g_TransferGirls.GetSelectedItemFromList(g_interfaceid.LIST_TRANSGLEFTBROTHEL);
			if(leftBrothel != -1)
			{
				// add the girls to the list
				Girl* temp = g_Brothels.GetGirl(leftBrothel, 0);
				int i=0;
				while(temp)
				{
					g_TransferGirls.AddToListBox(g_interfaceid.LIST_TRANSGLEFTGIRLS, i, temp->m_Realname);
					i++;
					temp = temp->m_Next;
				}
			}
		}
		else if(g_InterfaceEvents.CheckEvent(EVENT_SELECTIONCHANGE, g_interfaceid.LIST_TRANSGRIGHTBROTHEL))
		{
			g_TransferGirls.ClearListBox(g_interfaceid.LIST_TRANSGRIGHTGIRLS);
			rightBrothel = g_TransferGirls.GetSelectedItemFromList(g_interfaceid.LIST_TRANSGRIGHTBROTHEL);
			if(rightBrothel != -1)
			{
				// add the girls to the list
				Girl* temp = g_Brothels.GetGirl(rightBrothel, 0);
				int i=0;
				while(temp)
				{
					g_TransferGirls.AddToListBox(g_interfaceid.LIST_TRANSGRIGHTGIRLS, i, temp->m_Realname);
					i++;
					temp = temp->m_Next;
				}
			}
		}
	}
}

} // namespace WhoreMasterRenewal
