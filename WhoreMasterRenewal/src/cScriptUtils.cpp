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

#include "cScriptUtils.h"
#include "cGirls.h"
#include "GirlManager.hpp"
#include "BrothelManager.hpp"
#include "cMessageBox.h"
#include "Brothel.hpp"
#include "Girl.hpp"

namespace WhoreMasterRenewal
{

void cScriptUtils::add_girl_to_brothel(Girl *girl)
{
/*
 *	no girl -> nothing to do -> go home early
 */
	if(!girl) {
		return;
	}

	/* MYR: For some reason I can't figure out, a number of girl's house percentages
            are at zero or set to zero when they are sent to the brothel. I'm not sure
			how to fix it, so I'm explicitly setting the percentage to 60 here */
	girl->m_Stats[STAT_HOUSE] = 60;

    std::string text = girl->m_Realname;
/*
 *	OK: how rebellious is this floozy?
 */
	if(g_Girls.GetRebelValue(girl, false) >= 35) {
		text += " has been sent to your dungeon, as she is rebellious and poorly trained.";
		g_MessageQue.AddToQue(text, 0);
		g_Brothels.GetDungeon()->AddGirl(girl, DUNGEON_NEWGIRL);
		return;
	}
/*
 *	She qualifies for brothel duty - is there room?
 *	let's get some numbers
 */
	int total_rooms = g_Brothels.GetBrothel(g_CurrBrothel)->m_NumRooms;
	int rooms_used  = g_Brothels.GetBrothel(g_CurrBrothel)->m_NumGirls;
	int diff = total_rooms - rooms_used;
/*
 *	now then...
 */
	if(diff <= 0) {
		text += " has been sent to your dungeon, since current brothel is full.";
		g_MessageQue.AddToQue(text, 0);
		g_Brothels.GetDungeon()->AddGirl(girl, DUNGEON_NEWGIRL);
		return;
	}
/*
 *	otherwise, it's very simple
 */
	text += " has been sent to your current brothel.";
	g_MessageQue.AddToQue(text, 0);
	g_Brothels.AddGirl(g_CurrBrothel, girl);
}

} // namespace WhoreMasterRenewal
