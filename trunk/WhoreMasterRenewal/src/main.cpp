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
#include "Helper.hpp"
#include "GameFlags.h"
#include "InterfaceGlobals.h"
#include "InterfaceProcesses.h"
#include "sConfig.h"
#include "CSurface.h"
#include "cJobManager.h"
#include "Revision.h"
#include "cWindowManager.h"
#include "cBrothel.h"
#include "cGangs.h"

#ifndef LINUX
	#ifdef _DEBUG
// to enable leak detection uncomment the below and the first comment in main()
//		#ifndef _CRTDBG_MAP_ALLOC
//			#define _CRTDBG_MAP_ALLOC
//		#endif
//		#include <stdlib.h>
//		#include <crtdbg.h>
	#endif
#else
	#include "linux.h"
#endif
#include <signal.h>
#include <sstream>

#include "IconSurface.h"
#include <string>

// Function Defs
void Shutdown();
bool Init();

// Events
SDL_Event vent;
extern CResourceManager rmanager;

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
//
//				horizontal mouse scroll events happen here,
//              as do right and middle clicks.
//
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
//
//	build the caption string
//
	std::stringstream ss;
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
//
//      I think this should work - kept the old line below
//      reference
//
		/*char buffer[32];*/
		g_BrothelImages[i] = new ImageSurface("Brothel", toString(i).c_str());
		//g_BrothelImages[i]->LoadImage(file,false);
	}
	g_LogFile.write("Brothel Images Set");

	return true;
}

// trivial change to test Revision.h



#include <iostream>
#include <iomanip>
#include <sstream>

#include <SFML/Graphics.hpp>
#include <Rocket/Core.h>
#include <Rocket/Controls.h>
#include <Rocket/Debugger/Debugger.h>
#include <Rocket/Controls/DataSource.h>
#include <Rocket/Core/Types.h>

#include "libRocketSFMLInterface/RenderInterfaceSFML.h"
#include "libRocketSFMLInterface/SystemInterfaceSFML.h"
#include "libRocketSFMLInterface/ShellFileInterface.h"

#include "../../../../../../../DeveloperTools/Cpp/libRocket/Source/Core/StyleSheetFactory.h"

//#include "WhoreMasterRenewalWindow.hpp"
//#include "LuaUtils.hpp"

#include "CLog.h"

#include <map>
#include <vector>

namespace wmr = WhoreMasterRenewal;

Rocket::Core::Context* context = nullptr;

namespace WhoreMasterRenewal
{
    class Helper
    {
    public:
        static std::string DumpRocketEvent( Rocket::Core::Event& event )
        {
            std::stringstream debugText;
            
            debugText << event.GetTargetElement()->GetTagName().CString() <<
                "[" << event.GetTargetElement()->GetId().CString() << "]: "
                << event.GetType().CString() << "\nParameters: " << event.GetParameters()->Size() << "\n";
            
            Rocket::Core::Variant* val;
            Rocket::Core::String key;
            int pos = 0;
            while( event.GetParameters()->Iterate( pos, key, val ) )
            {
                debugText << "[" << pos << "]" << key.CString() << ": ";
                switch( val->GetType() )
                {
                case Rocket::Core::Variant::Type::BYTE:
                    debugText << val->Get<Rocket::Core::byte>();
                    break;
                case Rocket::Core::Variant::Type::CHAR:
                    debugText << val->Get<char>();
                    break;
                case Rocket::Core::Variant::Type::COLOURB:
                    debugText << "RGBA=";
                    debugText << val->Get<Rocket::Core::Colourb>().red << ";";
                    debugText << val->Get<Rocket::Core::Colourb>().green << ";";
                    debugText << val->Get<Rocket::Core::Colourb>().blue << ";";
                    debugText << val->Get<Rocket::Core::Colourb>().alpha;
                    break;
                case Rocket::Core::Variant::Type::COLOURF:
                    debugText << "RGBA=";
                    debugText << val->Get<Rocket::Core::Colourf>().red << ";";
                    debugText << val->Get<Rocket::Core::Colourf>().green << ";";
                    debugText << val->Get<Rocket::Core::Colourf>().blue << ";";
                    debugText << val->Get<Rocket::Core::Colourf>().alpha;
                    break;
                case Rocket::Core::Variant::Type::FLOAT:
                    debugText << val->Get<float>();
                    break;
                case Rocket::Core::Variant::Type::INT:
                    debugText << val->Get<int>();
                    break;
                case Rocket::Core::Variant::Type::NONE:
                    debugText << "NONE";
                    break;
                case Rocket::Core::Variant::Type::SCRIPTINTERFACE:
                    debugText << "SCRIPTINTERFACE";
                    break;
                case Rocket::Core::Variant::Type::STRING:
                    debugText << val->Get<Rocket::Core::String>().CString();
                    break;
                case Rocket::Core::Variant::Type::VECTOR2:
                    debugText << "VECTOR2"; // TODO:
                    break;
                case Rocket::Core::Variant::Type::VOIDPTR:
                    debugText << "VOIDPTR";
                    break;
                case Rocket::Core::Variant::Type::WORD:
                    debugText << val->Get<Rocket::Core::word>();
                    break;
                    
                default:
                    debugText << "Unknown";
                    break;
                }
                
                debugText << " ";
            }
            
            debugText << "\n\n";
            
            return debugText.str();
        }
    };
    
    class DebugGirl
    {
    public:
        DebugGirl( std::string name, int age, int looks, int health, int mood, int tired, std::string dayJob, std::string nightJob )
            : m_Name( name ), m_Age( age ), m_Looks( looks ), m_Health( health ), m_Mood( mood ), m_Tired( tired ), m_DayJob( dayJob ), m_NightJob( nightJob )
        {}
        virtual ~DebugGirl() = default;
        
        std::string GetName()
        {
            return m_Name;
        }
        void SetName( std::string name )
        {
            m_Name = name;
        }
        
        int GetAge()
        {
            return m_Age;
        }
        void SetAge( int age )
        {
            m_Age = age;
        }
        
        int GetLooks()
        {
            return m_Looks;
        }
        void SetLooks( int looks )
        {
            m_Looks = looks;
        }
        
        int GetHealth()
        {
            return m_Health;
        }
        void SetHealth( int health )
        {
            m_Health = health;
        }
        
        int GetMood()
        {
            return m_Mood;
        }
        void SetMood( int mood )
        {
            m_Mood = mood;
        }
        
        int GetTired()
        {
            return m_Tired;
        }
        void SetTired( int tired )
        {
            m_Tired = tired;
        }
        
        std::string GetDayJob()
        {
            return m_DayJob;
        }
        void SetDayJob( std::string dayJob )
        {
            m_DayJob = dayJob;
        }
        
        std::string GetNightJob()
        {
            return m_NightJob;
        }
        void SetNightJob( std::string nightJob )
        {
            m_NightJob = nightJob;
        }
        
    private:
        std::string m_Name = {""};
        int m_Age = {0};
        int m_Looks = {0};
        int m_Health = {0};
        int m_Mood = {0};
        int m_Tired = {0};
        
        std::string m_DayJob = {"Free Time"};
        std::string m_NightJob = {"Free Time"};
    };
    
    class Job
    {
    public:
        Job( std::string jobType, std::string jobName ) :
            m_JobType( jobType ), m_JobName( jobName )
        {}
        
        virtual ~Job()
        {}
        
        std::string GetJobType()
        {
            return m_JobType;
        }
        
        void SetJobType( std::string jobType )
        {
            m_JobType = jobType;
        }
        
        std::string GetJobName()
        {
            return m_JobName;
        }
        
        void SetJobName( std::string jobName )
        {
            m_JobName = jobName;
        }
        
    private:
        std::string m_JobType;
        std::string m_JobName;
    };
    
    class DebugGirlsList : public Rocket::Controls::DataSource
    {
    public:
        static void Initialise()
        {
            Logger() << "DebugGirlsList::Initialise().\n";
            
            new DebugGirlsList();
        }
        
        static void Shutdown()
        {
            Logger() << "DebugGirlsList::Shutdown().\n";
            
            delete m_Instance;
        }
        
        virtual void GetRow( Rocket::Core::StringList& row, const Rocket::Core::String& table, int rowIndex, const Rocket::Core::StringList& columns ) override
        {
            Rocket::Core::String columnsList;
            Rocket::Core::StringUtilities::JoinString( columnsList, columns, ',' );
            Logger() << "DebugGirlsList::GetRow()\nTable = \"" << table.CString() << "\", row = " << rowIndex << "\n"
                << "columns = [" << columnsList.CString() << "]\n";
            
            if( table == "girls" )
            {
                for( size_t i = 0; i < columns.size(); i++ )
                {
                    if( columns[i] == "name" )
                    {
                        row.push_back( Rocket::Core::String( m_Girls[rowIndex].GetName().c_str() ) );
                    }
                    else if( columns[i] == "age" )
                    {
                        row.push_back( Rocket::Core::String( 4, "%d", m_Girls[rowIndex].GetAge() ) );
                    }
                    else if( columns[i] == "looks" )
                    {
                        row.push_back( Rocket::Core::String( 4, "%d", m_Girls[rowIndex].GetLooks() ) );
                    }
                    else if( columns[i] == "health" )
                    {
                        row.push_back( Rocket::Core::String( 4, "%d", m_Girls[rowIndex].GetHealth() ) );
                    }
                    else if( columns[i] == "mood" )
                    {
                        row.push_back( Rocket::Core::String( 4, "%d", m_Girls[rowIndex].GetMood() ) );
                    }
                    else if( columns[i] == "tired" )
                    {
                        row.push_back( Rocket::Core::String( 4, "%d", m_Girls[rowIndex].GetTired() ) );
                    }
                    else if( columns[i] == "dayjob" )
                    {
                        row.push_back( Rocket::Core::String( m_Girls[rowIndex].GetDayJob().c_str() ) );
                    }
                    else if( columns[i] == "nightjob" )
                    {
                        row.push_back( Rocket::Core::String( m_Girls[rowIndex].GetNightJob().c_str() ) );
                    }
                }
            }
            else if( table == "jobtypes" || table == "jobnames" )
            {
                for( size_t i = 0; i < columns.size(); i++ )
                {
                    if( columns[i] == "jobtype" )
                    {
                        row.push_back( Rocket::Core::String( m_JobTypes[rowIndex].c_str() ) );
                    }
                    else if( columns[i] == "jobname" )
                    {
                        row.push_back( Rocket::Core::String( m_JobNames[rowIndex].c_str() ) );
                    }
                }
            }
            else if( table == "traits" )
            {
                for( size_t i = 0; i < columns.size(); i++ )
                {
                    if( columns[i] == "traitname" )
                    {
                        row.push_back( Rocket::Core::String( m_Traits[rowIndex].c_str() ) );
                    }
                }
            }
            
            Rocket::Core::String rowValuesList;
            Rocket::Core::StringUtilities::JoinString( rowValuesList, row, ',' );
            
            Logger() << "row values = [" << rowValuesList.CString() << "]\n";
        }
        
        virtual int GetNumRows( const Rocket::Core::String& table ) override
        {
            Logger() << "DebugGirlsList::GetNumRows( " << table.CString() << " ).\n";
            
            if( table == "girls" )
            {
                return m_Girls.size();
            }
            else if( table == "jobtypes" )
            {
                return m_JobTypes.size();
            }
            else if( table == "jobnames" )
            {
                return m_JobNames.size();
            }
            else if( table == "traits" )
            {
                return m_Traits.size();
            }
            else
            {
                return 0;
            }
        }
        
    private:
        DebugGirlsList() : Rocket::Controls::DataSource( "DebugGirlsList" )
        {
            Logger() << "DebugGirlsList::const().\n";
            
            m_Instance = this;
        }
        
        virtual ~DebugGirlsList()
        {
            Logger() << "DebugGirlsList::destr().\n";
            
            m_Instance = nullptr;
        }
        
        static DebugGirlsList* m_Instance;
        
        std::vector<DebugGirl> m_Girls = {
            DebugGirl( "Soifon", 20, 10, 100, 100, 21, "Free Time", "Security" ),
            DebugGirl( "Yoruichi Shihouin", 26, 31, 100, 100, 2, "Security", "Explore Catacombs" ),
            DebugGirl( "Sheryl Nome", 17, 72, 100, 100, 0, "Advertising", "Waitress" )
        };
        
        std::vector<Job> m_Jobs = {
            Job( "General", "Free Time" ),
            Job( "General", "Cleaning" ),
            Job( "General", "Security" ),
            Job( "General", "Advertising" ),
            Job( "General", "Customer Service" ),
            Job( "General", "Matron" ),
            Job( "Brothel", "Whore in Brothel" ),
            Job( "Brothel", "Whore on Streets" ),
            Job( "Brothel", "Stripper in Brothel" ),
            Job( "Brothel", "Masseure in Brothel" ),
            Job( "Gambling Hall", "Whore for Gamblers" ),
            Job( "Gambling Hall", "Game Dealer" ),
            Job( "Gambling Hall", "Entertainer" ),
            Job( "Gambling Hall", "XXX Entertainer" ),
            Job( "Bar", "Barmaid" ),
            Job( "Bar", "Waitress" ),
            Job( "Bar", "Stripper in Bar" ),
            Job( "Bar", "Whore in Bar" ),
            Job( "Bar", "Singer" )
        };
        
        std::vector<std::string> m_JobTypes = {
            std::string( "General" ),
            std::string( "Brothel" ),
            std::string( "Gambling Hall" ),
            std::string( "Bar" )
        };
        
        std::vector<std::string> m_JobNames = {
            "Free Time",
            "Cleaning",
            "Security",
            "Advertising",
            "Customer Service",
            "Matron"
        };
        
        std::vector<std::string> m_Traits =
        {
            "Aggresive",
            "Assassin",
            "Basic Magic",
            "Fleet of Foot",
            "Inhuman Lifespan",
            "Iron Will",
            "Lesbian",
            "Merciless",
            "Small Boobs",
            "Strong",
            "Tough",
            "Tsundere",
            "Normal Sex LV0",
            "Anal Sex LV0",
            "Oral Sex LV0",
            "BDSM Sex LV0",
            "Exhibitionist LV0",
            "Lesbian LV0.1",
            "Group Sex LV0"
        };
    };
    
    DebugGirlsList* DebugGirlsList::m_Instance = nullptr;
    
    class EventHandler
    {
    public:
        EventHandler() = default;
        virtual ~EventHandler()
        {}
        
        virtual void ProcessEvent( Rocket::Core::Event& event, const Rocket::Core::String& value ) = 0;
    };

    typedef std::map< Rocket::Core::String, EventHandler* > EventHandlerMap;
    EventHandlerMap eventHandlers;
    
    static EventHandler* currentEventHandler = nullptr;
    static std::string currentWindow = "";
    
    class ResizeEvent : public Rocket::Core::EventListener
    {
    public:
        virtual ~ResizeEvent()
        {}
        
        virtual void ProcessEvent( Rocket::Core::Event& event ) override
        {
            Logger() << Helper::DumpRocketEvent( event ).c_str();
        }
    };
    
    /*static ResizeEvent* resizeEvent = nullptr;*/
    
    class EventManager
    {
    public:
        EventManager() = delete;
        ~EventManager() = delete;
        
        static void Shutdown()
        {
            Logger() << "EventManager::Shutdown\n";
            
            for( EventHandlerMap::iterator it = eventHandlers.begin(); it != eventHandlers.end(); ++it )
            {
                Logger() << "Deleting event handler \"" << (*it).first.CString() << "\".\n";
                delete (*it).second;
            }
            
            eventHandlers.clear();
            currentEventHandler = nullptr;
            currentWindow = "";
            
            Logger() << "EventManager successfuly shut down.\n";
        }
        
        static void RegisterEventHandler( const Rocket::Core::String& handlerName, EventHandler* handler )
        {
            Logger() << "EventManager::RegisterEventHandler (" << handlerName.CString() << ")\n";
            
            EventHandlerMap::iterator it = eventHandlers.find( handlerName );
            if( it != eventHandlers.end() )
            {
                Logger() << "Deleting event handler \"" << (*it).first.CString() << "\"\n";
                delete (*it).second;
            }
            
            eventHandlers[handlerName] = handler;
        }
        
        static void ProcessEvent( Rocket::Core::Event& event, const Rocket::Core::String& value )
        {
            Logger() << "EventManager::ProcessEvent (" << value.CString() << ")\n";
            //Logger() << Helper::DumpRocketEvent( event );
            
            Rocket::Core::StringList commands;
            Rocket::Core::StringUtilities::ExpandString( commands, value, ';' );
            
            for( size_t i = 0; i < commands.size(); i++ )
            {
                Rocket::Core::StringList values;
                Rocket::Core::StringUtilities::ExpandString( values, commands[i], ' ' );
                
                if( values.empty() )
                    return;
                
                if( values[0] == "goto" && values.size() > 1 )
                {
                    if( LoadWindow( values[1] ) )
                        event.GetTargetElement()->GetOwnerDocument()->Close();
                }
                else if( values[0] == "load" && values.size() > 1 )
                {
                    LoadWindow( values[1] );
                }
                else if( values[0] == "close" )
                {
                    Rocket::Core::ElementDocument* targetDocument = nullptr;
                    
                    if( values.size() > 1 )
                        targetDocument = context->GetDocument( values[1].CString() );
                    else
                        targetDocument = event.GetTargetElement()->GetOwnerDocument();
                    
                    if( targetDocument != nullptr )
                        targetDocument->Close();
                }
                else if( values[0] == "unimplemented" )
                {
                    Rocket::Core::Log::Message( Rocket::Core::Log::Type::LT_WARNING, "Unimplemented" );
                }
                else if( values[0] == "exit" )
                {
                    
                }
                else
                {
                    if( currentEventHandler != nullptr )
                        currentEventHandler->ProcessEvent( event, commands[i] );
                }
            }
        }
        
        static bool LoadWindow( const Rocket::Core::String& windowName )
        {
            Logger() << "EventManager::LoadWindow (" << windowName.CString() << ")\n";
            
            EventHandler* oldEventHandler = currentEventHandler;
            EventHandlerMap::iterator it = eventHandlers.find( windowName );
            if( it != eventHandlers.end() )
                currentEventHandler = (*it).second;
            else
                currentEventHandler = nullptr;
            
            /*if( resizeEvent == nullptr )
                resizeEvent = new ResizeEvent();*/
            
            Rocket::Core::String documentPath = Rocket::Core::String("../../WhoreMasterRenewal/Resources/Interface/") + windowName + Rocket::Core::String(".rml");
            Rocket::Core::ElementDocument* document = context->LoadDocument( documentPath );
            if( document == nullptr )
            {
                currentEventHandler = oldEventHandler;
                return false;
            }
            
            /*document->AddEventListener( "resize", resizeEvent );*/
            
            Rocket::Core::Element* title = document->GetElementById("title");
            if( title != nullptr )
                title->SetInnerRML( document->GetTitle() );
            document->Focus();
            document->Show();
            
            Logger() << "Size: [" << document->GetClientHeight() << "," << document->GetClientWidth() << "].\n";
            
            currentWindow = windowName.CString();
            
            document->RemoveReference();
            
            int currentNumDocs = context->GetNumDocuments();
            std::stringstream sstream;
            sstream << "Current number of documents: " << currentNumDocs << "\n";
            for( int i = 0; i < currentNumDocs; i++ )
            {
                sstream << "[" << context->GetDocument( i )->GetId().CString() << "] ";
            }
            sstream << "\n";
            
            Logger() << sstream.str().c_str();
            
            return true;
        }
        
        static void ReloadWindow()
        {
            Logger() << "EventManager::ReloadWindow() currentWindow=\"" << currentWindow.c_str() << "\"\n";
            
            context->GetDocument( currentWindow.c_str() )->Close();
            Rocket::Core::Factory::ClearStyleSheetCache();
            LoadWindow( currentWindow.c_str() );
        }
    };
    
    class Event : public Rocket::Core::EventListener
    {
    public:
        Event( const Rocket::Core::String& value )
            : m_Value( value )
        {
            Logger() << "Event::const (" << value.CString() << ")\n";
        }
        virtual ~Event()
        {}
        
        virtual void ProcessEvent( Rocket::Core::Event& event ) override
        {
            Logger() << "Event::ProcessEvent [m_Value=" << m_Value.CString() << "]\n";
            EventManager::ProcessEvent( event, m_Value );
        }
        
        virtual void OnDetach( Rocket::Core::Element* ) override
        {
            Logger() << "Event::OnDetach [m_Value=" << m_Value.CString() << "]\n";
            delete this;
        }
        
    private:
        Rocket::Core::String m_Value;
    };
    
    class EventListenerInstancerI : public Rocket::Core::EventListenerInstancer
    {
    public:
        EventListenerInstancerI() : EventListenerInstancer()
        {
            Logger() << "EventListenerInstancer::const\n";
        }
        virtual ~EventListenerInstancerI()
        {}
        
        virtual Rocket::Core::EventListener* InstanceEventListener( const Rocket::Core::String& value ) override
        {
            Logger() << "EventListenerInstancer::InstanceEventListener (" << value.CString() << ")\n";
            return new Event( value );
        }
        
        virtual void Release() override
        {
            Logger() << "EventListenerInstancer::Release\n";
            delete this;
        }
    };
}

int main( int argc, char* argv[] )
{
    wmr::Logger() << "Loading Whore Master: Renewal...\n";
    
    main_old( argc, argv );
    /*
    try
    {
        sf::VideoMode currentVideoMode = sf::VideoMode().getDesktopMode();
        sf::RenderWindow sfWindow( currentVideoMode, "Whore Master: Renewal" );
        
        RocketSFMLRenderer sfRenderInterface;
        RocketSFMLSystemInterface sfSystemInterface;
        ShellFileInterface fileInterface( "Resources/" );
        
        if( !sfWindow.isOpen() )
        {
            Rocket::Core::Log::Message( Rocket::Core::Log::Type::LT_ERROR, "Cannot create window..." );
            std::cout.flush();
            std::cerr.flush();
            return 1;
        }
        
        sfRenderInterface.SetWindow( &sfWindow );
        
        Rocket::Core::SetFileInterface( &fileInterface );
        Rocket::Core::SetRenderInterface( &sfRenderInterface );
        Rocket::Core::SetSystemInterface( &sfSystemInterface );
        
        if( !Rocket::Core::Initialise() )
        {
            Rocket::Core::Log::Message( Rocket::Core::Log::Type::LT_ERROR, "Cannot initialise libRocket framework..." );
            std::cout.flush();
            std::cerr.flush();
            return 1;
        }
        
        Rocket::Controls::Initialise();
        
        context = Rocket::Core::CreateContext( "defaultContext",
            Rocket::Core::Vector2i( sfWindow.getSize().x, sfWindow.getSize().y ) );
        
        if( !Rocket::Debugger::Initialise( context ) )
        {
            Rocket::Core::Log::Message( Rocket::Core::Log::Type::LT_ERROR, "Cannot initialise libRocket Debugger..." );
            std::cout.flush();
            std::cerr.flush();
        }
        
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/PT Sans Bold Italic.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/PT Sans Bold.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/PT Sans Italic.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/PT Sans.ttf" );
        
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSans.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSans-Bold.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSans-Oblique.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSans-BoldOblique.ttf" );
                
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSansMono.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSansMono-Bold.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSansMono-Oblique.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSansMono-BoldOblique.ttf" );
                
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSerif.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSerif-Bold.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSerif-BoldItalic.ttf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSerif-Italic.ttf" );
                
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/Delicious-Roman.otf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/Delicious-Bold.otf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/Delicious-BoldItalic.otf" );
        Rocket::Core::FontDatabase::LoadFontFace( "Fonts/Delicious-Italic.otf" );
        
//        Rocket::Core::ElementDocument* document = context->LoadDocument( "Interface/MainMenu.rml" );
//        wmr::DebugEventListener* listener = new wmr::DebugEventListener();
//        
//        if( document )
//        {
//            document->AddEventListener( "show", listener );
//            document->AddEventListener( "hide", listener );
//            document->AddEventListener( "resize", listener );
//            document->AddEventListener( "scroll", listener );
//            document->AddEventListener( "focus", listener );
//            document->AddEventListener( "blur", listener );
//            document->AddEventListener( "keydown", listener );
//            document->AddEventListener( "keyup", listener );
//            document->AddEventListener( "textinput", listener );
//            document->AddEventListener( "click", listener );
//            document->AddEventListener( "dblclick", listener );
//            document->AddEventListener( "mouseover", listener );
//            document->AddEventListener( "mouseout", listener );
//            document->AddEventListener( "mousemove", listener );
//            document->AddEventListener( "mouseup", listener );
//            document->AddEventListener( "mousedown", listener );
//            document->AddEventListener( "mousescroll", listener );
//            document->AddEventListener( "dragstart", listener );
//            document->AddEventListener( "dragend", listener );
//            document->AddEventListener( "drag", listener );
//            document->AddEventListener( "dragover", listener );
//            document->AddEventListener( "dragout", listener );
//            document->AddEventListener( "dragmove", listener );
//            document->AddEventListener( "dragdrop", listener );
//            document->AddEventListener( "submit", listener );
//            document->AddEventListener( "change", listener );
//            document->AddEventListener( "load", listener );
//            document->AddEventListener( "unload", listener );
//            document->AddEventListener( "handledrag", listener );
//            document->AddEventListener( "columnadd", listener );
//            document->AddEventListener( "rowupdate", listener );
//            document->AddEventListener( "rowadd", listener );
//            document->AddEventListener( "rowremove", listener );
//            document->AddEventListener( "tabchange", listener );
//            
//            document->Show();
//            document->RemoveReference();
//        }
        
        wmr::EventListenerInstancerI* eventListenerInstancer = new wmr::EventListenerInstancerI();
        Rocket::Core::Factory::RegisterEventListenerInstancer( eventListenerInstancer );
        eventListenerInstancer->RemoveReference();
        
        wmr::DebugGirlsList::Initialise();
        
        wmr::EventManager::LoadWindow("MainMenu");
        
        while( sfWindow.isOpen() )
        {
            static sf::Event event;
            
            sfWindow.clear();
            context->Render();
            sfWindow.display();
            
            while( sfWindow.pollEvent( event ) )
            {
                switch( event.type )
                {
                case sf::Event::EventType::Resized:
                    wmr::Logger() << "Resized Event: " << event.size.width << "x" << event.size.height << "\n";
                    sfRenderInterface.Resize();
                    break;
                    
                case sf::Event::EventType::MouseMoved:
                    context->ProcessMouseMove( event.mouseMove.x, event.mouseMove.y, sfSystemInterface.GetKeyModifiers( event ) );
                    break;
                    
                case sf::Event::EventType::MouseButtonPressed:
                    context->ProcessMouseButtonDown( event.mouseButton.button, sfSystemInterface.GetKeyModifiers( event ) );
                    break;
                    
                case sf::Event::EventType::MouseButtonReleased:
                    context->ProcessMouseButtonUp( event.mouseButton.button, sfSystemInterface.GetKeyModifiers( event ) );
                    break;
                    
                case sf::Event::EventType::MouseWheelMoved:
                    context->ProcessMouseWheel( event.mouseWheel.delta * (-1), sfSystemInterface.GetKeyModifiers( event ) );
                    break;
                    
                case sf::Event::EventType::TextEntered:
                    context->ProcessTextInput( static_cast<Rocket::Core::word>( event.text.unicode ) );
                    break;
                    
                case sf::Event::EventType::KeyPressed:
                    context->ProcessKeyDown( sfSystemInterface.TranslateKey( event.key.code ), sfSystemInterface.GetKeyModifiers( event ) );
                    break;
                    
                case sf::Event::EventType::KeyReleased:
                    if( event.key.code == sf::Keyboard::Key::Tilde )
                        Rocket::Debugger::SetVisible( !Rocket::Debugger::IsVisible() );
                    if( event.key.code == sf::Keyboard::Key::R && event.key.control )
                        wmr::EventManager::ReloadWindow();
                    
                    context->ProcessKeyUp( sfSystemInterface.TranslateKey( event.key.code ), sfSystemInterface.GetKeyModifiers( event ) );
                    break;
                    
                case sf::Event::EventType::Closed:
                    sfWindow.close();
                    break;
                
                // following events not used
                case sf::Event::EventType::Count:
                case sf::Event::EventType::GainedFocus:
                case sf::Event::EventType::JoystickButtonPressed:
                case sf::Event::EventType::JoystickButtonReleased:
                case sf::Event::EventType::JoystickConnected:
                case sf::Event::EventType::JoystickDisconnected:
                case sf::Event::EventType::JoystickMoved:
                case sf::Event::EventType::LostFocus:
                case sf::Event::EventType::MouseEntered:
                case sf::Event::EventType::MouseLeft:
                    break;
                    
                default:
                    break;
                }
            }
            
            context->Update();
        }
        context->RemoveReference();
        
        wmr::DebugGirlsList::Shutdown();
        wmr::EventManager::Shutdown();
        Rocket::Core::Shutdown();
    }
    catch( std::exception& ex )
    {
        wmr::Logger() << "Exception caught:\nException type: \"" << typeid(ex).name() << "\"\n" << ex.what() << "\n";
    }
    */
    wmr::Logger() << "Whore Master: Renewal finished...\n";
    
    return 0;
}
