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
#ifndef CSCREENGIRLDETAILS_H_INCLUDED_1518
#define CSCREENGIRLDETAILS_H_INCLUDED_1518
#pragma once

#include "cInterfaceWindow.h" // required inheritance

class cScreenGirlDetails;
extern cScreenGirlDetails g_GirlDetails;

class Girl;

class cScreenGirlDetails : public cInterfaceWindowXML
{
public:
	void Free();

private:
	char	buffer[256];

	static bool ids_set;
/*
 *	interface/event IDs
 */
	int back_id;		// Back button

	int girlname_id;	// Girl Name text
	int girldesc_id;	// Girl Description text
	int girlimage_id;	// Girl image
	int more_id;		// More button
	int antipreg_id;	// Use Anti-preg checkbox
	int prev_id;		// Prev button
	int next_id;		// Next button
	int accomup_id;		// Accom Up button
	int accomdown_id;	// Accom Down button
	int inventory_id;	// Inventory button
	int senddungeon_id;	// Send To Dungeon button
	int reldungeon_id;	// Release From Dungeon button
	int interact_id;	// Interact button
	int takegold_id;	// Take Gold button
	int houseperc_id;	// House Percentage slider
	int housepercval_id;// House Percentage value text
	int gallery_id;		// Gallery button

	int jobtypehead_id;	// Job Types header text
	int jobtypelist_id;	// Job Types listbox
	int jobhead_id;		// Job header text
	int joblist_id;		// Job listbox
	int day_id;			// Day button
	int night_id;		// Night button
	int traithead_id;	// Trait header text
	int traitlist_id;	// Trait listbox
	int traitdesc_id;	// Trait Description text

	void set_ids();
	bool GirlDead(Girl* dgirl);
	void RefreshJobList();
	void PrevGirl();
	void NextGirl();
	Girl *get_prev_girl();
	Girl *get_next_girl();
	Girl *remove_selected_girl();
	bool do_take_gold(Girl *girl, std::string &message);
	void take_gold(Girl *girl);
public:
	cScreenGirlDetails();
	virtual ~cScreenGirlDetails();

	void init();
	void process();
	void check_events();
	bool check_keys();
};

#endif // CSCREENGIRLDETAILS_H_INCLUDED_1518
