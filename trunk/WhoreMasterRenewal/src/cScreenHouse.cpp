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

#include "cScreenHouse.h"
#include "cBrothel.h"
#include "cWindowManager.h"
#include "cGold.h"
#include "cGetStringScreenManager.h"
#include "InterfaceGlobals.h"
#include "cGangs.h"
#include "cInterfaceEvent.h"
#include "CLog.h"
#include "DirPath.h"

extern bool g_InitWin;
extern int g_CurrBrothel;
extern cGold g_Gold;
extern cBrothelManager g_Brothels;
extern cWindowManager g_WinManager;
extern cInterfaceEventManager g_InterfaceEvents;
extern long g_IntReturn;
extern cGangManager g_Gangs;

extern void GetString();
extern cInterfaceWindow g_GetString;
extern void GetInt();
extern cInterfaceWindow g_GetInt;

static std::string fmt_objective(std::stringstream &ss, std::string desc, int limit, int sofar=-1)
{
	ss << desc;
	if(limit != -1) {
		ss << " in " << limit << " weeks";
	}
	if(sofar > -1) {
		ss << ", " << sofar << " acquired so far";
	}
	ss << ".";
	return ss.str();
}


bool cScreenHouse::ids_set = false;

cScreenHouse::cScreenHouse()
{
    DirPath dp = DirPath()
        << "Resources"
        << "Interface"
        << "house_screen.xml"
    ;
    m_filename = dp.c_str();
}
cScreenHouse::~cScreenHouse()
{
    
}

void cScreenHouse::set_ids()
{
	ids_set = true;
	back_id	= get_id("BackButton");
	details_id = get_id("HouseDetails");
	header_id = get_id("ScreenHeader");
}

void cScreenHouse::init()
{
	if(!g_InitWin) {
		return;
	}
	Focused();
	g_InitWin = false;

	std::locale syslocale("");
	std::stringstream ss;
	ss.imbue(syslocale);

	ss << "CURRENT OBJECTIVE: ";
	sObjective* obj = g_Brothels.GetObjective();
	if(obj)
	{
		switch(obj->m_Objective) {
		case OBJECTIVE_REACHGOLDTARGET:
			ss << "Gather " << obj->m_Target << " gold";
			if(obj->m_Limit != -1) {
				ss << " in " << obj->m_Limit << " weeks";
			}
			ss << ", " << g_Gold.ival() << " gathered so far.";
			break;
		case OBJECTIVE_GETNEXTBROTHEL:
			fmt_objective(ss, "Purchase the next brothel", obj->m_Limit);
			break;
/*----
		case OBJECTIVE_PURCHASENEWGAMBLINGHALL:
			fmt_objective(ss, "Purchase a gambling hall", obj->m_Limit);
			break;
		case OBJECTIVE_PURCHASENEWBAR:
			fmt_objective(ss, "Purchase a bar", obj->m_Limit);
			break;
----*/
		case OBJECTIVE_LAUNCHSUCCESSFULATTACK:
			fmt_objective(ss, "Launch a successful attack", obj->m_Limit);
			break;
		case OBJECTIVE_HAVEXGOONS:
			ss << "Have " << obj->m_Target << " gangs";
			fmt_objective(ss, "", obj->m_Limit);
			break;
		case OBJECTIVE_STEALXAMOUNTOFGOLD:
			ss << "Steal " << obj->m_Target << " gold";
			fmt_objective(ss, "", obj->m_Limit, obj->m_SoFar);
			break;
		case OBJECTIVE_CAPTUREXMONSTERGIRLS:
			ss << "Capture " << obj->m_Target << " monster girls from the catacombs";
			fmt_objective(ss, "", obj->m_Limit, obj->m_SoFar);
			break;
		case OBJECTIVE_HAVEXMONSTERGIRLS:
			ss << "Have a total of " << obj->m_Target << " monster girls";
			fmt_objective(ss, "", obj->m_Limit, g_Brothels.GetTotalNumGirls(true));
			break;
		case OBJECTIVE_KIDNAPXGIRLS:
			ss << "Kidnap " << obj->m_Target << " girls from the streets";
			fmt_objective(ss, "", obj->m_Limit, obj->m_SoFar);
			break;
		case OBJECTIVE_EXTORTXNEWBUSSINESS:
			ss << "Control " << obj->m_Target << " city business";
			fmt_objective(ss, "", obj->m_Limit, obj->m_SoFar);
			break;
		case OBJECTIVE_HAVEXAMOUNTOFGIRLS:
			ss << "Have a total of " << obj->m_Target << " girls";
			fmt_objective(ss, "", obj->m_Limit, g_Brothels.GetTotalNumGirls(false));
			break;
        default:
            g_LogFile.ss() << "switch (obj->m_Objective): unknown value \"" << obj->m_Objective << "\"\n" << __FILE__ << " " << __LINE__ << "\n";
		}
	}
	else ss << "NONE\n";

	ss << "\nCurrent number of runaways: "
	   << g_Brothels.GetNumRunaways()
	   << "\n"
	;

	ss << "\n"
	   << "Current gold: " << g_Gold.ival() << "\n"
	   << "Bank account: " << g_Brothels.GetBankMoney() << "\n"
	   << "Businesses controlled: "
	   << g_Gangs.GetNumBusinessExtorted()
	   << "\n"
	;

	EditTextItem(ss.str(), details_id);
	obj = 0;
}

void cScreenHouse::process()
{
/*
 *	we need to make sure the ID variables are set
 */
	if(!ids_set)
		set_ids();

/*
 *	set up the window if needed
 */
	init();

/*
 *	check to see if there's a button event needing handling
 */
	check_events();
}


void cScreenHouse::check_events()
{
/*
 *	no events means we can go home
 */
	if(g_InterfaceEvents.GetNumEvents() == 0)
		return;

/*
 *	if it's the back button, pop the window off the stack
 *	and we're done
 */
	if(g_InterfaceEvents.CheckButton(back_id)) {
		g_InitWin = true;
		g_WinManager.Pop();
		return;
	}

}
