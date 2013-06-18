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

namespace WhoreMasterRenewal
{

class cScreenItemManagement;
extern cScreenItemManagement g_ItemManagement;

class cScreenItemManagement : public cInterfaceWindowXML
{
public:
    enum Side
    {
        Left    = 0,
        Right   = 1
    };
    
    cScreenItemManagement();
    virtual ~cScreenItemManagement();
    
    void init();
    void process();
    void check_events();
    void attempt_transfer( Side transfer_from );
    void refresh_item_list( Side which_list );
    
    // XML screen replacement for cSelector, for multi-select listboxes
    int multi_left_first();
    int multi_left_next();
    int multi_right_first();
    int multi_right_next();

private:
    void set_ids();
    
    
    
    char buffer[256];
    int sel_pos_l = -2;
    int sel_pos_r = -2;
    
    static bool ids_set;
    /*
     *  interface/event IDs
     */
    int curbrothel_id = 0;  // Current Brothel text
    int back_id = 0;        // Back button
    int shift_l_id = 0;     // Shift Item Left button
    int shift_r_id = 0;     // Shift Item Right button
    int gold_id = 0;        // Player Gold text
    int desc_id = 0;        // Item Description text
    int owners_l_id = 0;    // Owners Left list
    int items_l_id = 0;     // Items Left list
    int owners_r_id = 0;    // Owners Right list
    int items_r_id = 0;     // Items Right list
    int filter_id = 0;      // Filter list
    int equip_l_id = 0;     // Equip Left button
    int unequip_l_id = 0;   // Unequip Left button
    int equip_r_id = 0;     // Equip Right button
    int unequip_r_id = 0;   // Unequip Right button
};

} // namespace WhoreMasterRenewal

#endif // CSCREENITEMMANAGEMENT_H_INCLUDED_1514
