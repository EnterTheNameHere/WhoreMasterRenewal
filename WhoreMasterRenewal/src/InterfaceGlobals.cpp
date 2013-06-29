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

#include "InterfaceGlobals.h"
#include "DirPath.h"
#include "Brothel.hpp"
#include "Constants.h"
#include "cTariff.h"
#include "cWindowManager.h"
#include "cScreenBuildingManagement.h"
#include "cScreenPrison.h"
#include "cScreenTown.h"
#include "cScreenSlaveMarket.h"
#include "cScreenMayor.h"
#include "cScreenBank.h"
#include "cScreenHouse.h"
#include "cScreenItemManagement.h"
#include "cScreenBuildingSetup.h"
#include "cScreenGangs.h"
#include "cScreenGirlManagement.h"
#include "cScreenGirlDetails.h"
#include "cScreenDungeon.h"
#include "InterfaceIDs.h"
#include "cMessageBox.h"
#include "cChoiceMessage.h"
#include "cInterfaceEvent.h"
#include "CLog.h"

#include <iostream>

namespace WhoreMasterRenewal
{

// interface colors
unsigned char g_StaticImageR = 0, g_StaticImageG = 0, g_StaticImageB = 0;

unsigned char g_ChoiceMessageTextR = 0, g_ChoiceMessageTextG = 0, g_ChoiceMessageTextB = 0;
unsigned char g_ChoiceMessageBorderR = 0, g_ChoiceMessageBorderG = 0, g_ChoiceMessageBorderB = 0;
unsigned char g_ChoiceMessageBackgroundR = 0, g_ChoiceMessageBackgroundG = 0, g_ChoiceMessageBackgroundB = 0;
unsigned char g_ChoiceMessageSelectedR = 0, g_ChoiceMessageSelectedG = 0, g_ChoiceMessageSelectedB = 0;

unsigned char g_EditBoxBorderR = 0, g_EditBoxBorderG = 0, g_EditBoxBorderB = 0;
unsigned char g_EditBoxBackgroundR = 0, g_EditBoxBackgroundG = 0, g_EditBoxBackgroundB = 0;
unsigned char g_EditBoxSelectedR = 0, g_EditBoxSelectedG = 0, g_EditBoxSelectedB = 0;
unsigned char g_EditBoxTextR = 0, g_EditBoxTextG = 0, g_EditBoxTextB = 0;

unsigned char g_CheckBoxBorderR = 0, g_CheckBoxBorderG = 0, g_CheckBoxBorderB = 0;
unsigned char g_CheckBoxBackgroundR = 0, g_CheckBoxBackgroundG = 0, g_CheckBoxBackgroundB = 0;

unsigned char g_WindowBorderR = 0, g_WindowBorderG = 0, g_WindowBorderB = 0;
unsigned char g_WindowBackgroundR = 0, g_WindowBackgroundG = 0, g_WindowBackgroundB = 0;

unsigned char g_ListBoxBorderR = 0, g_ListBoxBorderG = 0, g_ListBoxBorderB = 0;
unsigned char g_ListBoxBackgroundR = 0, g_ListBoxBackgroundG = 0, g_ListBoxBackgroundB = 0;
unsigned char g_ListBoxElementBackgroundR = 0, g_ListBoxElementBackgroundG = 0, g_ListBoxElementBackgroundB = 0;
unsigned char g_ListBoxS1ElementBackgroundR = 0, g_ListBoxS1ElementBackgroundG = 0, g_ListBoxS1ElementBackgroundB = 0;
unsigned char g_ListBoxS2ElementBackgroundR, g_ListBoxS2ElementBackgroundG, g_ListBoxS2ElementBackgroundB;
unsigned char g_ListBoxSelectedElementR = 0, g_ListBoxSelectedElementG = 0, g_ListBoxSelectedElementB = 0;
unsigned char g_ListBoxSelectedS1ElementR = 0, g_ListBoxSelectedS1ElementG = 0, g_ListBoxSelectedS1ElementB = 0;
unsigned char g_ListBoxSelectedS2ElementR = 0, g_ListBoxSelectedS2ElementG = 0, g_ListBoxSelectedS2ElementB = 0;
unsigned char g_ListBoxElementBorderR = 0, g_ListBoxElementBorderG = 0, g_ListBoxElementBorderB = 0;
unsigned char g_ListBoxElementBorderHR = 0, g_ListBoxElementBorderHG = 0, g_ListBoxElementBorderHB = 0;
unsigned char g_ListBoxTextR = 0, g_ListBoxTextG = 0, g_ListBoxTextB = 0;

unsigned char g_ListBoxHeaderBackgroundR = 0, g_ListBoxHeaderBackgroundG = 0, g_ListBoxHeaderBackgroundB = 0;
unsigned char g_ListBoxHeaderBorderR = 0, g_ListBoxHeaderBorderG = 0, g_ListBoxHeaderBorderB = 0;
unsigned char g_ListBoxHeaderBorderHR = 0, g_ListBoxHeaderBorderHG = 0, g_ListBoxHeaderBorderHB = 0;
unsigned char g_ListBoxHeaderTextR = 0, g_ListBoxHeaderTextG = 0, g_ListBoxHeaderTextB = 0;

unsigned char g_MessageBoxBorderR = 0, g_MessageBoxBorderG = 0, g_MessageBoxBorderB = 0;
unsigned char g_MessageBoxBackground0R = 0, g_MessageBoxBackground0G = 0, g_MessageBoxBackground0B = 0;
unsigned char g_MessageBoxBackground1R = 0, g_MessageBoxBackground1G = 0, g_MessageBoxBackground1B = 0;
unsigned char g_MessageBoxBackground2R = 0, g_MessageBoxBackground2G = 0, g_MessageBoxBackground2B = 0;
unsigned char g_MessageBoxBackground3R = 0, g_MessageBoxBackground3G = 0, g_MessageBoxBackground3B = 0;
unsigned char g_MessageBoxTextR = 0, g_MessageBoxTextG = 0, g_MessageBoxTextB = 0;

unsigned long g_Year = 0;
unsigned long g_Month = 0;
unsigned long g_Day = 0;

bool g_ShiftDown = false;
bool g_CTRLDown = false;
int g_ScreenWidth = 800;
int g_ScreenHeight = 600;
bool g_Fullscreen = false;

void FreeInterface()
{
    //g_LogFile.ss() << "Freeing interface...";
    //g_LogFile.ssend();
    
	g_MainMenu.Free();
	g_GetString.Free();
	g_BrothelManagement.Free();
	g_GirlManagement.Free();
	g_GangManagement.Free();
	g_GirlDetails.Free();
	g_ChangeJobs.Free();
	g_Turnsummary.Free();
	g_Dungeon.Free();
	g_SlaveMarket.Free();
	g_TownScreen.Free();
	g_Gallery.Free();
	g_BuildingSetupScreen.Free();
	g_GetInt.Free();
	g_MayorsOfficeScreen.Free();
	g_BankScreen.Free();
	g_ChoiceManager.Free();
	g_MessageQue.Free();
	g_LoadGame.Free();
	g_PlayersHouse.Free();
	g_TransferGirls.Free();
	g_ItemManagement.Free();
	g_PrisonScreen.Free();
	g_BuildingManagementScreen.Reset();
	
	//g_LogFile.ss() << "Done freeing interface...";
    //g_LogFile.ssend();
}

void ResetInterface()
{
    //g_LogFile.ss() << "Resetting interface...";
    //g_LogFile.ssend();
    
	g_MainMenu.Reset();
	g_GetString.Reset();
	g_BrothelManagement.Reset();
	g_GirlManagement.Reset();
	g_GangManagement.Reset();
	g_GirlDetails.Reset();
	g_ChangeJobs.Reset();
	g_Turnsummary.Reset();
	g_Dungeon.Reset();
	g_SlaveMarket.Reset();
	g_TownScreen.Reset();
	g_Gallery.Reset();
	g_BuildingSetupScreen.Reset();
	g_GetInt.Reset();
	g_MayorsOfficeScreen.Reset();
	g_BankScreen.Reset();
	g_ChoiceManager.Free();
	g_LoadGame.Reset();
	g_PlayersHouse.Reset();
	g_TransferGirls.Reset();
	g_ItemManagement.Reset();
	g_PrisonScreen.Reset();
	g_BuildingManagementScreen.Reset();
	
	//g_LogFile.ss() << "Done resetting interface...";
    //g_LogFile.ssend();
}

void LoadInterface()
{
	cTariff tariff;
	int r=0, g=0, b=0;
	int a=0, c=0, d=0, e=0;
	std::ifstream incol;

	// load 
	// load interface colors
	// WD: Typecast to resolve ambiguous call in VS 2010
	g_LogFile.write("Loading InterfaceColors");
	DirPath dp = DirPath() << "Resources" << "Interface" << "InterfaceColors.txt";
	incol.open(dp.c_str());
	//incol.open(DirPath() << "Resources" << "Interface" << "InterfaceColors.txt");
	incol.seekg(0);
	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_StaticImageR = r;
	g_StaticImageG = g;
	g_StaticImageB = b;
	
	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ChoiceMessageTextR = r;
	g_ChoiceMessageTextG = g;
	g_ChoiceMessageTextB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ChoiceMessageBorderR = r;
	g_ChoiceMessageBorderG = g;
	g_ChoiceMessageBorderB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ChoiceMessageBackgroundR = r;
	g_ChoiceMessageBackgroundG = g;
	g_ChoiceMessageBackgroundB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ChoiceMessageSelectedR = r;
	g_ChoiceMessageSelectedG = g;
	g_ChoiceMessageSelectedB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_EditBoxBorderR = r;
	g_EditBoxBorderG = g;
	g_EditBoxBorderB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_EditBoxBackgroundR = r;
	g_EditBoxBackgroundG = g;
	g_EditBoxBackgroundB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_EditBoxSelectedR= r;
	g_EditBoxSelectedG = g;
	g_EditBoxSelectedB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_EditBoxTextR = r;
	g_EditBoxTextG = g;
	g_EditBoxTextB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_WindowBorderR = r;
	g_WindowBorderG =  g;
	g_WindowBorderB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_WindowBackgroundR = r;
	g_WindowBackgroundG = g;
	g_WindowBackgroundB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxBorderR = r;
	g_ListBoxBorderG = g;
	g_ListBoxBorderB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxBackgroundR = r;
	g_ListBoxBackgroundG = g;
	g_ListBoxBackgroundB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxElementBackgroundR = r;
	g_ListBoxElementBackgroundG = g;
	g_ListBoxElementBackgroundB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxS1ElementBackgroundR = r;
	g_ListBoxS1ElementBackgroundG = g;
	g_ListBoxS1ElementBackgroundB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxS2ElementBackgroundR = r;
	g_ListBoxS2ElementBackgroundG = g;
	g_ListBoxS2ElementBackgroundB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxSelectedElementR = r;
	g_ListBoxSelectedElementG = g;
	g_ListBoxSelectedElementB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxSelectedS1ElementR = r;
	g_ListBoxSelectedS1ElementG = g;
	g_ListBoxSelectedS1ElementB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxSelectedS2ElementR = r;
	g_ListBoxSelectedS2ElementG = g;
	g_ListBoxSelectedS2ElementB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxElementBorderR = r;
	g_ListBoxElementBorderG = g;
	g_ListBoxElementBorderB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxElementBorderHR = r;
	g_ListBoxElementBorderHG = g;
	g_ListBoxElementBorderHB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxTextR = r;
	g_ListBoxTextG = g;
	g_ListBoxTextB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxHeaderBackgroundR = r;
	g_ListBoxHeaderBackgroundG = g;
	g_ListBoxHeaderBackgroundB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxHeaderBorderR = r;
	g_ListBoxHeaderBorderG = g;
	g_ListBoxHeaderBorderB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxHeaderBorderHR = r;
	g_ListBoxHeaderBorderHG = g;
	g_ListBoxHeaderBorderHB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_ListBoxHeaderTextR = r;
	g_ListBoxHeaderTextG = g;
	g_ListBoxHeaderTextB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_MessageBoxBorderR = r;
	g_MessageBoxBorderG = g;
	g_MessageBoxBorderB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_MessageBoxBackground0R = r;
	g_MessageBoxBackground0G = g;
	g_MessageBoxBackground0B = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_MessageBoxBackground1R = r;
	g_MessageBoxBackground1G = g;
	g_MessageBoxBackground1B = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_MessageBoxBackground2R = r;
	g_MessageBoxBackground2G = g;
	g_MessageBoxBackground2B = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_MessageBoxBackground3R = r;
	g_MessageBoxBackground3G = g;
	g_MessageBoxBackground3B = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_MessageBoxTextR = r;
	g_MessageBoxTextG = g;
	g_MessageBoxTextB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_CheckBoxBorderR = r;
	g_CheckBoxBorderG = g;
	g_CheckBoxBorderB = b;

	incol>>r>>g>>b;incol.ignore(1000, '\n');
	g_CheckBoxBackgroundR = r;
	g_CheckBoxBackgroundG = g;
	g_CheckBoxBackgroundB = b;
	incol.close();


	// Main Menu
	g_LogFile.write("Loading Main Menu Screen");
	g_MainMenu.AddTextItem(g_interfaceid.STATIC_STATIC, 0, 0, 600, 32, "Please read the readme.html");
	
	// load main menu stuff
	// WD: Typecast to resolve ambiguous call in VS 2010
	g_LogFile.write("Loading MainMenu");
	dp = DirPath() << "Resources" << "Interface" << "MainMenu.txt";
	incol.open(dp.c_str());
	//incol.open(DirPath() << "Resources" << "Interface" << "MainMenu.txt");
	incol.seekg(0);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_MainMenu.CreateWindow(a,b,c,d,e);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');

	g_MainMenu.AddButtonND("NewGame", g_interfaceid.BUTTON_NEWGAME, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_MainMenu.AddButtonND("LoadGame", g_interfaceid.BUTTON_LOADGAME, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_MainMenu.AddButtonND("QuitGame", g_interfaceid.BUTTON_EXITGAME, a, b, c, d, true);
	incol.close();

	// Load game screen
	g_LogFile.write("Loading Load Game Screen");
	g_LoadGame.AddTextItem(g_interfaceid.STATIC_STATIC, 0, 0, 600, 32, "Please read the readme.html");
	// WD: Typecast to resolve ambiguous call in VS 2010
	dp = DirPath() << "Resources" << "Interface" << "LoadMenu.txt";
	incol.open(dp.c_str());
	//incol.open(DirPath() << "Resources" << "Interface" << "LoadMenu.txt");
	incol.seekg(0);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_LoadGame.CreateWindow(a,b,c,d,e);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_LoadGame.AddListBox(g_interfaceid.LIST_LOADGSAVES, a, b, c, d, e, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_LoadGame.AddButton("Load", g_interfaceid.BUTTON_LOADGLOAD, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_LoadGame.AddButton("Back", g_interfaceid.BUTTON_LOADGBACK, a, b, c, d, true);
	incol.close();

	// Get string
	// WD: Typecast to resolve ambiguous call in VS 2010
	g_LogFile.write("Loading Get String Screen");
	dp = DirPath() << "Resources" << "Interface" << "GetString.txt";
	incol.open(dp.c_str());
	//incol.open(DirPath() << "Resources" << "Interface" << "GetString.txt");
	incol.seekg(0);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_GetString.CreateWindow(a,b,c,d,e);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_GetString.AddButton("Ok", g_interfaceid.BUTTON_OK, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_GetString.AddButton("Cancel", g_interfaceid.BUTTON_CANCEL, a, b, c, d, true);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_GetString.AddTextItem(g_interfaceid.TEXT_TEXT1, a, b, c, d, "Enter Text:", e);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_GetString.AddEditBox(g_interfaceid.EDITBOX_NAME,a,b,c,d,e);
	incol.close();

	// Get Int
	// WD: Typecast to resolve ambiguous call in VS 2010
	g_LogFile.write("Loading Get Int Screen");
	dp = DirPath() << "Resources" << "Interface" << "GetInt.txt";
	incol.open(dp.c_str());
	//incol.open(DirPath() << "Resources" << "Interface" << "GetInt.txt");
	incol.seekg(0);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_GetInt.CreateWindow(a,b,c,d,e);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_GetInt.AddButton("Ok", g_interfaceid.BUTTON_OK, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_GetInt.AddButton("Cancel", g_interfaceid.BUTTON_CANCEL, a, b, c, d, true);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_GetInt.AddTextItem(g_interfaceid.TEXT_TEXT1, a, b, c, d, "Enter Number:", e);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_GetInt.AddEditBox(g_interfaceid.EDITBOX_NAME,a,b,c,d,e);
	incol.close();

	// Brothel management screen
	g_LogFile.write("Loading Brothel Management Screen");
	g_BrothelManagement.AddTextItem(g_interfaceid.TEXT_CURRENTBROTHEL, 0, 0, 900, 32, "", 10);
	// WD: Typecast to resolve ambiguous call in VS 2010
	dp = DirPath() <<	"Resources" << "Interface" << "BrothelScreen.txt";
	incol.open(dp.c_str());
	//incol.open(DirPath() << "Resources" << "Interface" << "BrothelScreen.txt");
	incol.seekg(0);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_BrothelManagement.CreateWindow(a,b,c,d,e);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddButton("Girls", g_interfaceid.BUTTON_MANAGEGIRLS, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddButton("Staff", g_interfaceid.BUTTON_MANAGESTAFF, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddButton("Building", g_interfaceid.BUTTON_MANAGEBUILDING, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddButton("Dungeon", g_interfaceid.BUTTON_VISITDUNGEON, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddButton("VisitTown", g_interfaceid.BUTTON_VISITTOWN, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddButton("Save", g_interfaceid.BUTTON_SAVEGAME, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddButton("NextWeek", g_interfaceid.BUTTON_NEWWEEK, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddButton("TurnSum", g_interfaceid.BUTTON_TURNSUMMARY, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddButton("Quit", g_interfaceid.BUTTON_QUIT, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddButton("Prev", g_interfaceid.BUTTON_PREVBROTHEL, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddButton("Next", g_interfaceid.BUTTON_NEXTBROTHEL, a, b, c, d, true);
	incol>>a>>b>>c>>d;incol.ignore(1000, '\n');
	g_BrothelManagement.AddImage(g_interfaceid.IMAGE_BIMAGE, "", a, b, c, d);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_BrothelManagement.AddTextItem(g_interfaceid.TEXT_BMDETAILS, a, b, c, d, "", e);
	incol>>a>>b>>c>>d>>e;incol.ignore(1000, '\n');
	g_BrothelManagement.AddTextItem(g_interfaceid.TEXT_BROTHELNAME, a, b, c, d, "", e);
	incol.close();

	// GIRL MANAGEMENT SCREEN
	g_LogFile.write("Loading Girl Management Screen");
	g_GirlManagement.load();
	g_WinManager.add_window("Girl Management", &g_GirlManagement);

	// GIRL DETAILS
	g_LogFile.write("Loading Girl Details Screen");
	g_GirlDetails.load();
	g_WinManager.add_window("Girl Details", &g_GirlDetails);

	// Change Jobs Screen
	g_LogFile.write("Loading Change Jobs Screen");
	g_ChangeJobs.CreateWindow(256,120,304,376,1);
	g_ChangeJobs.AddButton("Ok", g_interfaceid.BUTTON_CJOK, 8, 336, 128, 32, true);
	g_ChangeJobs.AddButton("Cancel", g_interfaceid.BUTTON_CJCANCEL, 152, 336, 128, 32, true);
	g_ChangeJobs.AddTextItem(g_interfaceid.TEXT_CJDESC, 8, 240, 272, 88, "", 10);
	g_ChangeJobs.AddListBox(g_interfaceid.LIST_CJDAYTIME, 8, 48, 144, 192, 1, true);
	g_ChangeJobs.AddListBox(g_interfaceid.LIST_CJNIGHTTIME, 152, 48, 144, 192, 1, true);
	g_ChangeJobs.AddTextItem(g_interfaceid.STATIC_STATIC, 8, 8, 128, 32, "Day Shift");
	g_ChangeJobs.AddTextItem(g_interfaceid.STATIC_STATIC, 152, 8, 128, 32, "Night Shift");

	// Gang Management
	g_LogFile.write("Loading Gang Management Screen");
	g_GangManagement.load();
	g_WinManager.add_window("Gangs", &g_GangManagement);

	// Turn summary screen
	g_LogFile.write("Loading Turn Summary Screen");
	g_Turnsummary.AddTextItem(g_interfaceid.TEXT_CURRENTBROTHEL, 0, 0, 900, 32, "", 10);
	g_Turnsummary.CreateWindow(8,8,786,584,1);
	g_Turnsummary.AddTextItem(g_interfaceid.STATIC_STATIC, 8, 8, 120, 32, "Category", 15);
	g_Turnsummary.AddTextItem(g_interfaceid.STATIC_STATIC, 8, 106, 120, 32, "Item", 15);
	g_Turnsummary.AddTextItem(g_interfaceid.STATIC_STATIC, 8, 266, 120, 32, "Event", 15);
	g_Turnsummary.AddTextItem(g_interfaceid.TEXT_TSEVENTDESC, 575, 8, 202, 406, "", 12);
	g_Turnsummary.AddButton("GoTo", g_interfaceid.BUTTON_TSGOTO, 598, 462, 160, 32, true);
	g_Turnsummary.AddButton("NextWeek", g_interfaceid.BUTTON_TSNEWWEEK, 598, 504, 160, 32, true);
	g_Turnsummary.AddButton("Back", g_interfaceid.BUTTON_TSCLOSE, 598, 546, 160, 32, true);
	g_Turnsummary.AddButton("Prev", g_interfaceid.BUTTON_TSPREVBROTHEL, 598, 422, 72, 32, true);
	g_Turnsummary.AddButton("Next", g_interfaceid.BUTTON_TSNEXTBROTHEL, 686, 422, 72, 32, true);
	g_Turnsummary.AddListBox(g_interfaceid.LIST_TSCATEGORY, 8, 40, 120, 66, 1, true);
	g_Turnsummary.AddListBox(g_interfaceid.LIST_TSITEM, 8, 136, 120, 128, 1, true);
	g_Turnsummary.AddListBox(g_interfaceid.LIST_TSEVENTS, 8, 296, 120, 282, 1, true);
	g_Turnsummary.AddImage(g_interfaceid.IMAGE_TSIMAGE, "", 136, 8, 434, 570);

	// Dungeon Screen
	g_LogFile.write("Loading Dungeon Screen");
	g_Dungeon.load();
	g_WinManager.add_window("Dungeon", &g_Dungeon);

	// Town screen
	g_LogFile.write("Loading Town Screen");
	g_TownScreen.load();
	g_WinManager.add_window("Town", &g_TownScreen);

	// Slave market screen
	g_LogFile.write("Loading Slave Market Screen");
	g_SlaveMarket.load();
	g_WinManager.add_window("Slave Market", &g_SlaveMarket);

	// gallery screen
	g_LogFile.write("Loading Gallery Screen");
	g_Gallery.CreateWindow(16,16,768,576,1);
	g_Gallery.AddButton("Anal", g_interfaceid.BUTTON_GALLERYANAL, 600, 8, 160, 32, true);
	g_Gallery.AddButton("BDSM", g_interfaceid.BUTTON_GALLERYBDSM, 600, 48, 160, 32, true);
	g_Gallery.AddButton("Sex", g_interfaceid.BUTTON_GALLERYSEX, 600, 88, 160, 32, true);
	g_Gallery.AddButton("Beast", g_interfaceid.BUTTON_GALLERYBEAST, 600, 128, 160, 32, true);
	g_Gallery.AddButton("Group", g_interfaceid.BUTTON_GALLERYGROUP, 600, 168, 160, 32, true);
	g_Gallery.AddButton("Lesbian", g_interfaceid.BUTTON_GALLERYLESBIAN, 600, 208, 160, 32, true);
	g_Gallery.AddButton("Pregnant", g_interfaceid.BUTTON_GALLERYPREGNANT, 600, 248, 160, 32, true);
	g_Gallery.AddButton("Death", g_interfaceid.BUTTON_GALLERYDEATH, 600, 288, 160, 32, true);
	g_Gallery.AddButton("Profile", g_interfaceid.BUTTON_GALLERYPROFILE, 600, 328, 160, 32, true);
	g_Gallery.AddButton("Back", g_interfaceid.BUTTON_GALLERYBACK, 600, 368, 160, 32, true);
	g_Gallery.AddButton("Prev", g_interfaceid.BUTTON_GALLERYPREV, 600, 408, 72, 32, true);
	g_Gallery.AddButton("Next", g_interfaceid.BUTTON_GALLERYNEXT, 688, 408, 72, 32, true);
	g_Gallery.AddImage(g_interfaceid.IMAGE_GALLERYIMAGE, "", 8, 8, 584, 560);

	// Building Setup Screen
	g_LogFile.write("Loading Building Setup Screen");
	g_BuildingSetupScreen.load();
	g_WinManager.add_window("Building Setup", &g_BuildingSetupScreen);

	// Mayors office
	g_LogFile.write("Loading Mayor Screen");
	g_MayorsOfficeScreen.load();
	g_WinManager.add_window("Mayor", &g_MayorsOfficeScreen);

	// Bank Screen
	g_LogFile.write("Loading Bank Screen");
	g_BankScreen.load();
	g_WinManager.add_window("Bank", &g_BankScreen);

	// Players house (statistic screen)
	g_LogFile.write("Loading House Screen");
	g_PlayersHouse.load();
	g_WinManager.add_window("House", &g_PlayersHouse);

	// Transfer Girls Screen
	g_LogFile.write("Loading Transfer Girls Screen");
	g_TransferGirls.CreateWindow(16,16,768,576,1);
	g_TransferGirls.AddButton("Back", g_interfaceid.BUTTON_TRANSGBACK, 308, 536, 160, 32, true);
	g_TransferGirls.AddButton("ShiftLeft", g_interfaceid.BUTTON_TRANSGSHIFTL, 366, 214, 48, 48, true);
	g_TransferGirls.AddButton("ShiftRight", g_interfaceid.BUTTON_TRANSGSHIFTR, 366, 304, 48, 48, true);
	g_TransferGirls.AddListBox(g_interfaceid.LIST_TRANSGLEFTGIRLS,168, 48, 190, 482, 1, true, true);
	g_TransferGirls.AddListBox(g_interfaceid.LIST_TRANSGRIGHTGIRLS,418, 48, 190, 482, 1, true, true);
	g_TransferGirls.AddListBox(g_interfaceid.LIST_TRANSGLEFTBROTHEL, 8, 48, 126, 234, 1, true, false);
	g_TransferGirls.AddListBox(g_interfaceid.LIST_TRANSGRIGHTBROTHEL,632, 48, 126, 234, 1, true, false);
	g_TransferGirls.AddTextItem(g_interfaceid.STATIC_STATIC, 8, 8, 160, 32, "BROTHELS");
	g_TransferGirls.AddTextItem(g_interfaceid.STATIC_STATIC, 632, 8, 160, 32, "BROTHELS");

	// Item Management Screen
	g_LogFile.write("Loading Item Management Screen");
	g_ItemManagement.load();
	g_WinManager.add_window("Item Management", &g_ItemManagement);

	// Prison Screen
	g_LogFile.write("Loading Prison Screen");
	g_PrisonScreen.load();
	g_WinManager.add_window("Prison", &g_PrisonScreen);
	
	// Building Management Screen
	g_LogFile.write("Loading Building Management Screen");
	g_BuildingManagementScreen.load();
	g_WinManager.add_window("Building Management", &g_BuildingManagementScreen);

	// Setup the messagebox
	g_LogFile.write("Setting up MessageBox");
	g_interfaceid.STATIC_STATIC=1;//evil magic number
	g_MessageBox.CreateWindow();
}

} // namespace WhoreMasterRenewal
