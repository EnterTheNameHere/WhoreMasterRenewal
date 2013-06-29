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

#include "cNameList.h"
#include "Constants.h"
#include "CLog.h"
#include "cRng.h"

#include <fstream>

namespace WhoreMasterRenewal
{

cNameList::cNameList()
{
    ;
}

cNameList::cNameList( std::string file )
{
    load( file );
}

void cNameList::load( std::string file )
{
    std::ifstream in;
    char buff[1024];
    bool first = true;
    /*
     *  open the file
     */
    in.open( file.c_str() );
    
    if( !in )
    {
        g_LogFile.ss()
                << "Error: unable to open file '"
                << file
                << "'"
                ;
        g_LogFile.ssend();
        return;
    }
    
    m_Names.clear();
    
    /*
     *  loop until the stream breaks
     */
    for( ;; )
    {
        in.getline( buff, sizeof( buff ) - 1 );
        
        /*
         *      before we do anythign with the string
         *      test to see if there's an error flag
         */
        if( !in.good() )
        {
            break;      // ok, we're done
        }
        
        /*
         *      the first line is the number of names
         *      we don't need it any more, so skip over it
         */
        if( first )
        {
            first = false;
            continue;
        }
        
        std::string s = buff;
        /*
         *      just for linux (although will do no harm on windows)
         *      we need to check for a carriage return char at the
         *      end of the line and remove it if found
         */
        int last = s.length() - 1;
        
        if( s[last] == '\r' )
        {
            s.erase( last );
        }
        
        /*
         *      finally, add it to the vector
         */
        m_Names.push_back( s );
    }
    
    /*
     *  quick sanity check
     */
    if( m_Names.empty() )
    {
        g_LogFile.ss()
                << "Error: zero names found in file '"
                << file
                << "'"
                ;
        g_LogFile.ssend();
        return;
    }
}

std::string cNameList::random()
{
    u_int size = m_Names.size();
    
    /*
     *  subscripting an empty vector causes a crash
     *  let's make sure that doesn't happen
     */
    if( size == 0 )
    {
        g_LogFile.ss()
                << "Error: no names in cNameList: "
                << "Returning name 'error'"
                ;
        g_LogFile.ssend();
        /*
         *      make sure we see the bug reports ...
         */
        return "error";
    }
    
    /*
     *  otherwise, pick a random name
     */
    return m_Names[ g_Dice.random( size - 1 ) ]; // edited from size to size-1 since I got an OOB vector crash once
}

cDoubleNameList::cDoubleNameList()
{
    ;
}
cDoubleNameList::cDoubleNameList( std::string first, std::string last )
    : m_first( first ), m_last( last )
{
    ;
}
void cDoubleNameList::load( std::string first, std::string last )
{
    m_last.load( last );
    m_first.load( first );
}
std::string cDoubleNameList::random()
{
    std::string s = "";
    s += m_first.random();
    s += " ";
    s += m_last.random();
    return s;
}

} // namespace WhoreMasterRenewal
