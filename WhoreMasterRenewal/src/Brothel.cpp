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

#include "Brothel.h"
#include "Helper.hpp"
#include "cGangs.h"
#include "DirPath.h"
#include "cMessageBox.h"
#include "strnatcmp.h"
#include "XmlMisc.h"
#include "cInventory.h"
#include "cGirls.h"
#include "cCustomers.h"
#include "cRng.h"
#include "CLog.h"
#include "cTariff.h"
#include "InterfaceProcesses.h"
#include "InterfaceGlobals.h"
#include "Girl.hpp"
#include "BrothelManager.hpp"

#include <sstream>

namespace WhoreMasterRenewal
{

Brothel::Brothel()
{
    for( unsigned int i = 0; i < NUMJOBTYPES; i++ )
        m_BuildingQuality[i] = 0;
}

Brothel::~Brothel()
{
    if( m_Next )
        delete m_Next;
        
    m_Next                  = nullptr;
    
    if( m_Girls )
        delete m_Girls;
        
    m_LastGirl              = nullptr;
    m_Girls                 = nullptr;
}

int Brothel::free_rooms()
{
    return m_NumRooms - m_NumGirls;
}

// ----- Matron
bool Brothel::matron_on_shift( int shift )
{
    return g_Brothels.GetNumGirlsOnJob( m_id, JOB_MATRON, shift == SHIFT_DAY ) > 0;
}

int Brothel::matron_count()
{
    int i, sum = 0;
    
    for( i = 0; i < 2; i++ )
    {
        if( g_Brothels.GetNumGirlsOnJob( m_id, JOB_MATRON, ( i == 0 ) ) > 0 )
        {
            sum ++;
        }
    }
    
    return sum;
}

bool Brothel::has_matron()
{
    int i;
    
    for( i = 0; i < 2; i++ )
    {
        if( g_Brothels.GetNumGirlsOnJob( m_id, JOB_MATRON, ( i == 0 ) ) > 0 )
        {
            return true;
        }
    }
    
    return false;
}

bool Brothel::LoadBrothelXML( TiXmlHandle hBrothel )
{
    //no need to init this, we just created it
    TiXmlElement* pBrothel = hBrothel.ToElement();
    
    if( pBrothel == nullptr )
    {
        return false;
    }
    
    if( pBrothel->Attribute( "Name" ) )
    {
        m_Name = pBrothel->Attribute( "Name" );
    }
    
    int tempInt = 0;
    
    std::string message = "Loading brothel: ";
    message += m_Name;
    g_LogFile.write( message );
    
    // load variables for sex restrictions
    pBrothel->QueryValueAttribute<bool>( "RestrictAnal", &m_RestrictAnal );
    pBrothel->QueryValueAttribute<bool>( "RestrictBDSM", &m_RestrictBDSM );
    pBrothel->QueryValueAttribute<bool>( "RestrictBeast", &m_RestrictBeast );
    pBrothel->QueryValueAttribute<bool>( "RestrictGroup", &m_RestrictGroup );
    pBrothel->QueryValueAttribute<bool>( "RestrictNormal", &m_RestrictNormal );
    pBrothel->QueryValueAttribute<bool>( "RestrictLesbian", &m_RestrictLesbian );
    
    pBrothel->QueryValueAttribute<unsigned short>( "AdvertisingBudget", &m_AdvertisingBudget );
    pBrothel->QueryIntAttribute( "Bar", &tempInt );
    m_Bar = tempInt;
    tempInt = 0;
    pBrothel->QueryIntAttribute( "Fame", &tempInt );
    m_Fame = tempInt;
    tempInt = 0;
    pBrothel->QueryIntAttribute( "GamblingHall", &tempInt );
    m_GamblingHall = tempInt;
    tempInt = 0;
    pBrothel->QueryValueAttribute<unsigned short>( "Happiness", &m_Happiness );
    pBrothel->QueryIntAttribute( "HasBarStaff", &tempInt );
    m_HasBarStaff = tempInt;
    tempInt = 0;
    pBrothel->QueryIntAttribute( "id", &m_id );
    pBrothel->QueryIntAttribute( "HasGambStaff", &tempInt );
    m_HasGambStaff = tempInt;
    tempInt = 0;
    pBrothel->QueryIntAttribute( "NumRooms", &tempInt );
    m_NumRooms = tempInt;
    tempInt = 0;
    pBrothel->QueryIntAttribute( "Filthiness", &m_Filthiness );
    pBrothel->QueryIntAttribute( "SecurityLevel", &m_SecurityLevel );
    
    m_Finance.loadGoldXML( hBrothel.FirstChild( "Gold" ) );
    
    // Load girls
    m_NumGirls = 0;
    TiXmlElement* pGirls = pBrothel->FirstChildElement( "Girls" );
    
    if( pGirls )
    {
        for( TiXmlElement* pGirl = pGirls->FirstChildElement( "Girl" );
                pGirl != nullptr;
                pGirl = pGirl->NextSiblingElement( "Girl" ) ) // load each girl and add her
        {
            Girl* girl = new Girl();
            bool success = girl->LoadGirlXML( TiXmlHandle( pGirl ) );
            
            if( success == true )
            {
                AddGirl( girl );
            }
            else
            {
                delete girl;
                continue;
            }
        }
    }
    
    //commented out before the conversion to XML
    //building.load(ifs);
    return true;
}

void Brothel::AddGirl( Girl* pGirl )
{
    if( m_Girls )
    {
        pGirl->m_Prev = m_LastGirl;
        m_LastGirl->m_Next = pGirl;
        m_LastGirl = pGirl;
    }
    else
    {
        m_LastGirl = m_Girls = pGirl;
    }
    
    ++m_NumGirls;
}

TiXmlElement* Brothel::SaveBrothelXML( TiXmlElement* pRoot )
{
    TiXmlElement* pBrothel = new TiXmlElement( "Brothel" );
    pRoot->LinkEndChild( pBrothel );
    pBrothel->SetAttribute( "Name", m_Name );
    
    // save variables for sex restrictions
    pBrothel->SetAttribute( "RestrictAnal", m_RestrictAnal );
    pBrothel->SetAttribute( "RestrictBDSM", m_RestrictBDSM );
    pBrothel->SetAttribute( "RestrictBeast", m_RestrictBeast );
    pBrothel->SetAttribute( "RestrictGroup", m_RestrictGroup );
    pBrothel->SetAttribute( "RestrictNormal", m_RestrictNormal );
    pBrothel->SetAttribute( "RestrictLesbian", m_RestrictLesbian );
    
    pBrothel->SetAttribute( "AdvertisingBudget", m_AdvertisingBudget );
    pBrothel->SetAttribute( "Bar", m_Bar );
    pBrothel->SetAttribute( "Fame", m_Fame );
    pBrothel->SetAttribute( "GamblingHall", m_GamblingHall );
    pBrothel->SetAttribute( "Happiness", m_Happiness );
    pBrothel->SetAttribute( "HasBarStaff", m_HasBarStaff );
    pBrothel->SetAttribute( "id", m_id );
    pBrothel->SetAttribute( "HasGambStaff", m_HasGambStaff );
    pBrothel->SetAttribute( "NumRooms", m_NumRooms );
    pBrothel->SetAttribute( "Filthiness", m_Filthiness );
    pBrothel->SetAttribute( "SecurityLevel", m_SecurityLevel );
    
    m_Finance.saveGoldXML( pBrothel );
    
    // save building qualities
#if 0
    //building qualities appears to be unimplemented, why waste space
    TiXmlElement* pBuildingQualities = new TiXmlElement( "BuildingQualities" );
    pBrothel->LinkEndChild( pBuildingQualities );
    SaveJobsXML( pBuildingQualities, m_BuildingQuality );
#endif
    
    // Save Girls
    TiXmlElement* pGirls = new TiXmlElement( "Girls" );
    pBrothel->LinkEndChild( pGirls );
    Girl* girl = m_Girls;
    
    while( girl )
    {
        girl->SaveGirlXML( pGirls );
        girl = girl->m_Next;
    }
    
    /*
    *       save the building setup
    */
    //this was commented out before the conversion to XML
    //current->building.save(ofs, current->m_Name);
    return pBrothel;
}



} // namespace WhoreMasterRenewal
