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

bool cJobManager::WorkMatron(sGirl* girl, sBrothel* brothel, int DayNight, std::string& summary)
{
	cTariff tariff;
	if(DayNight == 1)
		return false;

    std::string message = "";
	if(Preprocessing(ACTION_WORKMATRON, girl, brothel, DayNight, summary, message))
		return true;

	// Complication
	if(g_Dice%100 <= 10)
	{
		g_Girls.UpdateEnjoyment(girl, ACTION_WORKMATRON, -3, true);
		message = "Had trouble dealing with some of the girls.";
		girl->m_Events.AddMessage(message, IMGTYPE_PROFILE, DayNight);
	}
	else
	{
		g_Girls.UpdateEnjoyment(girl, ACTION_WORKMATRON, +3, true);
		message = "Enjoyed helping the girls with their lives.";
		girl->m_Events.AddMessage(message, IMGTYPE_PROFILE, DayNight);
	}
	

	// Improve girl
	int xp = 10, libido = 1, skill = 3;

	if (g_Girls.HasTrait(girl, "Quick Learner"))
	{
		skill += 1;
		xp += 5;
	}
	else if (g_Girls.HasTrait(girl, "Slow Learner"))
	{
		skill -= 1;
		xp -= 5;
	}

	if (g_Girls.HasTrait(girl, "Nymphomaniac"))
		libido += 2;

	int wages = tariff.matron_wages(g_Girls.GetSkill(girl, SKILL_SERVICE));
	g_Gold.girl_support(wages);  // matron wages come from you
	girl->m_Pay += wages;
	g_Girls.UpdateStat(girl, STAT_EXP, xp);
	g_Girls.UpdateSkill(girl, SKILL_SERVICE, skill);
	g_Girls.UpdateTempStat(girl, STAT_LIBIDO, libido);

	g_Girls.PossiblyGainNewTrait(girl, "Charismatic", 30, ACTION_WORKMATRON, "She has worked as a matron long enough that she has learned to be more Charismatic.", DayNight != 0);
	g_Girls.PossiblyGainNewTrait(girl, "Psychic", 60, ACTION_WORKMATRON, "She has learned to handle the girls so well that you'd almost think she was Psychic.", DayNight != 0);

	return false;
}
