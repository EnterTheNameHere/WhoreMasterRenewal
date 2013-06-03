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

#include "cJobManager.h"
#include "cBrothel.h"
#include "cCustomers.h"
#include "cRng.h"
#include "cInventory.h"
#include "sConfig.h"
#include "cRival.h"
#include "CLog.h"
#include "cTrainable.h"
#include "cTariff.h"
#include "cGold.h"
#include "cGangs.h"
#include "cMessageBox.h"
#include "cGirls.h"

#include <sstream>

bool cJobManager::WorkFluffer(sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary)
{
    std::string message = "";
	if(Preprocessing(ACTION_WORKSHOW, girl, brothel, DayNight, summary, message))
		return true;

	// put that shit away, you'll scare off the customers!
	g_Girls.UnequipCombat(girl);

	// TODO need better dialog
	if(g_Dice%100 <= 10)
	{
		g_Girls.UpdateEnjoyment(girl, ACTION_WORKSHOW, -3, true);
		message = " She got frustrated when fluffing didn't work exactly as planned.";
		girl->m_Events.AddMessage(message,IMGTYPE_PROFILE,DayNight);
	}
	else
	{
		g_Girls.UpdateEnjoyment(girl, ACTION_WORKSHOW, +3, true);
		message = " She enjoyed her time fluffing in the show today.";
		girl->m_Events.AddMessage(message,IMGTYPE_PROFILE,DayNight);
		brothel->m_CurrFilm->quality_multiplyer=1.5;
	}

	// Improve girl
	int xp = 3, libido = 1, skill = 1;

	if (g_Girls.HasTrait(girl, "Quick Learner"))
	{
		skill += 1;
		xp += 2;
	}
	else if (g_Girls.HasTrait(girl, "Slow Learner"))
	{
		skill -= 1;
		xp -= 2;
	}

	if (g_Girls.HasTrait(girl, "Nymphomaniac"))
		libido += 2;

	girl->m_Pay += 55;
	g_Gold.staff_wages(55);  // wages come from you
	g_Girls.UpdateStat(girl, STAT_EXP, xp);
	g_Girls.UpdateSkill(girl, SKILL_SERVICE, skill);
	g_Girls.UpdateTempStat(girl, STAT_LIBIDO, libido);

	return false;
}
