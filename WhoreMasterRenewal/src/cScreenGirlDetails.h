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

#include "InterfaceWindowXML.hpp" // required inheritance

namespace WhoreMasterRenewal
{

class cScreenGirlDetails;
extern cScreenGirlDetails g_GirlDetails;

class Girl;

class cScreenGirlDetails : public cInterfaceWindowXML
{
public:
    cScreenGirlDetails();
    virtual ~cScreenGirlDetails();
    
    void Free();
    
    void init();
    void process();
    void check_events();
    bool check_keys();
    
private:
    void set_ids();
    bool GirlDead( Girl* dgirl );
    void RefreshJobList();
    void PrevGirl();
    void NextGirl();
    Girl *get_prev_girl();
    Girl *get_next_girl();
    Girl *remove_selected_girl();
    bool do_take_gold( Girl *girl, std::string &message );
    void take_gold( Girl *girl );
    
    
    
    char buffer[256];
    
    static bool ids_set;
    
    /*
     *  interface/event IDs
     */
    int back_id = 0;        // Back button
    
    int girlname_id = 0;    // Girl Name text
    int girldesc_id = 0;    // Girl Description text
    int girlimage_id = 0;   // Girl image
    int more_id = 0;        // More button
    int antipreg_id = 0;    // Use Anti-preg checkbox
    int prev_id = 0;        // Prev button
    int next_id = 0;        // Next button
    int accomup_id = 0;     // Accom Up button
    int accomdown_id = 0;   // Accom Down button
    int inventory_id = 0;   // Inventory button
    int senddungeon_id = 0; // Send To Dungeon button
    int reldungeon_id = 0;  // Release From Dungeon button
    int interact_id = 0;    // Interact button
    int takegold_id = 0;    // Take Gold button
    int houseperc_id = 0;   // House Percentage slider
    int housepercval_id = 0;// House Percentage value text
    int gallery_id = 0;     // Gallery button
    
    int jobtypehead_id = 0; // Job Types header text
    int jobtypelist_id = 0; // Job Types listbox
    int jobhead_id = 0;     // Job header text
    int joblist_id = 0;     // Job listbox
    int day_id = 0;         // Day button
    int night_id = 0;       // Night button
    int traithead_id = 0;   // Trait header text
    int traitlist_id = 0;   // Trait listbox
    int traitdesc_id = 0;   // Trait Description text
};

} // namespace WhoreMasterRenewal

#endif // CSCREENGIRLDETAILS_H_INCLUDED_1518
