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
#include "cTariff.h"
#include "BrothelManager.hpp"
#include "cGold.h"
#include "Girl.hpp"
#include "cGangs.h"
#include "cMessageBox.h"
#include "cGirls.h"
#include "GirlManager.hpp"

#include <sstream>

namespace WhoreMasterRenewal
{

bool cJobManager::WorkBeastCare(Girl* girl, Brothel* brothel, int DayNight, std::string& summary)
{
    std::string message = "";
	if(Preprocessing(ACTION_WORKCARING, girl, brothel, DayNight, summary, message))
		return true;

	// TODO need better dialog
	if(g_Dice%100 <= 10)
	{
		g_Girls.UpdateEnjoyment(girl, ACTION_WORKCARING, -3, true);
		message = " The animals were restless and disobedient.";
		girl->m_Events.AddMessage(message,IMGTYPE_PROFILE,DayNight);
		g_Brothels.add_to_beasts(-1);
	}
	else
	{
		g_Girls.UpdateEnjoyment(girl, ACTION_WORKCARING, +3, true);
		message = " She enjoyed her time working with the animals today.";
		girl->m_Events.AddMessage(message,IMGTYPE_PROFILE,DayNight);
		g_Brothels.add_to_beasts(2);
	}

	// Improve girl
	int xp = 5, libido = 1, skill = 1;

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

	girl->m_Pay += 65;
	g_Gold.staff_wages(65);  // wages come from you
	g_Girls.UpdateStat(girl, STAT_EXP, xp);
	g_Girls.UpdateSkill(girl, SKILL_SERVICE, skill);
	g_Girls.UpdateTempStat(girl, STAT_LIBIDO, libido);
	g_Girls.UpdateSkill(girl, SKILL_BEASTIALITY, g_Dice%8);
	
	return false;
}

} // namespace WhoreMasterRenewal
