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
#include "cBrothel.h"
#include "cScreenBuildingSetup.h"
#include "cWindowManager.h"
#include "cGold.h"
#include "cTariff.h"

extern bool g_InitWin;
extern int g_CurrBrothel;
extern cGold g_Gold;
extern cBrothelManager g_Brothels;
extern cWindowManager g_WinManager;

static cTariff tariff;
static std::stringstream ss;

bool cScreenBuildingSetup::ids_set = false;

void cScreenBuildingSetup::set_ids()
{
	ids_set = true;
	back_id = get_id("BackButton");
	curbrothel_id = get_id("CurrentBrothel");
	potioncost_id = get_id("PotionCost");
	potionavail_id = get_id("AvailablePotions");
	potions10_id = get_id("10PotionsButton");
	potions20_id = get_id("20PotionsButton");
	autopotions_id = get_id("AutoBuyPotionToggle");
	barstaff_id = get_id("BarStaffCost");
	barhire_id = get_id("BarHireButton");
	barfire_id = get_id("BarFireButton");
	casinostaff_id = get_id("CasinoStaffCost");
	casinohire_id = get_id("CasinoHireButton");
	casinofire_id = get_id("CasinoFireButton");
	roomcost_id = get_id("RoomAddCost");
	buyrooms_id = get_id("BuildRoomsButton");
	restrict_id = get_id("SexRestrictions");
	noanal_id = get_id("ProhibitAnalToggleRes");
	nobdsm_id = get_id("ProhibitBDSMToggleRes");
	nonormal_id = get_id("ProhibitNormalToggleRes");
	nobeast_id = get_id("ProhibitBeastToggleRes");
	nogroup_id = get_id("ProhibitGroupToggleRes");
	nolesbian_id = get_id("ProhibitLesbianToggleRes");
	advertsli_id = get_id("AdvertisingSlider");
	advertamt_id = get_id("AdvertisingValue");
}

void cScreenBuildingSetup::init()
{
	if(!g_InitWin)
		return;

	Focused();
	g_InitWin = false;

////////////////////

	ss.str("");
	ss << "Add Rooms: " << tariff.add_room_cost(5) << " gold";
	EditTextItem(ss.str(), roomcost_id);

	ss.str("");
	ss << "Anti-Preg Potions: " << tariff.anti_preg_price(1) << " gold each";
	EditTextItem(ss.str(), potioncost_id);

	ss.str("");
	ss << "Bar Staff: " << tariff.bar_staff_wages() << " gold / week";
	EditTextItem(ss.str(), barstaff_id);

	ss.str("");
	ss << "Casino Staff: " << tariff.casino_staff_wages() << " gold / week";
	EditTextItem(ss.str(), casinostaff_id);

    std::string brothel = "Current Brothel: ";
	brothel += g_Brothels.GetName(g_CurrBrothel);
	EditTextItem(brothel, curbrothel_id);

	int number = g_Brothels.GetNumPotions();

    std::string message = "You have: ";
	message += toString(number);
	EditTextItem(message, potionavail_id);
	DisableCheckBox(autopotions_id, number < 1);

	// let's limit advertising budget to multiples of 50 gold (~3 added customers), from 0 - 2000
	int advert = g_Brothels.GetBrothel(g_CurrBrothel)->m_AdvertisingBudget / 50;
	advert = SliderRange(advertsli_id, 0, (2000 / 50), advert, 4);  // set slider min/max range
	ss.str("");
	ss << "Advertising Budget: " << (advert * 50) << " gold / week";
	EditTextItem(ss.str(), advertamt_id);

	// setup check boxes
	SetCheckBox(autopotions_id, g_Brothels.GetPotionRestock());
	SetCheckBox(noanal_id, g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictAnal);
	SetCheckBox(nobdsm_id, g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictBDSM);
	SetCheckBox(nonormal_id, g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictNormal);
	SetCheckBox(nobeast_id, g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictBeast);
	SetCheckBox(nogroup_id, g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictGroup);
	SetCheckBox(nolesbian_id, g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictLesbian);

	if(g_Brothels.GetBrothel(g_CurrBrothel)->m_Bar == 0)
	{
		DisableButton(barhire_id, true);
		DisableButton(barfire_id, true);
	}
	else
	{
		DisableButton(barhire_id, !(g_Brothels.GetBrothel(g_CurrBrothel)->m_HasBarStaff == 0));
		DisableButton(barfire_id, (g_Brothels.GetBrothel(g_CurrBrothel)->m_HasBarStaff == 0));
	}

	if(g_Brothels.GetBrothel(g_CurrBrothel)->m_GamblingHall == 0)
	{
		DisableButton(casinohire_id, true);
		DisableButton(casinofire_id, true);
	}
	else
	{
		DisableButton(casinohire_id, !(g_Brothels.GetBrothel(g_CurrBrothel)->m_HasGambStaff == 0));
		DisableButton(casinofire_id, (g_Brothels.GetBrothel(g_CurrBrothel)->m_HasGambStaff == 0));
	}

}

void cScreenBuildingSetup::process()
{
	// we need to make sure the ID variables are set
	if(!ids_set)
		set_ids();

	// set up the window if needed
	init();

	// check to see if there's a button event needing handling
	check_events();
}


void cScreenBuildingSetup::check_events()
{
	// no events means we can go home
	if(g_InterfaceEvents.GetNumEvents() == 0)
		return;

	// if it's the back button, pop the window off the stack and we're done
	if(g_InterfaceEvents.CheckButton(back_id)) {
		g_InitWin = true;
		g_WinManager.Pop();
		return;
	}
	if(g_InterfaceEvents.CheckButton(buyrooms_id))
	{
		if(!g_Gold.brothel_cost(5000))
			g_MessageQue.AddToQue("You need 5000 gold to add 5 rooms", 1);
		else
			g_Brothels.GetBrothel(g_CurrBrothel)->m_NumRooms += 5;
	}
	if(g_InterfaceEvents.CheckButton(potions10_id))
	{
		int MaxSupplies = g_Brothels.GetSupplyShedLevel()*700;
		int number = g_Brothels.GetNumPotions();
		if(number == MaxSupplies)
		{
		    std::string message = "You can only store up to ";
			message += toString(MaxSupplies);
			message += " potions";
			g_MessageQue.AddToQue(message, 0);
		}
		else
		{
			if(number+10 > MaxSupplies)
				number = (number+10) - MaxSupplies;
			else
				number = 10;

			// afford returns a bool
			if(g_Gold.afford(number*2))
			{
				g_Gold.girl_support(number*2);
				g_Brothels.AddAntiPreg(number);
			}
			else
				g_MessageQue.AddToQue("You don't have enough gold", 0);
		}

		g_InitWin = true;
		return;
	}
	if(g_InterfaceEvents.CheckButton(potions20_id))
	{
		int MaxSupplies = g_Brothels.GetSupplyShedLevel()*700;
		int number = g_Brothels.GetNumPotions();
		if(number == MaxSupplies)
		{
		    std::string message = "You can only store up to ";
			message+=toString(MaxSupplies);
			message += " potions";
			g_MessageQue.AddToQue(message, 0);
		}
		else
		{
			if(number+20 > MaxSupplies)
				number = (number+20) - MaxSupplies;
			else
				number = 20;

			if(g_Gold.afford(number*2))
			{
				g_Gold.girl_support(number*2);
				g_Brothels.AddAntiPreg(number);
			}
			else
				g_MessageQue.AddToQue("You don't have enough gold", 0);
		}

		g_InitWin = true;
		return;
	}
	if(g_InterfaceEvents.CheckButton(barhire_id))
	{
		g_Brothels.GetBrothel(g_CurrBrothel)->m_HasBarStaff = 1;
		g_InitWin = true;
	}
	if(g_InterfaceEvents.CheckButton(barfire_id))
	{
		g_Brothels.GetBrothel(g_CurrBrothel)->m_HasBarStaff = 0;
		g_InitWin = true;
	}
	if(g_InterfaceEvents.CheckButton(casinohire_id))
	{
		g_Brothels.GetBrothel(g_CurrBrothel)->m_HasGambStaff = 1;
		g_InitWin = true;
	}
	if(g_InterfaceEvents.CheckButton(casinofire_id))
	{
		g_Brothels.GetBrothel(g_CurrBrothel)->m_HasGambStaff = 0;
		g_InitWin = true;
	}
	if(g_InterfaceEvents.CheckSlider(advertsli_id))
	{
		// set advertising budget based on slider
		g_Brothels.GetBrothel(g_CurrBrothel)->m_AdvertisingBudget = SliderValue(advertsli_id) * 50;
		ss.str("");
		ss << "Advertising Budget: " << (SliderValue(advertsli_id) * 50) << " gold / week";
		EditTextItem(ss.str(), advertamt_id);
	}
	if(g_InterfaceEvents.CheckCheckbox(autopotions_id))
		g_Brothels.KeepPotionsStocked(IsCheckboxOn(autopotions_id));
	if(g_InterfaceEvents.CheckCheckbox(noanal_id))
		g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictAnal = IsCheckboxOn(noanal_id);
	if(g_InterfaceEvents.CheckCheckbox(nobdsm_id))
		g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictBDSM = IsCheckboxOn(nobdsm_id);
	if(g_InterfaceEvents.CheckCheckbox(nonormal_id))
		g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictNormal = IsCheckboxOn(nonormal_id);
	if(g_InterfaceEvents.CheckCheckbox(nobeast_id))
		g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictBeast = IsCheckboxOn(nobeast_id);
	if(g_InterfaceEvents.CheckCheckbox(nogroup_id))
		g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictGroup = IsCheckboxOn(nogroup_id);
	if(g_InterfaceEvents.CheckCheckbox(nolesbian_id))
		g_Brothels.GetBrothel(g_CurrBrothel)->m_RestrictLesbian = IsCheckboxOn(nolesbian_id);
}
