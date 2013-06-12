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
#ifndef CSCREENPRISON_H_INCLUDED_1514
#define CSCREENPRISON_H_INCLUDED_1514
#pragma once

#include "cInterfaceWindow.h" // required inheritance

class cScreenPrison;
extern cScreenPrison g_PrisonScreen;

class Girl;

class cScreenPrison : public cInterfaceWindowXML
{
public:

private:
	int		selection;
	int		DetailLevel;
	char	buffer[256];

static	bool		ids_set;
/*
 *	interface/event IDs
 */
	int		header_id;	// screen header
	int		back_id;	// Back button
	int		more_id;	// More button
	int		release_id;	// Release button
	int		prison_list_id;	// Prisoner list
	int		girl_desc_id;	// Girl description

	void		set_ids();
public:
	cScreenPrison();
	virtual ~cScreenPrison();

	void init();
	void process();
	void more_button();
	void release_button();
	void update_details();
	Girl* get_selected_girl();
	void selection_change();
	bool check_keys();

};

#endif // CSCREENPRISON_H_INCLUDED_1514