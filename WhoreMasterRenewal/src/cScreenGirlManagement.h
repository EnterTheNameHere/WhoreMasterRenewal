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
#ifndef CSCREENGIRLMANAGEMENT_H_INCLUDED_1517
#define CSCREENGIRLMANAGEMENT_H_INCLUDED_1517
#pragma once

#include "InterfaceWindowXML.hpp" // required inheritance

namespace WhoreMasterRenewal
{

class cScreenGirlManagement;
extern cScreenGirlManagement g_GirlManagement;

class Girl;

class cScreenGirlManagement : public cInterfaceWindowXML
{
public:
	cScreenGirlManagement();
	virtual ~cScreenGirlManagement();

	void init();
	void process();
	void check_events();
	bool check_keys();
	void update_image();

private:
    void set_ids();
	bool GirlDead(Girl* dgirl);
	void RefreshSelectedJobType();
	void RefreshJobList();
	void GetSelectedGirls(std::vector<int> *girl_array);
	void ViewSelectedGirl();
	
	
	
    char buffer[256];

	static bool ids_set;
/*
 *	interface/event IDs
 */
	int back_id = 0;		// Back button
	int curbrothel_id = 0;	// Current Brothel text
	int girllist_id = 0;	// Girls listbox
	int girlimage_id = 0;	// Girl image
	int girldesc_id = 0;	// Girl Description text
	int transfer_id = 0;	// Transfer Girl button
	int firegirl_id = 0;	// Fire Girl button
	int freeslave_id = 0;	// Free Slave Button
	int viewdetails_id = 0;	// View Details button
	int sellslave_id = 0;	// Sell Slave button
	int jobtypehead_id = 0;	// Job Types header text
	int jobtypelist_id = 0;	// Job Types listbox
	int jobtypedesc_id = 0;	// Job Types Description text
	int jobhead_id = 0;		// Job header text
	int joblist_id = 0;		// Job listbox
	int jobdesc_id = 0;		// Job Description text
	int day_id = 0;			// Day button
	int night_id = 0;		// Night button
};

} // namespace WhoreMasterRenewal

#endif // CSCREENGIRLMANAGEMENT_H_INCLUDED_1517
