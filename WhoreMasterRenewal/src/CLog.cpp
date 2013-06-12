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

#include "CLog.h"

#include <iostream>

namespace WhoreMasterRenewal
{

std::ofstream& CLogInner::os()
{
    return m_ofile;
}

std::stringstream& CLogInner::ss()
{
    return m_ss;
}

void CLogInner::ssend()
{
    write( m_ss.str() );
    m_ss.str("");
}

CLogInner::CLogInner()
    : m_ofile(), m_ss()
{
	if( !setup )
    {
		init();
	}
}

void CLogInner::init()
{
	if( setup )
        return;
    
    std::cout << "CLogInner::init" << std::endl;
	setup = true;
	m_ofile.open("gamelog.txt");
}

CLogInner::~CLogInner()
{
    write( "Finished Logging..." );
    
	m_ofile.close();
}

void CLogInner::write( std::string text )
{
    if( m_ofile.is_open() )
        m_ofile << text << std::endl;
	std::cout << text << std::endl;
}

CLog::CLog( bool a_glob )
    : m_glob( a_glob )
{
    
}

CLog::~CLog()
{
    if( !m_glob )
    {
        return;
    }
    if( inner )
    {
        delete inner;
    }
    inner = nullptr;
}

void CLog::write( std::string text )
{
    if( !inner )
        inner = new CLogInner();
    inner->write( text );
}

std::stringstream& CLog::ss()
{
    if( !inner )
        inner = new CLogInner();
    return inner->ss();
}

void CLog::ssend()
{
    if( !inner )
        inner = new CLogInner();
    inner->ssend();
}

bool CLogInner::setup = false;
CLogInner* CLog::inner = nullptr;

} // namespace WhoreMasterRenewal
