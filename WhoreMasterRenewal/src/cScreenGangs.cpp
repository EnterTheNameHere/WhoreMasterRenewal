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

#include "cScreenGangs.h"
#include "Helper.hpp"
#include "Brothel.hpp"
#include "cWindowManager.h"
#include "cGold.h"
#include "InterfaceProcesses.h"
#include "InterfaceGlobals.h"
#include "cGangs.h"
#include "GangManager.hpp"
#include "cInterfaceEvent.h"
#include "CLog.h"
#include "cTariff.h"
#include "DirPath.h"

namespace WhoreMasterRenewal
{

static cTariff tariff;
static std::stringstream ss;

static int selection = -1;
static int sel_recruit = -1;

bool cScreenGangs::ids_set = false;

cScreenGangs::cScreenGangs()
{
    DirPath dp = DirPath()
        << "Resources"
        << "Interface"
        << "gangs_screen.xml"
    ;
    m_filename = dp.c_str();
}
cScreenGangs::~cScreenGangs()
{
    
}

void cScreenGangs::set_ids()
{
	ids_set = true;
	back_id = get_id("BackButton");
	gangcost_id = get_id("GangHireCost");
	ganghire_id = get_id("GangHireButton");
	gangfire_id = get_id("GangFireButton");
	totalcost_id = get_id("TotalCost");
	ganglist_id = get_id("GangList");
	missionlist_id = get_id("MissionList");
	gangdesc_id = get_id("GangDescription");
	missiondesc_id = get_id("MissionDescription");
	weaponlevel_id = get_id("WeaponDescription");
	weaponup_id = get_id("WeaponUpButton");
	netdesc_id = get_id("NetDescription");
	netbuy_id = get_id("BuyNetsButton");
	netautobuy_id = get_id("AutoBuyNetsToggle");
	healdesc_id = get_id("HealPotDescription");
	healbuy_id = get_id("BuyHealPotButton");
	healautobuy_id = get_id("AutoBuyHealToggle");
	recruitlist_id = get_id("RecruitList");

	//Set the default sort order for columns, so listboxes know the order in which data will be sent
    std::string RecruitColumns[] = {"GangName", "Number", "Combat", "Magic", "Intelligence", "Agility", "Constitution", "Charisma"};
	SortColumns(recruitlist_id, RecruitColumns, 8);
    std::string GangColumns[] = {"GangName", "Number", "Mission", "Combat", "Magic", "Intelligence", "Agility", "Constitution", "Charisma"};
	SortColumns(ganglist_id, GangColumns, 9);
}

void cScreenGangs::init()
{
	if(!g_InitWin)
		return;

	Focused();
	g_InitWin = false;

////////////////////

	cConfig cfg;

	selection = GetLastSelectedItemFromList(ganglist_id);
	sel_recruit = GetLastSelectedItemFromList(recruitlist_id);

	int cost = tariff.goon_mission_cost(MISS_GUARDING);
	ss.str("");
	ss << "Gangs: " << cost << " gold / week";
	EditTextItem(ss.str(), gangcost_id);

	ClearListBox(missionlist_id);
	AddToListBox(missionlist_id, 0, "GUARDING");
	AddToListBox(missionlist_id, 1, "SABOTAGE");
	AddToListBox(missionlist_id, 2, "SPY ON GIRLS");
	AddToListBox(missionlist_id, 3, "RECAPTURE");
	AddToListBox(missionlist_id, 4, "ACQUIRE TERRITORY");
	AddToListBox(missionlist_id, 5, "PETTY THEFT");
	AddToListBox(missionlist_id, 6, "GRAND THEFT");
	AddToListBox(missionlist_id, 7, "KIDNAPPING");
	AddToListBox(missionlist_id, 8, "CATACOMBS");
	AddToListBox(missionlist_id, 9, "TRAINING");
	AddToListBox(missionlist_id, 10, "RECRUITING");

	SetCheckBox(netautobuy_id, (g_Gangs.GetNetRestock() > 0));
	SetCheckBox(healautobuy_id, (g_Gangs.GetHealingRestock() > 0));

	// weapon upgrades
	int *wlev = g_Gangs.GetWeaponLevel();
	ss.str("");
	ss << "Weapon Level: " << *wlev;
	if((*wlev) < 4)
	{
		EnableButton(weaponup_id);
		ss << " Next: "
		   << tariff.goon_weapon_upgrade(*wlev)
		   << "g";
	}
	else
		DisableButton(weaponup_id);
	std::string s = ss.str();
	g_LogFile.ss() << "weapon text = '" << s << "'" << std::endl;
	g_LogFile.ssend();
	EditTextItem(s, weaponlevel_id);

	int *nets = g_Gangs.GetNets();
	ss.str("");
	ss << "Nets ("
	   << tariff.nets_price(1)
	   << "g each): " << *nets;
	DisableButton(netbuy_id, *nets >= 60);
	DisableCheckBox(netautobuy_id, *nets < 1);
	EditTextItem(ss.str(), netdesc_id);

	int *potions = g_Gangs.GetHealingPotions();
	ss.str("");
	ss << "Heal Potions ("
	   << tariff.healing_price(1)
	   << "g each): " <<
	   *potions
	;
	EditTextItem(ss.str(), healdesc_id);
	DisableButton(healbuy_id, *potions >= 200);
	DisableCheckBox(healautobuy_id, *potions < 1);

    std::string message;
	buffer[0] = '\0';
	if(g_Gangs.GetNumGangs() > 0)
	{
		cost = 0;
		for(int i=0; i < g_Gangs.GetNumGangs(); i++)
		{
			sGang* g = g_Gangs.GetGang(i);
			if (g == nullptr)
				g = g_Gangs.GetGang(i-1);

			cost += tariff.goon_mission_cost(g->m_MissionID);
		}
		message = "Weekly Cost: ";
		message += toString(cost);
	}
	else
		message = "Weekly Cost: 0";

	EditTextItem(message, totalcost_id);

	ClearListBox(ganglist_id);

	int num = 0;
	sGang* current = g_Gangs.GetGang(0);

/*
 *		loop through the gangs, populating the list box
 */
	g_LogFile.ss() << "Setting gang mission descriptions" << std::endl;
	g_LogFile.ssend();
	for(current = g_Gangs.GetGang(0); current; current = current->m_Next)
	{
/*
 *			format the string with the gang name, mission and number of men
 */
		std::string Data[9];
		ss.str("");
		ss << current->m_Name;
		Data[0] = ss.str();
		ss.str("");
		ss << current->m_Num;
		Data[1] = ss.str();
		ss.str("");
		ss << short_mission_desc(current->m_MissionID);
		Data[2] = ss.str();
		ss.str("");
		ss << (int)current->m_Skills[SKILL_COMBAT] << "%";
		Data[3] = ss.str();
		ss.str("");
		ss << (int)current->m_Skills[SKILL_MAGIC] << "%";
		Data[4] = ss.str();
		ss.str("");
		ss << (int)current->m_Stats[STAT_INTELLIGENCE] << "%";
		Data[5] = ss.str();
		ss.str("");
		ss << (int)current->m_Stats[STAT_AGILITY] << "%";
		Data[6] = ss.str();
		ss.str("");
		ss << (int)current->m_Stats[STAT_CONSTITUTION] << "%";
		Data[7] = ss.str();
		ss.str("");
		ss << (int)current->m_Stats[STAT_CHARISMA] << "%";
		Data[8] = ss.str();

//		g_LogFile.ss() << "Gang:\t" << Data[0] << "\t" << Data[1] << "\t" << Data[2]
//			<< "\t" << Data[3] << "\t" << Data[4] << "\t" << Data[5] << "\t" << Data[6] << std::endl;
//      g_LogFile.ssend();

/*
 *			add the box to the list; red highlight gangs that are low on numbers
 */
		AddToListBox( ganglist_id, num++, Data, 9, (current->m_Num < 6 ? LISTBOX_RED : LISTBOX_BLUE) );
	}

	ClearListBox(recruitlist_id);

	num = 0;
	current = g_Gangs.GetHireableGang(0);

/*
 *		loop through the gangs, populating the list box
 */
	g_LogFile.ss() << "Setting recruitable gang info" << std::endl;
	g_LogFile.ssend();
	for(current = g_Gangs.GetHireableGang(0); current; current = current->m_Next)
	{
/*
 *			format the string with the gang name, mission and number of men
 */
		std::string Data[8];
		ss.str("");
		ss << current->m_Name;
		Data[0] = ss.str();
		ss.str("");
		ss << current->m_Num;
		Data[1] = ss.str();
		ss.str("");
		ss << (int)current->m_Skills[SKILL_COMBAT] << "%";
		Data[2] = ss.str();
		ss.str("");
		ss << (int)current->m_Skills[SKILL_MAGIC] << "%";
		Data[3] = ss.str();
		ss.str("");
		ss << (int)current->m_Stats[STAT_INTELLIGENCE] << "%";
		Data[4] = ss.str();
		ss.str("");
		ss << (int)current->m_Stats[STAT_AGILITY] << "%";
		Data[5] = ss.str();
		ss.str("");
		ss << (int)current->m_Stats[STAT_CONSTITUTION] << "%";
		Data[6] = ss.str();
		ss.str("");
		ss << (int)current->m_Stats[STAT_CHARISMA] << "%";
		Data[7] = ss.str();

//		g_LogFile.ss() << "Recruitable\t" << Data[0] << "\t" << Data[1] << "\t" << Data[2]
//			<< "\t" << Data[3] << "\t" << Data[4] << "\t" << Data[5] << std::endl;
//      g_LogFile.ssend();

/*
 *			add the box to the list
 */
		AddToListBox( recruitlist_id, num++, Data, 8, (current->m_Num < 6 ? LISTBOX_RED : LISTBOX_BLUE) );
	}

	if(selection == -1 && GetListBoxSize(ganglist_id) >= 1)
		selection = 0;

	if(selection >= 0)
	{
		while(selection > GetListBoxSize(ganglist_id) && selection != -1)
			selection--;
	}
	if(selection >= 0)
		SetSelectedItemInList(ganglist_id, selection);

	if(sel_recruit == -1 && GetListBoxSize(recruitlist_id) >= 1)
		sel_recruit = 0;
	if(sel_recruit >= 0)
		SetSelectedItemInList(recruitlist_id, sel_recruit);

	DisableButton(ganghire_id, (g_Gangs.GetNumHireableGangs() <= 0)
							|| (g_Gangs.GetNumGangs() >= 8)
							|| (sel_recruit == -1));
	DisableButton(gangfire_id, (g_Gangs.GetNumGangs() <= 0)
							|| (selection == -1));

	potions = nullptr;
	wlev = nullptr;
	nets = nullptr;
}

void cScreenGangs::process()
{
	// we need to make sure the ID variables are set
	if(!ids_set)
		set_ids();

	// handle arrow keys
 	if(check_keys())
		return;

	// set up the window if needed
	init();

	// check to see if there's a button event needing handling
	check_events();
}

bool cScreenGangs::check_keys()
{
	if(g_UpArrow) {
		selection = ArrowUpListBox(ganglist_id);
		g_UpArrow = false;
		return true;
	}
	if(g_DownArrow) {
		selection = ArrowDownListBox(ganglist_id);
		g_DownArrow = false;
		return true;
	}
	return false;
}


void cScreenGangs::check_events()
{
	// no events means we can go home
	if(g_InterfaceEvents.GetNumEvents() == 0)
		return;

	// if it's the back button, pop the window off the stack and we're done
	if(g_InterfaceEvents.CheckButton(back_id))
	{
		g_InitWin = true;
		g_WinManager.Pop();
		return;
	}
	if(g_InterfaceEvents.CheckButton(ganghire_id))
	{
		hire_recruitable();
		return;
	}
	if(g_InterfaceEvents.CheckButton(weaponup_id))
	{
		int cost = 0;
		int *wlev = g_Gangs.GetWeaponLevel();
		cost = tariff.goon_weapon_upgrade(*wlev);
		if(g_Gold.item_cost(cost) == true)
		{
			*wlev += 1;
			g_InitWin = true;
		}
		wlev = nullptr;
		return;
	}
	if(g_InterfaceEvents.CheckButton(netbuy_id))
	{
		int cost = 0;
		int amount = 20;
		int *nets = g_Gangs.GetNets();
		if(((*nets) + 20) > 60)
			amount = 60 - (*nets);
		cost = tariff.nets_price(amount);
		if(g_Gold.item_cost(cost) == true)
		{
			*nets += amount;
			if(IsCheckboxOn(netautobuy_id))
				g_Gangs.KeepNetStocked(*nets);
			g_InitWin = true;
		}
		nets = nullptr;
		return;
	}
	if(g_InterfaceEvents.CheckButton(healbuy_id))
	{
		int cost = 0;
		int amount = 20;
		int *potions = g_Gangs.GetHealingPotions();
		if(((*potions) + 20) > 200)
			amount = 200 - (*potions);
		cost = tariff.healing_price(amount);
		if(g_Gold.item_cost(cost) == true)
		{
			*potions += amount;
			g_InitWin = true;
		}
		potions = nullptr;
		return;
	}
	if(g_InterfaceEvents.CheckCheckbox(netautobuy_id))
	{
		int *nets = g_Gangs.GetNets();
		if(IsCheckboxOn(netautobuy_id))
			g_Gangs.KeepNetStocked(*nets);
		else
			g_Gangs.KeepNetStocked(0);
	}
	if(g_InterfaceEvents.CheckCheckbox(healautobuy_id))
	{
		int *potions = g_Gangs.GetHealingPotions();
		if(IsCheckboxOn(healautobuy_id))
			g_Gangs.KeepHealStocked(*potions);
		else
			g_Gangs.KeepHealStocked(0);
	}
	if(g_InterfaceEvents.CheckButton(gangfire_id))
	{
		selection = GetLastSelectedItemFromList(ganglist_id);
		if(selection != -1)
		{
			g_Gangs.FireGang(selection);
			g_InitWin = true;
		}
		return;
	}
	if(g_InterfaceEvents.CheckListbox(recruitlist_id))
	{
	    std::string ClickedHeader = HeaderClicked(recruitlist_id);
		if(ClickedHeader != "")
		{
			g_LogFile.ss() << "User clicked \"" << ClickedHeader << "\" column header on Recruit listbox" << std::endl;
            g_LogFile.ssend();
			return;
		}

		g_LogFile.ss() << "selected recruitable gang changed" << std::endl;
		g_LogFile.ssend();
		sel_recruit = GetLastSelectedItemFromList(recruitlist_id);

		if(ListDoubleClicked(recruitlist_id))
		{
			g_LogFile.ss() << "User double-clicked recruitable gang! Hiring if possible." << std::endl;
			g_LogFile.ssend();
			hire_recruitable();
			return;
		}
//		g_InitWin = true;
	}
/*
 *		this is what gets called it you change the selected gang
 */
	if(g_InterfaceEvents.CheckListbox(ganglist_id))
	{
	    std::string ClickedHeader = HeaderClicked(ganglist_id);
		if(ClickedHeader != "")
		{
			g_LogFile.ss() << "User clicked \"" << ClickedHeader << "\" column header on Gangs listbox" << std::endl;
			g_LogFile.ssend();
			return;
		}

		g_LogFile.ss() << "selected gang changed" << std::endl;
		g_LogFile.ssend();
		selection = GetLastSelectedItemFromList(ganglist_id);
		if(selection != -1)
		{
			sGang* gang = g_Gangs.GetGang(selection);

		    std::string text = "Name: ";
			text += gang->m_Name;
			text += "\n";
			text += "Number: ";
			text += toString((int)gang->m_Num);
			text += "\n";
			text += "Combat: ";
			text += toString(gang->m_Skills[SKILL_COMBAT]);
			text += "%\n";
			text += "Magic: ";
			text += toString(gang->m_Skills[SKILL_MAGIC]);
			text += "%\n";
			text += "Intelligence: ";
			text += toString(gang->m_Stats[STAT_INTELLIGENCE]);
			text += "%\n";
			EditTextItem(text, gangdesc_id);

			SetSelectedItemInList(missionlist_id, gang->m_MissionID, false);
/*
 *				set the long description for the mission
 */
			set_mission_desc(gang->m_MissionID);
		}
	}
	if(g_InterfaceEvents.CheckListbox(missionlist_id))
	{
/*
 *			get the index into the missions list
 */
		int mission_id =  GetLastSelectedItemFromList(missionlist_id);
		g_LogFile.ss() << "selchange: mid = " << mission_id << std::endl;
		g_LogFile.ssend();
/*
 *			set the textfield with the long description and price
 *			for this mission
 */
		set_mission_desc(mission_id);

		g_LogFile.ss() << "selection change: rebuilding gang list box" << std::endl;
		g_LogFile.ssend();

		for(int	selection = multi_first();
			selection != -1;
			selection = multi_next()
		) {
			sGang* gang = g_Gangs.GetGang(selection);
/*
 *				make sure we found the gang - pretty catastrophic
 *				if not, so log it if we do
 */
			if(gang == nullptr) {
				g_LogFile.ss()	<< "Error: No gang for index "
				  		<< selection
				;
				g_LogFile.ssend();
				continue;
			}
/*
 *				if the mission id is -1, nothing else to do
 *				(moving this to before the recruitment check
 *				since -1 most likely means nothing selected in
 *				the missions list)
 */
			if(mission_id == -1) {
				continue;
			}
/*
 *				if the gang is already doing <whatever>
 *				then let them get on with it
 */
			if(gang->m_MissionID == u_int(mission_id)) {
				continue;
			}
/*
 *				if they were recruiting, turn off the
 *				auto-recruit flag
 */
			if(gang->m_MissionID == MISS_RECRUIT && gang->m_AutoRecruit)
			{
				gang->m_AutoRecruit = false;
				gang->m_LastMissID = -1;
			}
			gang->m_MissionID = mission_id;
/*
 *				format the display line
 */
		    std::string Data[6];
			ss.str("");
			ss << gang->m_Name;
			Data[0] = ss.str();
			ss.str("");
			ss << gang->m_Num;
			Data[1] = ss.str();
			ss.str("");
			ss << short_mission_desc(mission_id);
			Data[2] = ss.str();
			ss.str("");
			ss << (int)gang->m_Skills[SKILL_COMBAT] << "%";
			Data[3] = ss.str();
			ss.str("");
			ss << (int)gang->m_Skills[SKILL_MAGIC] << "%";
			Data[4] = ss.str();
			ss.str("");
			ss << (int)gang->m_Stats[STAT_INTELLIGENCE] << "%";
			Data[5] = ss.str();

//		    g_LogFile.ss() << "Gang:\t" << Data[0] << "\t" << Data[1] << "\t" << Data[2]
//			    << "\t" << Data[3] << "\t" << Data[4] << "\t" << Data[5] << "\t" << Data[6] << std::endl
//              << "        index " << mission_id << ": " << ss.str() << std::endl;
//          g_LogFile.ssend();
/*
 *				and add it to the list
 */
			SetSelectedItemText( ganglist_id, selection, Data, 6 );
		}

	    std::string message = "";
		if(g_Gangs.GetNumGangs() > 0)
		{
			int cost = 0;
			for(int i=0; i < g_Gangs.GetNumGangs(); i++)
			{
				sGang* g = g_Gangs.GetGang(i);
				cost += tariff.goon_mission_cost(
					g->m_MissionID
				);
			}
			message = "Daily Cost: ";
			message += toString(cost);
		}
		else
			message = "Daily Cost: 0";

		EditTextItem(message, totalcost_id);
	}

}


std::string cScreenGangs::mission_desc(int mid)
{
	switch(mid) {
	case MISS_GUARDING:
		return	"Your men will guard your property.";
	case MISS_SABOTAGE:
		return	"Your men will move about town "
			"and destroy and loot enemy businesses."
		;
	case MISS_SPYGIRLS:
		return	"Your men will spy on your working girls, looking "
			"for the ones who take extra for themselves."
		;
	case MISS_CAPTUREGIRL:
		return
			"Your men will set out to re-capture any girls who have "
			"recently run away."
		;
	case MISS_EXTORTION:
		return
			"Sends your men out to force local gangs out of "
			"their areas of town."
		;
	case MISS_PETYTHEFT:
		return "Your men will mug people in the street.";
	case MISS_GRANDTHEFT:
		return
			"Your men will attempt to rob a bank or other risky place "
			"with high rewards."
		;
	case MISS_KIDNAPP:
		return
			"Your men will kidnap beggar, homeless or lost "
			"girls from the street and also lure other girls "
			"into working for you."
		;
	case MISS_CATACOMBS:
		return "Your men will explore the catacombs for treasure.";
	case MISS_TRAINING:
		return "Your men will improve their skills slightly.";
	case MISS_RECRUIT:
		return "Your men will replace their missing men (up to 15).";
	default:
		break;
	}

	ss << "Error: unexpected mission ID: " << mid;
	return ss.str();
}


std::string cScreenGangs::short_mission_desc(int mid)
{
	g_LogFile.ss() << "short_mission_desc(" << mid << ")" << std::endl;
	g_LogFile.ssend();
	switch(mid)
	{
	case MISS_GUARDING:	return "Guarding";
	case MISS_SABOTAGE:	return "Sabotaging";
	case MISS_SPYGIRLS:	return "Watching Girls";
	case MISS_CAPTUREGIRL:	return "Finding escaped girls";
	case MISS_EXTORTION:	return "Acquiring Territory";
	case MISS_PETYTHEFT:	return "Mugging people";
	case MISS_GRANDTHEFT:	return "Robbing places";
	case MISS_KIDNAPP:	return "Kidnapping Girls";
	case MISS_CATACOMBS:	return "Exploring Catacombs";
	case MISS_TRAINING:	return "Training Skills";
	case MISS_RECRUIT:	return "Recruiting Men";
	default:		return "Error: Unknown";
	}
}

int cScreenGangs::set_mission_desc(int mid)
{
/*
 *	OK: get the difficulty-adjusted price for this mission
 */
	int price = tariff.goon_mission_cost(mid);
/*
 *	and get a description of the mission
 */
    std::string desc = mission_desc(mid);
/*
 *	stick 'em both together ...
 */
	ss.str("");
	ss << desc << " (" << price << "g)";
/*
 *	... and set the text field
 */
	EditTextItem(ss.str(), missiondesc_id);
/*
 *	return the mission price
 */
	return price;
}

void cScreenGangs::hire_recruitable()
{
	if ((g_Gangs.GetNumGangs() >= 8) || (sel_recruit == -1))
		return;
	g_Gangs.HireGang(sel_recruit);
	g_InitWin = true;
}

int cScreenGangs::multi_first()
{
    sel_pos = 0;
    return GetNextSelectedItemFromList(ganglist_id, 0, sel_pos);
}

int cScreenGangs::multi_next()
{
    return GetNextSelectedItemFromList(ganglist_id, sel_pos+1, sel_pos);
}

} // namespace WhoreMasterRenewal
