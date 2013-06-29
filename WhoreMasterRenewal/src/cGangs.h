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
#ifndef CGANGS_H_INCLUDED_1530
#define CGANGS_H_INCLUDED_1530
#pragma once

#include "Constants.h"
#include "cEvents.h" // required cEvents

#include <string>

class TiXmlElement;
class TiXmlHandle;

namespace WhoreMasterRenewal
{

class Girl;

// A gang of street toughs
typedef struct sGang
{
public:
    sGang();
    ~sGang();
    
    sGang( const sGang& ) = delete;
    sGang& operator = ( const sGang& rhs );
    
    int magic() { return m_Skills[SKILL_MAGIC]; }
    int combat() { return m_Skills[SKILL_COMBAT]; }
    int intelligence() { return m_Stats[STAT_INTELLIGENCE]; }
    
    TiXmlElement* SaveGangXML( TiXmlElement* pRoot );
    bool LoadGangXML( TiXmlHandle hGang );
    
    
    int m_Num = 0;  // number in the gang
    unsigned char m_Skills[NUM_SKILLS]; // skills of the gang
    unsigned char m_Stats[NUM_STATS];   // stats of the gang
    u_int m_MissionID = MISS_GUARDING;  // the type of mission currently performing
    int m_LastMissID = -1;   // the last mission if auto changed to recruit mission
    bool m_AutoRecruit = false; // true if auto recruiting
    std::string m_Name = "Unnamed";
    bool m_Combat = false;  // is true when gang has seen combat in the last week
    cEvents m_Events = {};
    
    sGang* m_Next = nullptr;
    sGang* m_Prev = nullptr;
    
} sGang;



} // namespace WhoreMasterRenewal

#endif  // CGANGS_H_INCLUDED_1530
