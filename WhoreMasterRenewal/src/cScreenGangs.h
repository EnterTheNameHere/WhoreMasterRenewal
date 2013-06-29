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
#ifndef CSCREENGANGS_H_INCLUDED_1518
#define CSCREENGANGS_H_INCLUDED_1518
#pragma once

#include "InterfaceWindowXML.hpp" // required inheritance

namespace WhoreMasterRenewal
{

class cScreenGangs;
extern cScreenGangs g_GangManagement;

class cScreenGangs : public cInterfaceWindowXML
{
public:
    cScreenGangs();
    virtual ~cScreenGangs();
    
    void init();
    void process();
    void check_events();
    bool check_keys();
    
    int set_mission_desc( int mid );
    std::string mission_desc( int mid );
    std::string short_mission_desc( int mid );
    
    // XML screen replacement for cSelector, for multi-select listboxes
    int multi_first();
    int multi_next();
    
private:
    void set_ids();
    void hire_recruitable();
    
    
    
    char buffer[256];
    int sel_pos = 0;
    
    static bool ids_set;
    
    /*
     *  interface/event IDs
     */
    int back_id = 0;        // Back button
    int gangcost_id = 0;    // Gang Hiring Cost text
    int ganghire_id = 0;    // Hire Gang button
    int gangfire_id = 0;    // Fire Gang button
    int totalcost_id = 0;   // Weekly Cost text
    int ganglist_id = 0;    // Controlled Gangs list
    int missionlist_id = 0; // Missions list
    int gangdesc_id = 0;    // Gang Description text
    int missiondesc_id = 0; // Mission Description text
    int weaponlevel_id = 0; // Weapon Level text
    int weaponup_id = 0;    // Increase Weapon Level button
    int netdesc_id = 0;     // Nets Description text
    int netbuy_id = 0;      // Buy 20 Nets button
    int netautobuy_id = 0;  // Auto-buy Nets checkbox
    int healdesc_id = 0;    // Healing Potions Description text
    int healbuy_id = 0;     // Buy 20 Healing Potions button
    int healautobuy_id = 0; // Auto-buy Healing Potions checkbox
    int recruitlist_id = 0; // Recruitable Gangs list
};

} // namespace WhoreMasterRenewal

#endif // CSCREENGANGS_H_INCLUDED_1518
