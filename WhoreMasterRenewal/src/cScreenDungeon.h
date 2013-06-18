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
#ifndef CSCREENDUNGEON_H_INCLUDED_1518
#define CSCREENDUNGEON_H_INCLUDED_1518
#pragma once

#include "cInterfaceWindow.h" // required inheritance

namespace WhoreMasterRenewal
{

class cScreenDungeon;
extern cScreenDungeon g_Dungeon;

class Girl;
struct sGang;

extern int g_CurrBrothel;

class cScreenDungeon : public cInterfaceWindowXML
{
public:
    cScreenDungeon();
    virtual ~cScreenDungeon();
    
    enum
    {
        GirlLoses = 0,
        GirlWins  = 1
    };
    
    bool    torture_possible();
    int     enslave();
    int     enslave_customer( int girls_removed, int custs_removed );
    int     process_events();
    int     view_girl();
    void    init();
    void    process();
    void    selection_change();
    void    set_slave_stats( Girl *girl );
    void    release_all_customers();
    void    release_all_girls();
    void    sell_slaves();
    void    stop_feeding();
    void    start_feeding();
    void    torture_customer( int girls_removed );
    void    torture();
    void    release();
    void    talk();
    void    get_selected_girls( std::vector<int> *girl_array );
    void    store_selected_girls();
    void    check_gang_for_survivors( Girl *girl, sGang *gang, std::string &message, int start_count );
    
// WD   Replaced by Doclox's cGirlTorture and cGirlGangFight code.
//  int     girl_fights_gang(Girl *girl, sGang *&gang);
//  bool    girl_fight_torture(Girl *girl, std::string &message, bool &fight);


    // XML screen replacement for cSelector, for multi-select listboxes
    int multi_first();
    int multi_next();
    
private:
    void set_ids();
    
    
    
    char buffer[256];
    int sel_pos = 0;
    
    static bool ids_set;
    /*
     *  interface/event IDs
     */
    int back_id = 0;        // Back button
    int girllist_id = 0;    // Girls listbox
    int header_id = 0;      // Dungeon header text
    int brandslave_id = 0;  // Brand Slave button
    int release_id = 0;     // Release button
    int allowfood_id = 0;   // Allow Food button
    int torture_id = 0;     // Torture button
    int stopfood_id = 0;    // Stop Feeding button
    int interact_id = 0;    // Interact With button
    int releaseall_id = 0;  // Release All Girls button
    int releasecust_id = 0; // Release All Customers button
    int viewdetails_id = 0; // Girl Details button
    int sellslave_id = 0;   // Sell Slave button
    
    int selection = -1;
    bool no_guards = false;
    enum
    {
        Return,
        Continue
    };
};

} // namespace WhoreMasterRenewal

#endif // CSCREENDUNGEON_H_INCLUDED_1518
