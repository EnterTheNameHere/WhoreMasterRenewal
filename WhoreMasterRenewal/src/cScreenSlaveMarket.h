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
#ifndef CSCREENSLAVEMARKET_H_INCLUDED_1513
#define CSCREENSLAVEMARKET_H_INCLUDED_1513
#pragma once

#include "cInterfaceWindow.h" // required inheritance

namespace WhoreMasterRenewal
{

class cScreenSlaveMarket;
extern cScreenSlaveMarket g_SlaveMarket;

class cScreenSlaveMarket : public cInterfaceWindowXML
{
public:
    cScreenSlaveMarket();
    virtual ~cScreenSlaveMarket();
    
    void init();
    void process();
    
    void generate_unique_girl( int i, bool &unique );
    bool check_events();
    bool check_keys();
    int &image_num() { return ImageNum; }
    int &selected_item() { return selection; }
    bool change_selected_girl();
    
    int multi_slave_first();
    int multi_slave_next();
    
private:
    void set_ids();
    
    
    
    int selection = -1;
    int DetailLevel = 0;
    int ImageNum = -1;
    char buffer[256];
    int sel_pos = 0;
    
    static bool ids_set;
    /*
     *  interface/event IDs
     */
    int back_id = 0;        // Back button
    int more_id = 0;        // More Details button
    int buy_slave_id = 0;   // Buy button
    int cur_brothel_id = 0; // Current Brothel text
    int slave_list_id = 0;  // Slave listbox
    int trait_list_id = 0;  // Trait listbox
    int details_id = 0;     // Girl Details text
    int trait_id = 0;       // Trait text
    int slave_image_id = 0; // Image of selected slave
    int header_id = 0;      // page header text ("Slave Market")
};

} // namespace WhoreMasterRenewal

#endif // CSCREENSLAVEMARKET_H_INCLUDED_1513
