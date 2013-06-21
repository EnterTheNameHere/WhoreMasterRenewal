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
#ifndef CBROTHEL_H_INCLUDED_1533
#define CBROTHEL_H_INCLUDED_1533
#pragma once

#include "cGold.h" // required cGold
#include "cBuilding.h" // required cBuilding
#include "cEvents.h" // required cEvents

#include <string>
#include <vector>
#include <sstream>
#include <fstream>

class TiXmlElement;
class TiXmlHandle;

namespace WhoreMasterRenewal
{

class Girl;



// defines a single brothel
class Brothel
{
public:
    Brothel();                         // constructor
    ~Brothel();                        // destructor
    
    Brothel( const Brothel& ) = delete;
    Brothel& operator = ( const Brothel& ) = delete;
    
    // mod docclox - convenience funcs
    int free_rooms();
    bool matron_on_shift( int shift );
    bool has_matron();
    int matron_count();
    // end mod
    TiXmlElement* SaveBrothelXML( TiXmlElement* pRoot );
    bool LoadBrothelXML( TiXmlHandle hBrothel );
    void AddGirl( Girl* pGirl );
    
    
    
    int m_id = 0;
    std::string m_Name = "";
    unsigned short m_Happiness = 0;        // av. % happy customers last week
    unsigned short m_TotalCustomers = 0;   // the total number of customers for the last week
    unsigned short m_MiscCustomers = 0;    // customers used for temp purposes but must still be taken into account
    unsigned char m_Fame = 0;             // How famous this brothel is
    unsigned char m_NumRooms = 0;         // How many rooms it has
    unsigned char m_Bar = 0;              // level of bar: 0 => none
    unsigned char m_GamblingHall = 0;     // as above
    unsigned char m_NumGirls = 0;         // How many girls are here
    unsigned short m_AdvertisingBudget = 0;// Budget player has set for weekly advertising
    double m_AdvertisingLevel = 0.0; // multiplier for how far budget goes, based on girls working in advertising
    
    cBuilding m_Building = {};
    
    // for keeping track of how well the place is doing (for the last week)
    cGold m_Finance = {};
    
    unsigned char m_HasGambStaff = 0;     // gambling hall or
    unsigned char m_HasBarStaff = 0;      // Bar staff. Not as good as girls but consistent
    
    // non private variables (just cause it makes life so much easier)
    bool m_RestrictAnal = false;
    bool m_RestrictBDSM = false;
    bool m_RestrictBeast = false;
    bool m_RestrictGroup = false;
    bool m_RestrictNormal = false;
    bool m_RestrictLesbian = false;
    
    int m_Filthiness = 0;
    
    cEvents m_Events = {};
    
    int m_BuildingQuality[NUMJOBTYPES];
    
    Girl* m_Girls = nullptr;                // A list of all the girls this place has
    Girl* m_LastGirl = nullptr;
    Brothel* m_Next = nullptr;
    
    int m_SecurityLevel = 0;
};

} // namespace WhoreMasterRenewal

#endif  // CBROTHEL_H_INCLUDED_1533
