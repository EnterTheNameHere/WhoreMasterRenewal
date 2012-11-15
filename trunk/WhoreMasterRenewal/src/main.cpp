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
#include "InterfaceGlobals.h"
#include "GameFlags.h"
#include "InterfaceProcesses.h"
#include "sConfig.h"
#include "CSurface.h"
#include "cJobManager.h"
#include "Revision.h"
#ifndef LINUX
	#ifdef _DEBUG
// to enable leak detection uncomment the below and the first comment in main()
/*		#ifndef _CRTDBG_MAP_ALLOC
			#define _CRTDBG_MAP_ALLOC
		#endif
		#include <stdlib.h>
		#include <crtdbg.h>*/
	#endif
#else
	#include "linux.h"
#endif
#include <signal.h>
#include <sstream>

#include "IconSurface.h"
#include <string>

extern int g_ScreenWidth, g_ScreenHeight;
extern bool g_Fullscreen;
sInterfaceIDs g_interfaceid;

// Function Defs

void Shutdown();
bool Init();

int g_CurrBrothel = 0;
unsigned int g_LastSeed = 0;		// for seeding the random number generater every 3 seconds (3000 ticks)
bool eventrunning = false;
bool newWeek = false;

bool g_ShiftDown = false;
bool g_CTRLDown = false;

bool g_LeftArrow = false;
bool g_RightArrow = false;
bool g_UpArrow = false;
bool g_DownArrow = false;

bool g_EnterKey = false;

cScrollBar* g_DragScrollBar = 0;  // if a scrollbar is being dragged, this points to it
cSlider* g_DragSlider = 0;  // if a slider is being dragged, this points to it

extern CSurface* g_BrothelImages[6];
extern bool g_InitWin;
extern sGirl* MarketSlaveGirls[8];
extern int MarketSlaveGirlsDel[8];

// SDL Graphics interface
CGraphics g_Graphics;

// Resource Manager
CResourceManager rmanager;

// Events
SDL_Event vent;

// logfile
CLog g_LogFile(true);

// Trait list
cTraits g_Traits;

// Girl manager
cGirls g_Girls;
cAbstractGirls *g_GirlsPtr = &g_Girls;

// Brothel Manager
cBrothelManager g_Brothels;

// Gang Manager
cGangManager g_Gangs;

// Customer Manager
cCustomers g_Customers;

// the background image
CSurface* g_BackgroundImage = 0;

// The global trigger manager
cTriggerList g_GlobalTriggers;

// Holds the currently running script

cWindowManager g_WinManager;

// Keeping time in the game
unsigned long g_Year;
unsigned long g_Month;
unsigned long g_Day;

// the players gold
cGold g_Gold;

// Inventory manager
cInventory g_InvManager;

// TEmporary testing crap
int IDS = 0;
cRng g_Dice;

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
/*
 *			this will make "m" go to brothel management
 *			you need "M" to go to the mayor screen now
 *			which is far less used, I think, and easy to get
 *			to from the town screen
 *
 *			we should consider some customisable keyboard mapping
 *			mechanism at some point
 */
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

	CLog log = log;
	log.write("calling init");
	// INit the program
	if(!Init())
		return 1;

	g_WinManager.Push(MainMenu, &g_MainMenu);

	while(running)
	{
        while(SDL_PollEvent(&vent))
		{
			if(vent.type == SDL_QUIT)
				running = false;
			else if(vent.type == SDL_MOUSEBUTTONUP)
			{
				if(mouseDown == true)
				{
					if (g_DragScrollBar != 0)
					{
						g_DragScrollBar->SetTopValue(g_DragScrollBar->m_ItemTop);
						g_DragScrollBar = 0;
					}
					else if(g_DragSlider != 0)
					{
						g_DragSlider->EndDrag();
						g_DragSlider = 0;
					}
					else if(g_MessageBox.IsActive())
						g_MessageBox.Advance();
					else if(g_ChoiceManager.IsActive())
						g_ChoiceManager.ButtonClicked(vent.motion.x, vent.motion.y);
					else
						g_WinManager.UpdateMouseClick(vent.motion.x, vent.motion.y);
					mouseDown = false;
				}
			}
			else if(vent.type == SDL_MOUSEBUTTONDOWN)
			{
				if(vent.button.button == SDL_BUTTON_WHEELDOWN)
				{
					g_WinManager.UpdateMouseClick(vent.motion.x, vent.motion.y, true, false);
				}
				else if(vent.button.button == SDL_BUTTON_WHEELUP)
				{
					g_WinManager.UpdateMouseClick(vent.motion.x, vent.motion.y, false, true);
				}
				else if(vent.button.button == SDL_BUTTON_LEFT)
				{
					//srand(SDL_GetTicks());
					if(mouseDown == false)
						mouseDown = true;
					g_WinManager.UpdateMouseDown(vent.motion.x, vent.motion.y);
				}
/*
 *				horizontal mouse scroll events happen here,
 *				as do right and middle clicks.
 */
				else {
					// do nothing ...
				}
			}
			else if(vent.type == SDL_KEYUP)
			{
				if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
				{
					if(vent.key.keysym.sym == SDLK_RSHIFT || vent.key.keysym.sym == SDLK_LSHIFT)	// enable multi select
						g_ShiftDown = false;
					else if(vent.key.keysym.sym == SDLK_RCTRL || vent.key.keysym.sym == SDLK_LCTRL)	// enable multi select
						g_CTRLDown = false;

					if(vent.key.keysym.sym == SDLK_UP)
						g_UpArrow = false;
					else if(vent.key.keysym.sym == SDLK_DOWN)
						g_DownArrow = false;
					else if(vent.key.keysym.sym == SDLK_LEFT)
						g_LeftArrow = false;
					else if(vent.key.keysym.sym == SDLK_RIGHT)
						g_RightArrow = false;
				}
			}
			else if(vent.type == SDL_KEYDOWN)
			{
				if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
				{
					if(g_WinManager.HasEditBox())
					{
						if(vent.key.keysym.sym == SDLK_BACKSPACE)
							g_WinManager.UpdateKeyInput('-');
						else if(vent.key.keysym.sym == SDLK_RETURN)
							g_EnterKey = true;
						else if((vent.key.keysym.sym >= 97 && vent.key.keysym.sym <= 122) || vent.key.keysym.sym == 39 || vent.key.keysym.sym == 32 || (vent.key.keysym.sym >= 48 && vent.key.keysym.sym <= 57) || ((vent.key.keysym.sym >= 256 && vent.key.keysym.sym <= 265)))
						{
							if(vent.key.keysym.sym >= 256)
							{
								if(vent.key.keysym.sym == 256)
									vent.key.keysym.sym = SDLK_0;
								else if(vent.key.keysym.sym == 257)
									vent.key.keysym.sym = SDLK_1;
								else if(vent.key.keysym.sym == 258)
									vent.key.keysym.sym = SDLK_2;
								else if(vent.key.keysym.sym == 259)
									vent.key.keysym.sym = SDLK_3;
								else if(vent.key.keysym.sym == 260)
									vent.key.keysym.sym = SDLK_4;
								else if(vent.key.keysym.sym == 261)
									vent.key.keysym.sym = SDLK_5;
								else if(vent.key.keysym.sym == 262)
									vent.key.keysym.sym = SDLK_6;
								else if(vent.key.keysym.sym == 263)
									vent.key.keysym.sym = SDLK_7;
								else if(vent.key.keysym.sym == 264)
									vent.key.keysym.sym = SDLK_8;
								else if(vent.key.keysym.sym == 265)
									vent.key.keysym.sym = SDLK_9;
							}

							if(vent.key.keysym.mod & KMOD_LSHIFT || vent.key.keysym.mod & KMOD_RSHIFT || vent.key.keysym.mod & KMOD_CAPS)
								g_WinManager.UpdateKeyInput((char)vent.key.keysym.sym, true);
							else
								g_WinManager.UpdateKeyInput((char)vent.key.keysym.sym);
						}
					}
					else	// hotkeys
					{
						handle_hotkeys();
					}
				}
			}
			else if(vent.type == SDL_MOUSEMOTION)
			{
				if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
				{
					// if dragging a scrollbar, send movements to it exclusively until mouseup
					if (g_DragScrollBar != 0)
						g_DragScrollBar->DragMove(vent.motion.y);
					// if dragging a slider, send movements to it exclusively until mouseup
					else if(g_DragSlider != 0)
						g_DragSlider->DragMove(vent.motion.x);
					// update interface
					else
						g_WinManager.UpdateMouseMovement(vent.motion.x, vent.motion.y);
				}
				else
					g_ChoiceManager.IsOver(vent.motion.x, vent.motion.y);
			}
		}

		//		if(!sleeping)
//		{
			// Clear the screen
			g_Graphics.Begin();

			// draw the background image
			SDL_Rect clip;
			clip.x = 0;
			clip.y = 0;
			clip.w = g_ScreenWidth;
			clip.h = g_ScreenHeight;
			g_BackgroundImage->DrawSurface(clip.x,clip.y,0,&clip,true);

			// Draw the interface
			g_WinManager.Draw();

			if(!g_MessageBox.IsActive() && g_MessageQue.HasNext())
				g_MessageQue.ActivateNext();

			if(eventrunning && !g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())	// run any events that are being run
				GameEvents();

			// Run the interface
			if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
				g_WinManager.UpdateCurrent();

			// Draw Any message boxes
			if(g_MessageBox.IsActive())
			{
				g_MessageBox.Draw();
			}
			else if(g_ChoiceManager.IsActive() && !g_MessageQue.HasNext())
				g_ChoiceManager.Draw();

			rmanager.CullOld(g_Graphics.GetTicks());

			g_Graphics.End();
/*		}
		else
			SDL_Delay(1000);
*/	}

	Shutdown();
	return 0;
}

void Shutdown()
{
	g_LogFile.write("Shutting Down");
	g_Graphics.Free();

	delete g_BackgroundImage;

	for(int i=0; i<6; i++)
	{
		if(g_BrothelImages[i])
		{
			delete g_BrothelImages[i];
			g_BrothelImages[i] = 0;
		}
	}

	for(int i=0; i<8; i++)
	{
		if(MarketSlaveGirls[i] && MarketSlaveGirlsDel[i] == -1)
			delete MarketSlaveGirls[i];
		MarketSlaveGirls[i] = 0;
	}

	g_Brothels.Free();
	g_Customers.Free();
	g_Girls.Free();
	g_Traits.Free();
	g_InvManager.Free();

	FreeInterface();

	rmanager.Free();
	#ifdef _DEBUG
	cJobManager::freeJobs();
	#else
	cJobManager::free();
	#endif
}

bool Init()
{
	g_LogFile.write("Initializing Graphics");
/*
 *	build the caption string
 */
	stringstream ss;
	ss << "Whore Master v"
	   << g_MajorVersion
	   << "."
	   << g_MinorVersionA
	   << g_MinorVersionB
	   << "."
	   << g_StableVersion
	   << " BETA"
	   << " Svn: " << svn_revision
	;
/*
 *	init the graphics, with the caption on the titlebar
 */
	if(!g_Graphics.InitGraphics(ss.str(), 0,0,32))
	{
		g_LogFile.write("ERROR - Initializing Graphics");
		return false;
	}

	g_LogFile.write("Graphics Initialized");
	// Load the universal background image
	g_BackgroundImage = new ImageSurface("background", "");
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
			delete g_BrothelImages[i];
			g_BrothelImages[i] = 0;
		}
/*
 *		I think this should work - kept the old line below
 *		reference
 */
		char buffer[32];
		g_BrothelImages[i] = new ImageSurface("Brothel", toString(i).c_str());
		//g_BrothelImages[i]->LoadImage(file,false);
	}
	g_LogFile.write("Brothel Images Set");

	return true;
}

// trivial change to test Revision.h



#include "WhoreMasterRenewalWindow.hpp"

#include "LuaUtils.hpp"

namespace WhoreMasterRenewal
{

class Condition
{
    Condition() = delete;
    Condition( const Condition& ) = delete;
    Condition( const Condition&& ) = delete;
    Condition& operator= ( const Condition& ) = delete;
    virtual ~Condition();
};

class That : Condition
{
    That() = delete;
    virtual ~That();
    
    bool Equals();
};

class Assert
{
public:
    Assert() = delete;
    Assert( const Assert& ) = delete;
    Assert( const Assert&& ) = delete;
    Assert& operator= ( const Assert& ) = delete;
    ~Assert();
    
    template<typename T>
    void That( T& value, Condition condition )
    {
        ;
    }
    /*
    template<typename T, typename U>
    static void Equals( T& value1, U& value2 )
    {
        if( value1 != value2 )
        {
            std::cout << "Warning: Assert.Equals expected \"" << value2 << "\", but \"" << value1 << "\" was given.\n";
        }
    }
    */
    
    template<typename T>
    static void Equals( T value1, T value2, T precission = 0.001, typename std::enable_if< std::is_floating_point<T>::value, T >::type* = 0 )
    {
        if( (value2 - precission) < value1 && (value2 + precission) > value1 )
        {
            std::cout << "Debug: " << (value2 - precission) << " < " << value1 << " < " << (value2 + precission) << "\n";
            std::cout << "Warning: Assert.Equals expected \"" << value2 << "\" (precission=\"" << precission << "\"), but \"" << value1 << "\" was given.\n";
        }
    }
    
    template<typename T, typename U>
    static void Equals( T value1, U value2 )
    {
        if( value1 != value2 )
        {
            std::cout << "Warning: Assert.Equals expected \"" << value2 << "\", but \"" << value1 << "\" was given.\n";
        }
    }
};

}

using namespace WhoreMasterRenewal;

static int average( lua_State* L )
{
    luaD_DumpStack( L );
    
    int n = lua_gettop( L );
    double sum = 0;
    
    for( int i = 1; i <= n; i++ )
    {
        if( !lua_isnumber( L, i ) )
        {
            lua_pushstring( L, "Incorrect argument to 'average'" );
            lua_error( L );
        }
        
        sum += lua_tonumber( L, i );
    }
    
    lua_pushnumber( L, sum / n );
    lua_pushnumber( L, sum );
    
    luaD_DumpStack( L );
    
    return 2;
}

int main( int argc, char* argv[] )
{
    try
    {
        /*
        std::clog << "main()\n";
        
        WhoreMasterRenewalWindow window;
        
        std::clog << "main() : pre window.Run()\n";
        
        window.Run();
        
        std::clog << "main() : post window.Run()\n";
        */
        
        
        LuaRuntime lua( true );
        
        lua.RegisterFunction( "average", &average );
        
        lua.SetVariable( "answerToEverything", 42 );
        lua.SetVariable( "temperature", 36.8 );
        lua.SetVariable( "name", "EnterTheNameHere" );
        lua.SetVariable( "running", true );
        lua.SetVariable( "character.health", 100.0 );
        
        
        lua.ExecuteString( "function add( first, second ) return first + second end" );
        lua.ExecuteString( "temperature = temperature + 0.7" );
        lua.ExecuteString( "character.health = character.health + answerToEverything" );
        lua.ExecuteString( "name = name .. \" Galicia\"" );
        lua.ExecuteString( "avg, sum = average( 10, 12, 17, 3 )" );
        lua.ExecuteString( "running = false" );
        lua.ExecuteString( "print( \"avg:\", avg ) print( \"sum:\", avg )" );
        lua.ExecuteString( "print( \"avg:\", avg )" );
        lua.ExecuteString( "print( \"sum:\", sum )" );
        lua.ExecuteString( "result = add( 14.2, 2 )" );
        
        Assert::Equals( lua.GetVariable<int>( "notExisting" ), 0 );
        Assert::Equals( lua.GetVariable<int>( "answerToEverything" ), 42 );
        Assert::Equals( lua.GetVariable<bool>( "running" ), false );
        Assert::Equals( lua.GetVariable<string>( "name" ), "EnterTheNameHere Galicia" );
        Assert::Equals( lua.GetVariable<double>( "temperature" ), 37.5 );
        Assert::Equals( lua.GetVariable<float>( "character.health" ), 100.0f );
        Assert::Equals( lua.GetVariable<float>( "result" ), 16.2f );
        Assert::Equals( lua.GetVariable<int>( "sum" ), lua.GetVariable<int>( "answerToEverything" ) );
        Assert::Equals( lua.GetVariable<double>( "avg" ), 10.5 );
        std::cout << "=== 34.5, 34.5 ===\n";
        Assert::Equals( 34.5, 34.5 );
        std::cout << "=== 34.6, 34.5 ===\n";
        Assert::Equals( 34.6, 34.5 );
        std::cout << "=== 34.5, -34.5 ===\n";
        Assert::Equals( 34.5, -34.5 );
        std::cout << "=== 34.5454, 34.5445 ===\n";
        Assert::Equals( 34.5454, 34.5445 );
        std::cout << "=== 34.5f, 34.5f ===\n";
        Assert::Equals( 34.5f, 34.5f );
        std::cout << "=== 34.6f, 34.5f ===\n";
        Assert::Equals( 34.6f, 34.5f );
        std::cout << "=== 34.5f, -34.5f ===\n";
        Assert::Equals( 34.5f, -34.5f );
        std::cout << "=== 34.5454f, 34.5445f ===\n";
        Assert::Equals( 34.5454f, 34.5445f );
        
        /*
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);
        
        lua_register( L, "average", average );
        
        if( luaL_dofile( L, "Resources/Scripts/TestScript.lua" ) )
        {
            std::cout << "Error while executing script:\n" << lua_tostring( L, -1 ) << "\n";
        }
        else
        {
            luaD_DumpStack( L );
        
            lua_getglobal( L, "add" );
            lua_pushnumber( L, 3 );
            lua_pushnumber( L, 4 );
            
            luaD_DumpStack( L );
            
            lua_call( L, 2, 1 );
            
            luaD_DumpStack( L );
            
            std::cout << "add( 3, 4 ) = " << lua_tonumber( L, -1 ) << "\n";
            
            lua_pop( L, -1 );
            
            luaD_DumpStack( L );
            
            lua_getglobal( L, "add" );
            lua_pushnumber( L, 3.f );
            lua_pushnumber( L, 15.33f );
            
            luaD_DumpStack( L );
            
            lua_call( L, 2, 1 );
            
            luaD_DumpStack( L );
            
            std::cout << "add( 3, 4 ) = " << lua_tonumber( L, -1 ) << "\n";
            
            lua_pop( L, -1 );
            
            luaD_DumpStack( L );
        };
        
        lua_close(L);
        
        */
    }
    catch( exception& ex )
    {
        std::clog << "Exception caught:\nException type: \"" << typeid(ex).name() << "\"\n" << ex.what();
    }
    return 0;
}
