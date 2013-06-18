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
#ifndef CSCREENBUILDINGSETUP_H_INCLUDED_1518
#define CSCREENBUILDINGSETUP_H_INCLUDED_1518
#pragma once

#include "cInterfaceWindow.h" // required inheritance

namespace WhoreMasterRenewal
{

class cScreenBuildingSetup;
extern cScreenBuildingSetup g_BuildingSetupScreen;

class cScreenBuildingSetup : public cInterfaceWindowXML
{
public:
	cScreenBuildingSetup();
	virtual ~cScreenBuildingSetup();

	void init();
	void process();
	void check_events();

private:
    void set_ids();
    
    
    
	char buffer[256];

	static bool ids_set;
/*
 *	interface/event IDs
 */
    int back_id = 0;        // Back button
	int curbrothel_id = 0;	// Current Brothel text
	int potioncost_id = 0;	// Potion Cost text
	int potionavail_id = 0;	// Available Potions text
	int potions10_id = 0;	// Buy 10 Potions button
	int potions20_id = 0;	// Buy 20 Potions button
	int autopotions_id = 0;	// Auto Buy Potions checkbox
	int barstaff_id = 0;	// Bar Staff Cost text
	int barhire_id = 0;		// Hire Bar Staff button
	int barfire_id = 0;		// Fire Bar Staff button
	int casinostaff_id = 0;	// Casino Staff Cost text
	int casinohire_id = 0;	// Hire Casino Staff button
	int casinofire_id = 0;	// Fire Casino Staff button
	int roomcost_id = 0;	// Room Cost text
	int buyrooms_id = 0;	// Add 5 Rooms button
	int restrict_id = 0;	// Sex Restrictions text
	int noanal_id = 0;		// Prohibit Anal checkbox
	int nobdsm_id = 0;		// Prohibit BDSM checkbox
	int nonormal_id = 0;	// Prohibit Normal Sex checkbox
	int nobeast_id = 0;		// Prohibit Bestiality checkbox
	int nogroup_id = 0;		// Prohibit Group Sex checkbox
	int nolesbian_id = 0;	// Prohibit Lesbian checkbox
	int advertsli_id = 0;	// Advertising Budget slider
	int advertamt_id = 0;	// Advertising Budget value output text
};

} // namespace WhoreMasterRenewal

#endif // CSCREENBUILDINGSETUP_H_INCLUDED_1518
