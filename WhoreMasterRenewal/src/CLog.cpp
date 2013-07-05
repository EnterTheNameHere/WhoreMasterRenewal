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
#include "Logger.hpp"

#include <iostream>

namespace WhoreMasterRenewal
{

CLog::CLog()
{
    
}

CLog::~CLog()
{
    
}

void CLog::write( std::string text )
{
    Logger() << text;
}

std::stringstream& CLog::ss()
{
    return m_ss;
}

void CLog::ssend()
{
    m_ss << std::endl;
    write( m_ss.str() );
    m_ss.str("");
}

} // namespace WhoreMasterRenewal
