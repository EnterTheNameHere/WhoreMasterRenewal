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
#ifndef CSCREENBUILDINGMANAGER_H_INCLUDED_1519
#define CSCREENBUILDINGMANAGER_H_INCLUDED_1519
#pragma once

#include "cInterfaceWindow.h" // required inheritance

namespace WhoreMasterRenewal
{

class cBuildingManagement;
extern cBuildingManagement g_BuildingManagementScreen;

class cBuilding;
class Brothel;

class cBuildingManagement : public cInterfaceWindowXML
{
public:
    struct IDBlock
    {
        int unit = 0;
        int shade = 0;
        int desc = 0;
        int type = 0;
        int rename = 0;
        int space = 0;
        int cost = 0;
        int bevel_t = 0;
        int bevel_b = 0;
        int bevel_l = 0;
        int bevel_r = 0;
        int newsplash = 0;
        int delete_btn = 0;
        
        int glitz_cap = 0;
        int glitz_level = 0;
        int glitz_down = 0;
        int glitz_up = 0;
        int provides_cap = 0;
        int provides_level = 0;
        int provides_down = 0;
        int provides_up = 0;
        int secure_cap = 0;
        int secure_level = 0;
        int secure_down = 0;
        int secure_up = 0;
        int stealth_cap = 0;
        int stealth_level = 0;
        int stealth_down = 0;
        int stealth_up = 0;
        
        void hide( cBuildingManagement *mgr );
        void display( cBuildingManagement *m, int n, cBuilding *b );
    };
    
    
    
    cBuildingManagement();
    virtual ~cBuildingManagement();
    
    cBuildingManagement( const cBuildingManagement& ) = delete;
    cBuildingManagement& operator = ( const cBuildingManagement& ) = delete;
    
    void init();
    void process();
    void buy_button();
    void revert_button();
    bool check_keys();
    void new_facility();
    
    void rename_button( int facility_idx );
    void delete_button( int facility_idx );
    void glitz_up( int facility_idx );
    void glitz_down( int facility_idx );
    void secure_down( int facility_idx );
    void secure_up( int facility_idx );
    void stealth_down( int facility_idx );
    void stealth_up( int facility_idx );
    
private:
    void set_ids();
    
    
    
    Brothel* brothel = nullptr;
    cBuilding* building = nullptr;
    int selection = 0;
    int screen = 0;
    
    static bool ids_set;
    /*
     *  interface/event IDs
     */
    int header_id = 0;  // screen header - changes with brothel
    int back_id = 0;    // back button
    int capacity_id = 0;    // capacity/used/free line
    int listbox_id = 0; // big listbox
    int narrative_id = 0;   // narrative text at bottom
    int new_id = 0;     // new button
    int gold_id = 0;    // gold display
    int total_cost_id = 0;  // total cost
    int buy_button_id = 0;
    int revert_button_id = 0;
    IDBlock blocks[6];
    
    int total_cost = 0;
};

} // namespace WhoreMasterRenewal

#endif // CSCREENBUILDINGMANAGER_H_INCLUDED_1519
