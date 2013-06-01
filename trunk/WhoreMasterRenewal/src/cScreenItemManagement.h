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
#ifndef CSCREENITEMMANAGEMENT_H_INCLUDED_1514
#define CSCREENITEMMANAGEMENT_H_INCLUDED_1514
#pragma once

#include "cInterfaceWindow.h" // required inheritance

class cScreenItemManagement : public cInterfaceWindowXML
{
public:

private:
	char	buffer[256];
	int		sel_pos_l;
	int		sel_pos_r;

	enum Side {
		Left	= 0,
		Right	= 1
	};

	static bool ids_set;
/*
 *	interface/event IDs
 */
	int curbrothel_id;	// Current Brothel text
	int back_id;		// Back button
	int shift_l_id;		// Shift Item Left button
	int shift_r_id;		// Shift Item Right button
	int gold_id;		// Player Gold text
	int desc_id;		// Item Description text
	int owners_l_id;	// Owners Left list
	int items_l_id;		// Items Left list
	int owners_r_id;	// Owners Right list
	int items_r_id;		// Items Right list
	int filter_id;		// Filter list
	int equip_l_id;		// Equip Left button
	int unequip_l_id;	// Unequip Left button
	int equip_r_id;		// Equip Right button
	int unequip_r_id;	// Unequip Right button

	void set_ids();
public:
	cScreenItemManagement();
	virtual ~cScreenItemManagement();

	void init();
	void process();
	void check_events();
	void attempt_transfer(Side transfer_from);
	void refresh_item_list(Side which_list);

	// XML screen replacement for cSelector, for multi-select listboxes
	int multi_left_first();
	int multi_left_next();
	int multi_right_first();
	int multi_right_next();
};

#endif // CSCREENITEMMANAGEMENT_H_INCLUDED_1514
