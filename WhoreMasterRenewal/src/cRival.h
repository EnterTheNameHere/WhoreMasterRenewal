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
#ifndef CRIVAL_H_INCLUDED_1520
#define CRIVAL_H_INCLUDED_1520
#pragma once

#include "cNameList.h" // required cDoubleNameList

#include <string>

class TiXmlElement;
class TiXmlHandle;

namespace WhoreMasterRenewal
{

class cRival
{
public:
    cRival();
    ~cRival();
    
    cRival( const cRival& ) = delete;
	cRival& operator = ( const cRival& ) = delete;
    
    // variables
    int m_NumGangs = 3;
    int m_NumBrothels = 1;
    int m_NumGirls = 8;
    int m_NumBars = 0;
    int m_NumGamblingHalls = 0;
    long m_Gold = 5000;
    int m_BusinessesExtort = 0;
    long m_BribeRate = 0;
    int m_Influence = 0;    // based on the bribe rate this is the percentage of influence you have
    std::string m_Name = "Default cRival::m_Name value";
    
    cRival* m_Next = nullptr;
    cRival* m_Prev = nullptr;
};

class cRivalManager
{
public:
    cRivalManager();
    ~cRivalManager();
    
    cRivalManager( const cRivalManager& ) = delete;
	cRivalManager& operator = ( const cRivalManager& ) = delete;
    
    void Free();
    
    void Update( int& NumPlayerBussiness );
    cRival* GetRandomRival();
    cRival* GetRivals()
    {
        return m_Rivals;
    }
    cRival* GetRival( std::string name );
    cRival* GetRival( int number );
    
    TiXmlElement* SaveRivalsXML( TiXmlElement* pRoot );
    bool LoadRivalsXML( TiXmlHandle hRivalManager );
    void LoadRivalsLegacy( std::ifstream& ifs );
    
    void CreateRival( long bribeRate, int extort, long gold, int bars, int gambHalls, int Girls, int brothels, int goons );
    void AddRival( cRival* rival );
    void RemoveRival( cRival* rival );
    void CreateRandomRival();
    
    int GetNumBusinesses();
    
    int GetNumRivals()
    {
        return m_NumRivals;
    }
    
    bool NameExists( std::string name );
    
    bool player_safe()
    {
        return m_PlayerSafe;
    }
    cRival* get_influential_rival();
    void rivals_plunder_pc_gold( cRival* rival, std::string& message );
    
private:
    int m_NumRivals = 0;
    cRival* m_Rivals = nullptr;
    cRival* m_Last = nullptr;
    
    bool m_PlayerSafe = true;
    
    cDoubleNameList m_Names = {};
};

} // namespace WhoreMasterRenewal

#endif // CRIVAL_H_INCLUDED_1520
