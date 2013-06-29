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

#include "cGangs.h"
#include "XmlMisc.h"

namespace WhoreMasterRenewal
{

sGang::sGang()
{
    for( auto & value : m_Skills )
    {
        value = 0;
    }
    
    for( auto & value : m_Stats )
    {
        value = 0;
    }
}

sGang::~sGang()
{
    if( m_Next )
        delete m_Next;
        
    m_Prev = nullptr;
    m_Next = nullptr;
}

sGang& sGang::operator = ( const sGang& rhs )
{
    if( this != &rhs )
    {
        m_Num = rhs.m_Num;
        m_MissionID = rhs.m_MissionID;
        m_LastMissID = rhs.m_LastMissID;
        m_AutoRecruit = rhs.m_AutoRecruit;
        m_Name = rhs.m_Name;
        m_Combat = rhs.m_Combat;
        m_Events = rhs.m_Events;
        
        m_Next = rhs.m_Next;
        m_Prev = rhs.m_Prev;
        
        for( unsigned int i = 0; i < NUM_SKILLS; ++i )
        {
            m_Skills[i] = rhs.m_Skills[i];
        }
        
        for( auto i = 0; i < NUM_STATS; ++i )
        {
            m_Stats[i] = rhs.m_Stats[i];
        }
    }
    
    return *this;
}

TiXmlElement* sGang::SaveGangXML( TiXmlElement* pRoot )
{
    TiXmlElement* pGang = new TiXmlElement( "Gang" );
    pRoot->LinkEndChild( pGang );
    
    pGang->SetAttribute( "Num", m_Num );
    
    // save their skills
    SaveSkillsXML( pGang, m_Skills );
    
    // save their stats
    SaveStatsXML( pGang, m_Stats );
    
    // save their name
    pGang->SetAttribute( "Name", m_Name );
    return pGang;
}

bool sGang::LoadGangXML( TiXmlHandle hGang )
{
    TiXmlElement* pGang = hGang.ToElement();
    
    if( pGang == nullptr )
    {
        return false;
    }
    
    if( pGang->Attribute( "Name" ) )
    {
        m_Name = pGang->Attribute( "Name" );
    }
    
    // load their skills
    LoadSkillsXML( hGang.FirstChild( "Skills" ), m_Skills );
    
    // load their stats
    LoadStatsXML( hGang.FirstChild( "Stats" ), m_Stats );
    
    pGang->QueryIntAttribute( "Num", &m_Num );
    
    //these may not have been saved
    //if not, the query just does not set the value
    //so the default is used, assuming the gang was properly init
    {
        pGang->QueryValueAttribute<u_int>( "MissionID", &m_MissionID );
        pGang->QueryIntAttribute( "LastMissID", &m_LastMissID );
        
        // load the combat boolean
        pGang->QueryValueAttribute<bool>( "Combat", &m_Combat );
        
        // load the auto recruit boolean
        pGang->QueryValueAttribute<bool>( "AutoRecruit", &m_AutoRecruit );
    }
    return true;
}

} // namespace WhoreMasterRenewal
