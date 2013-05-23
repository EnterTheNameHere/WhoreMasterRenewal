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
#ifndef MAIN_H_INCLUDED_1502
#define MAIN_H_INCLUDED_1502
#pragma once

//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <stdio.h>
//#include <time.h>
//#include <math.h>

//#include "GameSystem.h"
//#include "Constants.h"
//#include "cTraits.h"
//#include "cGirls.h"
#include "cBrothel.h"
#include "cGangs.h"
//#include "cEvents.h"
#include "cCustomers.h"
#include "cTriggers.h"
//#include "cGameScript.h"
//#include "cRival.h"
#include "cGold.h"
//#include "cScrollBar.h"


template<typename T> std::string toString( T value )
{
    std::stringstream str;
    str << value;
    std::string result;
    str >> result;
    return result;
}

int main_old( int, char* [] );
void handle_hotkeys();

// SDL Graphics interface
//extern CGraphics g_Graphics;

// Resource Manager
//extern CResourceManager rmanager;

// Events
//extern SDL_Event vent;

// logfile
//extern CLog g_LogFile;

// Trait list
//extern cTraits g_Traits;

// Girl manager
//extern cGirls g_Girls;

// Brothel Manager
//extern cBrothelManager g_Brothels;

// Gang Manager
//extern cGangManager g_Gangs;

// Customer Manager
//extern cCustomers g_Customers;

// The global trigger manager
//extern cTriggerList g_GlobalTriggers;

// Keeping time in the game
//extern unsigned long g_Year;
//extern unsigned long g_Month;
//extern unsigned long g_Day;

// the players gold
//extern cGold g_Gold;

// Inventory manager
//extern cInventory g_InvManager;

//extern int g_CurrBrothel;

// if a scrollbar is currently being dragged, this points to it
//extern cScrollBar* g_DragScrollBar;

#endif // MAIN_H_INCLUDED_1502
