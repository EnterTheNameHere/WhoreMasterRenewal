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
#ifndef CSCREENTOWN_H_INCLUDED_1513
#define CSCREENTOWN_H_INCLUDED_1513
#pragma once

#include "InterfaceWindowXML.hpp" // required inheritance

namespace WhoreMasterRenewal
{

class cScreenTown;
extern cScreenTown g_TownScreen;

class Girl;

class cScreenTown : public cInterfaceWindowXML
{
public:
	cScreenTown();
	virtual ~cScreenTown();

	void init();
	void process();
	void more_button();
	void release_button();
	void update_details();
	Girl* get_selected_girl();
	void selection_change();
	bool check_keys();
	void do_walk();
    std::string walk_no_luck();
    
private:
    void set_ids();
	void check_brothel(int BrothelNum);
	
	
	
	int BuyBrothel = -1;
	bool GetName = false;

    static bool ids_set;
/*
 *	interface/event IDs
 */
	int back_id = 0;		// Back button
	int walk_id = 0;		// Walk Around Town button
	int curbrothel_id = 0;	// Current Brothel text
	int slavemarket_id = 0;	// Slave Market map button
	int shop_id = 0;		// Shop map button
	int mayor_id = 0;		// Mayor's Office map button
	int bank_id = 0;		// Bank map button
	int house_id = 0;		// Player's House map button
	int prison_id = 0;		// Town Prison map button
	int brothel0_id = 0;	// Brothel 0 map button
	int brothel1_id = 0;	// Brothel 1 map button
	int brothel2_id = 0;	// Brothel 2 map button
	int brothel3_id = 0;	// Brothel 3 map button
	int brothel4_id = 0;	// Brothel 4 map button
	int brothel5_id = 0;	// Brothel 5 map button

	bool m_first_walk = true;
};

} // namespace WhoreMasterRenewal

#endif // CSCREENTOWN_H_INCLUDED_1513
