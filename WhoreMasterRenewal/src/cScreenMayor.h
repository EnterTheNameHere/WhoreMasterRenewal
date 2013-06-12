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
#ifndef CSCREENMAYOR_H_INCLUDED_1514
#define CSCREENMAYOR_H_INCLUDED_1514
#pragma once

#include "cInterfaceWindow.h" // required inheritance

namespace WhoreMasterRenewal
{

class cScreenMayor;
extern cScreenMayor g_MayorsOfficeScreen;

class cScreenMayor : public cInterfaceWindowXML
{
public:

private:
	bool	SetBribe;
	char	buffer[256];

	static bool ids_set;
/*
 *	interface/event IDs
 */
	int back_id;		// Back button
	int bribe_id;		// Bribe button
	int details_id;		// Mayor Details text
	int header_id;		// page header text ("Town Hall")

	void set_ids();
public:
	cScreenMayor();
	virtual ~cScreenMayor();

	void init();
	void process();
	void check_events();
};

} // namespace WhoreMasterRenewal

#endif // CSCREENMAYOR_H_INCLUDED_1514
