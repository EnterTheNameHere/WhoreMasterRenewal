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

#include "cTraits.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdio>

sTrait::sTrait()
{
    m_Name = nullptr;
    m_Desc = nullptr;
    m_Next = nullptr;
}

sTrait::~sTrait()
{
    if( m_Name )
        delete [] m_Name;
        
    m_Name = nullptr;
    
    if( m_Desc )
        delete [] m_Desc;
        
    m_Desc = nullptr;
    
    if( m_Next )
        delete m_Next;
        
    m_Next = nullptr;
}

cTraits::cTraits()
{
    m_ParentTrait = nullptr;
    m_LastTrait = nullptr;
    m_NumTraits = 0;
}

cTraits::~cTraits()
{
    Free();
}

void cTraits::Free()
{
    if( m_ParentTrait )
        delete m_ParentTrait;
        
    m_ParentTrait = nullptr;
    m_LastTrait = nullptr;
}

void cTraits::LoadTraits( std::string filename )
{
    std::ifstream in;
    in.open( filename.c_str() );
    char* pt = nullptr, buffer[500];
    sTrait* newTrait = nullptr;
    
    while( in.good() )
    {
        newTrait = new sTrait;
        
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        in.getline( buffer, sizeof( buffer ), '\n' );   // get the name
        
        if( ( pt = strrchr( buffer, '\r' ) ) )
        {
            *pt = 0;
        }
        
        newTrait->m_Name = new char[strlen( buffer ) + 1];
        strcpy( newTrait->m_Name, buffer );
        
        if( in.peek() == '\n' ) in.ignore( 1, '\n' );
        
        if( in.peek() == '\r' ) in.ignore( 1, '\r' );
        
        in.getline( buffer, sizeof( buffer ), '\n' );   // get the description
        
        if( ( pt = strrchr( buffer, '\r' ) ) )
        {
            *pt = 0;
        }
        
        if( strcmp( buffer, "na" ) != 0 )
        {
            newTrait->m_Desc = new char[strlen( buffer ) + 1];
            strcpy( newTrait->m_Desc, buffer );
        }
        else
            newTrait->m_Desc = nullptr;
            
        AddTrait( newTrait );
        newTrait = nullptr;
    }
    
    in.close();
}

void cTraits::SaveTraits( std::string filename )
{
    std::ofstream out;
    sTrait* current = m_ParentTrait;
    
    out.open( filename.c_str() );
    
    while( current != nullptr )
    {
        out << current->m_Name << "\n";
        out << current->m_Desc << "\n";
        current = current->m_Next;
    }
    
    out.flush();
    out.close();
}

void cTraits::AddTrait( sTrait* trait )
{
    trait->m_Next = nullptr;
    
    if( m_ParentTrait )
        m_LastTrait->m_Next = trait;
    else
        m_ParentTrait = trait;
        
    m_LastTrait = trait;
    m_NumTraits++;
}

void cTraits::RemoveTrait( std::string name )
{
    if( m_ParentTrait == nullptr )
        return;
        
    sTrait* current = m_ParentTrait;
    sTrait* last = nullptr;
    
    if( strcmp( current->m_Name, name.c_str() ) == 0 )
    {
        m_ParentTrait = current->m_Next;
        current->m_Next = nullptr;
        delete current;
    }
    else
    {
        last = current ;
        current = current->m_Next;
        
        while( strcmp( current->m_Name, name.c_str() ) == 0 )
        {
            last = current ;
            current = current->m_Next;
        }
        
        last->m_Next = current->m_Next;
        
        current->m_Next = nullptr;
        delete current;
    }
    
    m_NumTraits--;
    current = nullptr;
    last = nullptr;
}

sTrait* cTraits::GetTrait( std::string name )
{
    if( m_ParentTrait == nullptr )
        return nullptr;
        
    sTrait* current = m_ParentTrait;
    
    /*
     *  MOD docclox
     *  changed the way the loop works to make it easier to include
     *  debug print statements
     */
    // g_LogFile.os() << "Looking up trait '" << name << "'" << std::endl;
    for( current = m_ParentTrait; current; current = current->m_Next )
    {
        /*
        g_LogFile.os()  << "    testing '"
            << current->m_Name
            <<"'"
            << std::endl;
        */
        if( name == current->m_Name )
        {
            //g_LogFile.os() << "       gotit!" << std::endl;
            return current;
        }
        
        //g_LogFile.os() << "       nope!" << std::endl;
    }
    
    // END MOD
    return nullptr;
}

sTrait* cTraits::GetTraitNum( int num )
{
    int count = 0;
    
    if( m_ParentTrait == nullptr )
        return nullptr;
        
    sTrait* current = m_ParentTrait;
    
    while( current )
    {
        if( count == num )
            break;
            
        count++;
        
        current = current->m_Next;
        
        if( current == nullptr )
            break;
    }
    
    return current;
}
