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
 
#include "main.h"
#include "BrothelManager.hpp"
#include "Logger.hpp"
#include "Helper.hpp"
#include "InterfaceGlobals.h"
#include "InterfaceProcesses.h"
#include "sConfig.h"
#include "CSurface.h"
#include "cJobManager.h"
#include "cWindowManager.h"
#include "cMessageBox.h"
#include "cChoiceMessage.h"
#include "Brothel.hpp"
#include "cGangs.h"
#include "GangManager.hpp"
#include "IconSurface.h"
#include "cSlider.h"
#include "cScrollBar.h"
#include "GameFlags.h"
#include "cGirls.h"
#include "GirlManager.hpp"
#include "Girl.hpp"
#include "cTraits.h"
#include "cCustomers.h"
#include "cInventory.h"
#include "CGraphics.h"
#include "cTriggers.h"
#include "cRng.h"
#include "InterfaceIDs.h"
#include "cInterfaceEvent.h"
#include "cInterfaceWindow.h"
#include "cScreenBank.h"
#include "cScreenBuildingManagement.h"
#include "cScreenBuildingSetup.h"
#include "cScreenDungeon.h"
#include "cScreenGangs.h"
#include "cScreenGirlDetails.h"
#include "cScreenGirlManagement.h"
#include "cScreenHouse.h"
#include "cScreenItemManagement.h"
#include "cScreenMayor.h"
#include "cScreenPrison.h"
#include "cScreenSlaveMarket.h"
#include "cScreenTown.h"
#include "cGetStringScreenManager.h"

//#include <SDL.h>
#include <SDL_events.h>
#include <SDL_keysym.h>

#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <csignal>

namespace WhoreMasterRenewal
{

// Function Defs
void Shutdown();
bool Init();

// Events
CLog g_LogFile;
SDL_Event vent;
BrothelManager g_Brothels;
cCustomers g_Customers;
cGangManager g_Gangs;
cGold g_Gold;
CGraphics g_Graphics;
cInventory g_InvManager;
cRng g_Dice;
cTraits g_Traits;
cWindowManager g_WinManager;
sInterfaceIDs g_interfaceid;
cInterfaceEventManager g_InterfaceEvents;
cInterfaceWindow g_MainMenu;
cInterfaceWindow g_GetString;
cInterfaceWindow g_BrothelManagement;
cInterfaceWindow g_ChangeJobs;
cInterfaceWindow g_Turnsummary;
cInterfaceWindow g_GetInt;
cInterfaceWindow g_LoadGame;
cInterfaceWindow g_Gallery;
cInterfaceWindow g_TransferGirls;
cScreenGirlManagement g_GirlManagement;
cScreenGangs g_GangManagement;
cScreenGirlDetails g_GirlDetails;
cScreenDungeon g_Dungeon;
cScreenSlaveMarket g_SlaveMarket;
cScreenTown g_TownScreen;
cScreenBuildingSetup g_BuildingSetupScreen;
cScreenMayor g_MayorsOfficeScreen;
cScreenBank g_BankScreen;
cScreenHouse g_PlayersHouse;
cScreenItemManagement g_ItemManagement;
cScreenPrison g_PrisonScreen;
cBuildingManagement g_BuildingManagementScreen;
cMessageBox g_MessageBox;
cChoiceManager g_ChoiceManager;
cMessageQue g_MessageQue;
cTriggerList g_GlobalTriggers;
GirlManager g_Girls;


cAbstractGirls* g_GirlsPtr = &g_Girls;
cScrollBar* g_DragScrollBar = nullptr;
int g_CurrBrothel = 0;
unsigned int g_GameFlags[NUM_GAMEFLAGS][2];
bool g_Cheats = false;
bool g_WalkAround = false;	// for keeping track of weather have walked around town today
bool g_AllTogle = false;	// used on screens when wishing to apply something to all items
std::string g_ReturnText = "";
bool g_LeftArrow = false;
bool g_RightArrow = false;
bool g_UpArrow = false;
bool g_DownArrow = false;
bool g_EnterKey = false;
bool g_InitWin = true;
long g_IntReturn;
bool eventrunning = false;
int g_TalkCount = 10;
bool g_GenGirls = false;
Girl* selected_girl;  // global pointer for the currently selected girl
std::vector<int> cycle_girls;  // globally available sorted list of girl IDs for Girl Details screen to cycle through
int cycle_pos;  //currently selected girl's position in the cycle_girls vector
char buffer[1000];
cSlider* g_DragSlider = nullptr;
std::shared_ptr<CSurface> g_BackgroundImage = nullptr;
std::shared_ptr<CSurface> g_BrothelImages[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
Girl* MarketSlaveGirls[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
int MarketSlaveGirlsDel[8] = {-1,-1,-1,-1,-1,-1,-1,-1};


void handle_hotkeys()
{
	if(vent.key.keysym.sym == SDLK_RSHIFT || vent.key.keysym.sym == SDLK_LSHIFT)	// enable multi select
		g_ShiftDown = true;
	else if(vent.key.keysym.sym == SDLK_RCTRL || vent.key.keysym.sym == SDLK_LCTRL)	// enable multi select
		g_CTRLDown = true;

	if(vent.key.keysym.sym == SDLK_UP)
		g_UpArrow = true;
	else if(vent.key.keysym.sym == SDLK_DOWN)
		g_DownArrow = true;
	else if(vent.key.keysym.sym == SDLK_LEFT)
		g_LeftArrow = true;
	else if(vent.key.keysym.sym == SDLK_RIGHT)
		g_RightArrow = true;

	if(g_WinManager.GetWindow() != &g_MainMenu)
	{
		int br_no = 0;

		switch(vent.key.keysym.sym) {
		case SDLK_1: case SDLK_2: case SDLK_3:
		case SDLK_4: case SDLK_5: case SDLK_6:
			br_no = vent.key.keysym.sym - SDLK_1;
			if(g_Brothels.GetNumBrothels() > br_no) {
				g_CurrBrothel = br_no;
				g_InitWin = true;
			}
			break;
		case SDLK_d:
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Dungeon");
			break;
		case SDLK_s:	// slave market screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Town");
			g_WinManager.push("Slave Market");
			break;
		case SDLK_m:	// mayors office screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
//
//			this will make "m" go to brothel management
//			you need "M" to go to the mayor screen now
//			which is far less used, I think, and easy to get
//			to from the town screen
//
//			we should consider some customisable keyboard mapping
//			mechanism at some point
//
			if(g_ShiftDown) {
				g_WinManager.push("Town");
				g_WinManager.push("Mayor");
			}
			break;
		case SDLK_b:	// bank screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Town");
			g_WinManager.push("Bank");
			break;
		case SDLK_g:	// girl management screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Girl Management");
			break;
		case SDLK_t:	// staff management screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Gangs");
			break;
		case SDLK_u:	// upgrades management screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			if(g_ShiftDown) {
				g_WinManager.push("Building Management");
			}
			else {
				g_WinManager.push("Building Setup");
			}
			break;
		case SDLK_a:	// turn summary screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.Push(Turnsummary, &g_Turnsummary);
			break;
		case SDLK_o:	// town screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Town");
			break;
		case SDLK_p:	// shop screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Town");
			g_WinManager.push("Item Management");
			break;
		default:
			// do nothing, but the "default" clause silences an irritating warning
			break;
		}
	}
}



int main_old(int ac, char* av[])
{
	#ifndef LINUX
		#ifdef _DEBUG
			//_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
			//_CrtSetBreakAlloc(16477);
		#endif
	#endif

	cConfig cfg;
	bool running = true;
	bool mouseDown = false;

	g_LogFile.ss()
		<< "Startup: checking for variant config: argc = " << ac << ".\n";
	if(ac > 1) {
		g_LogFile.ss() << "	attempting to load '" << av[1] << "'\n";
		cfg.reload(av[1]);
	}

	g_LogFile.write("calling init");
	// INit the program
	if(!Init())
		return 1;

	g_WinManager.Push(MainMenu, &g_MainMenu);
    
    running = false;
    
	while(running)
	{
//        while(SDL_PollEvent(&vent))
//		{
//			if(vent.type == SDL_QUIT)
//				running = false;
//			else if(vent.type == SDL_MOUSEBUTTONUP)
//			{
//				if(mouseDown == true)
//				{
//					if (g_DragScrollBar != nullptr)
//					{
//						g_DragScrollBar->SetTopValue(g_DragScrollBar->m_ItemTop);
//						g_DragScrollBar = nullptr;
//					}
//					else if(g_DragSlider != nullptr)
//					{
//						g_DragSlider->EndDrag();
//						g_DragSlider = nullptr;
//					}
//					else if(g_MessageBox.IsActive())
//						g_MessageBox.Advance();
//					else if(g_ChoiceManager.IsActive())
//						g_ChoiceManager.ButtonClicked(vent.motion.x, vent.motion.y);
//					else
//						g_WinManager.UpdateMouseClick(vent.motion.x, vent.motion.y);
//					mouseDown = false;
//				}
//			}
//			else if(vent.type == SDL_MOUSEBUTTONDOWN)
//			{
//				if(vent.button.button == SDL_BUTTON_WHEELDOWN)
//				{
//					g_WinManager.UpdateMouseClick(vent.motion.x, vent.motion.y, true, false);
//				}
//				else if(vent.button.button == SDL_BUTTON_WHEELUP)
//				{
//					g_WinManager.UpdateMouseClick(vent.motion.x, vent.motion.y, false, true);
//				}
//				else if(vent.button.button == SDL_BUTTON_LEFT)
//				{
//					//srand(SDL_GetTicks());
//					if(mouseDown == false)
//						mouseDown = true;
//					g_WinManager.UpdateMouseDown(vent.motion.x, vent.motion.y);
//				}
////
////				horizontal mouse scroll events happen here,
////              as do right and middle clicks.
////
//				else {
//					// do nothing ...
//				}
//			}
//			else if(vent.type == SDL_KEYUP)
//			{
//				if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
//				{
//					if(vent.key.keysym.sym == SDLK_RSHIFT || vent.key.keysym.sym == SDLK_LSHIFT)	// enable multi select
//						g_ShiftDown = false;
//					else if(vent.key.keysym.sym == SDLK_RCTRL || vent.key.keysym.sym == SDLK_LCTRL)	// enable multi select
//						g_CTRLDown = false;
//
//					if(vent.key.keysym.sym == SDLK_UP)
//						g_UpArrow = false;
//					else if(vent.key.keysym.sym == SDLK_DOWN)
//						g_DownArrow = false;
//					else if(vent.key.keysym.sym == SDLK_LEFT)
//						g_LeftArrow = false;
//					else if(vent.key.keysym.sym == SDLK_RIGHT)
//						g_RightArrow = false;
//				}
//			}
//			else if(vent.type == SDL_KEYDOWN)
//			{
//				if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
//				{
//					if(g_WinManager.HasEditBox())
//					{
//						if(vent.key.keysym.sym == SDLK_BACKSPACE)
//							g_WinManager.UpdateKeyInput('-');
//						else if(vent.key.keysym.sym == SDLK_RETURN)
//							g_EnterKey = true;
//						else if((vent.key.keysym.sym >= 97 && vent.key.keysym.sym <= 122) || vent.key.keysym.sym == 39 || vent.key.keysym.sym == 32 || (vent.key.keysym.sym >= 48 && vent.key.keysym.sym <= 57) || ((vent.key.keysym.sym >= 256 && vent.key.keysym.sym <= 265)))
//						{
//							if(vent.key.keysym.sym >= 256)
//							{
//								if(vent.key.keysym.sym == 256)
//									vent.key.keysym.sym = SDLK_0;
//								else if(vent.key.keysym.sym == 257)
//									vent.key.keysym.sym = SDLK_1;
//								else if(vent.key.keysym.sym == 258)
//									vent.key.keysym.sym = SDLK_2;
//								else if(vent.key.keysym.sym == 259)
//									vent.key.keysym.sym = SDLK_3;
//								else if(vent.key.keysym.sym == 260)
//									vent.key.keysym.sym = SDLK_4;
//								else if(vent.key.keysym.sym == 261)
//									vent.key.keysym.sym = SDLK_5;
//								else if(vent.key.keysym.sym == 262)
//									vent.key.keysym.sym = SDLK_6;
//								else if(vent.key.keysym.sym == 263)
//									vent.key.keysym.sym = SDLK_7;
//								else if(vent.key.keysym.sym == 264)
//									vent.key.keysym.sym = SDLK_8;
//								else if(vent.key.keysym.sym == 265)
//									vent.key.keysym.sym = SDLK_9;
//							}
//
//							if(vent.key.keysym.mod & KMOD_LSHIFT || vent.key.keysym.mod & KMOD_RSHIFT || vent.key.keysym.mod & KMOD_CAPS)
//								g_WinManager.UpdateKeyInput( static_cast<char>( vent.key.keysym.sym ), true);
//							else
//								g_WinManager.UpdateKeyInput( static_cast<char>( vent.key.keysym.sym ) );
//						}
//					}
//					else	// hotkeys
//					{
//						handle_hotkeys();
//					}
//				}
//			}
//			else if(vent.type == SDL_MOUSEMOTION)
//			{
//				if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
//				{
//					// if dragging a scrollbar, send movements to it exclusively until mouseup
//					if (g_DragScrollBar != nullptr)
//						g_DragScrollBar->DragMove(vent.motion.y);
//					// if dragging a slider, send movements to it exclusively until mouseup
//					else if(g_DragSlider != nullptr)
//						g_DragSlider->DragMove(vent.motion.x);
//					// update interface
//					else
//						g_WinManager.UpdateMouseMovement(vent.motion.x, vent.motion.y);
//				}
//				else
//					g_ChoiceManager.IsOver(vent.motion.x, vent.motion.y);
//			}
//		}
//
//		//		if(!sleeping)
////		{
//			// Clear the screen
//			g_Graphics.Begin();
//
//			// draw the background image
//			SDL_Rect clip;
//			clip.x = 0;
//			clip.y = 0;
//			clip.w = g_ScreenWidth;
//			clip.h = g_ScreenHeight;
//			g_BackgroundImage->DrawSurface(clip.x,clip.y,nullptr,&clip,true);
//
//			// Draw the interface
//			g_WinManager.Draw();
//
//			if(!g_MessageBox.IsActive() && g_MessageQue.HasNext())
//				g_MessageQue.ActivateNext();
//
//			if(eventrunning && !g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())	// run any events that are being run
//				GameEvents();
//
//			// Run the interface
//			if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
//				g_WinManager.UpdateCurrent();
//
//			// Draw Any message boxes
//			if(g_MessageBox.IsActive())
//			{
//				g_MessageBox.Draw();
//			}
//			else if(g_ChoiceManager.IsActive() && !g_MessageQue.HasNext())
//				g_ChoiceManager.Draw();
//
//			g_Graphics.End();
//		}
//		else
//			SDL_Delay(1000);
	}

	Shutdown();
	return 0;
}

void Shutdown()
{
	g_LogFile.write("Shutting Down");
	g_Graphics.Free();

	g_BackgroundImage.reset();

	for(int i=0; i<6; i++)
	{
		if(g_BrothelImages[i])
		{
			g_BrothelImages[i].reset();
		}
	}

	for(int i=0; i<8; i++)
	{
		if(MarketSlaveGirls[i] && MarketSlaveGirlsDel[i] == -1)
			delete MarketSlaveGirls[i];
		MarketSlaveGirls[i] = nullptr;
	}

	g_Brothels.Free();
	g_Customers.Free();
	g_Girls.Free();
	g_Traits.Free();
	g_InvManager.Free();

	FreeInterface();
    
    cGetStringScreenManager::ReleaseResources();
    
	#ifdef _DEBUG
	cJobManager::freeJobs();
	#else
	cJobManager::free();
	#endif
}

bool Init()
{
	g_LogFile.write("Initializing Graphics");
//
//	build the caption string
//
	std::stringstream ss;
	ss << "Whore Master: Renewal v"
	   << g_MajorVersion
	   << "."
	   << g_MinorVersion
	   << "."
	   << g_PatchVersion
	   << g_MetadataVersion;
	;
//
//  init the graphics, with the caption on the titlebar
//
	if(!g_Graphics.InitGraphics(ss.str(), 0,0,32))
	{
		g_LogFile.write("ERROR - Initializing Graphics");
		return false;
	}

	g_LogFile.write("Graphics Initialized");
	// Load the universal background image
	g_BackgroundImage.reset( new ImageSurface("background", "") );
	g_LogFile.write("Background Image Set");

	LoadInterface();	// Load the interface
	g_LogFile.write("Interface Loaded");

	InitGameFlags();	// Init the game flags
	g_LogFile.write("Game Flags Initialized");

	// Load the brothel images
	for(int i=0; i<6; i++)
	{
		if(g_BrothelImages[i])
		{
			g_BrothelImages[i].reset();
		}
//
//      I think this should work - kept the old line below
//      reference
//
		/*char buffer[32];*/
		g_BrothelImages[i].reset( new ImageSurface("Brothel", toString(i).c_str()) );
		//g_BrothelImages[i]->LoadImage(file,false);
	}
	g_LogFile.write("Brothel Images Set");

	return true;
}

} // namespace WhoreMasterRenewal



namespace wmr = WhoreMasterRenewal;

int main( int argc, char* argv[] )
{
    wmr::Logger() << "Loading Whore Master: Renewal...\n";
    
    try
    {
        wmr::main_old( argc, argv );
    }
    catch( std::exception& ex )
    {
        wmr::Logger() << "Exception caught:\nException type: \"" << typeid(ex).name() << "\"\n" << ex.what() << "\n";
    }
    catch( ... )
    {
        wmr::Logger() << "Unknown exception caught...\n";
    }
    
    wmr::Logger() << "Whore Master: Renewal finished...\n";
    
    return 0;
}
