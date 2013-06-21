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
#include "Brothel.hpp"
#include "cCustomers.h"
#include "cRng.h"
#include "cInventory.h"
#include "sConfig.h"
#include "cRival.h"
#include "CLog.h"
#include "cTrainable.h"
#include "Girl.hpp"
#include "cTariff.h"
#include "cGold.h"
#include "cGangs.h"
#include "cMessageBox.h"
#include "cGirls.h"
#include "GirlManager.hpp"

#include <sstream>

namespace WhoreMasterRenewal
{

bool cJobManager::WorkCleaning(Girl* girl, Brothel* brothel, int DayNight, std::string& summary)
{
    std::string message = "";
	if(Preprocessing(ACTION_WORKCLEANING, girl, brothel, DayNight, summary, message))
		return true;

	// put that shit away, you'll scare off the customers!
	g_Girls.UnequipCombat(girl);

	// Complications
	if(g_Dice%100 <= 10)
	{
		g_Girls.UpdateEnjoyment(girl, ACTION_WORKCLEANING, -3, true);
		message = "Spilled a bucket of something unpleasant all over herself.";
		girl->m_Events.AddMessage(message, IMGTYPE_PROFILE, DayNight);
	}
	else
	{
		g_Girls.UpdateEnjoyment(girl, ACTION_WORKCLEANING, +3, true);
		message = "Cleaned the building while humming a pleasant tune.";
		girl->m_Events.AddMessage(message, IMGTYPE_PROFILE, DayNight);
	}
	
	// cleaning is a service skill
	int CleanAmt;
	if(g_Girls.GetSkill(girl, SKILL_SERVICE) >= 10)
		CleanAmt = ((g_Girls.GetSkill(girl, SKILL_SERVICE)/10)+5) * 10;
	else
	   CleanAmt = 50;

	brothel->m_Filthiness -= CleanAmt;
	std::stringstream sstemp;
    sstemp << "Cleanliness rating improved by " << CleanAmt;
	girl->m_Events.AddMessage(sstemp.str(), IMGTYPE_PROFILE, DayNight);

	// Improve girl
	int xp = 5, libido = 1, skill = 3;

	if (g_Girls.HasTrait(girl, "Quick Learner"))
	{
		skill += 1;
		xp += 3;
	}
	else if (g_Girls.HasTrait(girl, "Slow Learner"))
	{
		skill -= 1;
		xp -= 3;
	}

	if (g_Girls.HasTrait(girl, "Nymphomaniac"))
		libido += 2;

	girl->m_Pay += 25;
	g_Gold.building_upkeep(25);  // wages come from you
	g_Girls.UpdateStat(girl, STAT_EXP, xp);
	g_Girls.UpdateSkill(girl, SKILL_SERVICE, skill);
	g_Girls.UpdateTempStat(girl, STAT_LIBIDO, libido);

	return false;
}

} // namespace WhoreMasterRenewal
