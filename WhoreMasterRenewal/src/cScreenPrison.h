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

namespace WhoreMasterRenewal
{

class cScreenPrison;
extern cScreenPrison g_PrisonScreen;

class Girl;

class cScreenPrison : public cInterfaceWindowXML
{
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
    
private:
    void set_ids();
    
    
    
    int selection = -1;
    int DetailLevel = 0;
    char buffer[256];
    
    static bool ids_set;
    /*
     *  interface/event IDs
     */
    int header_id = 0;  // screen header
    int back_id = 0;    // Back button
    int more_id = 0;    // More button
    int release_id = 0; // Release button
    int prison_list_id = 0; // Prisoner list
    int girl_desc_id = 0;   // Girl description
};

} // namespace WhoreMasterRenewal

#endif // CSCREENPRISON_H_INCLUDED_1514
