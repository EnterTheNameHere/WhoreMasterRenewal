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
#include <iostream>
#include "CLog.h"

namespace WhoreMasterRenewal
{
    Logger::Logger( const std::string& filename, bool append )
    {
        //if( m_FirstRun )
        //{
        //    sf::Lock lock( m_Mutex );
        //    {
        //        if( m_FirstRun )
        //        {
        //            m_FirstRun = false;
        //            
                    if( append )
                        m_LogFileStream.open( filename, std::ios_base::out | std::ios_base::app );
                    else
                        m_LogFileStream.open( filename, std::ios_base::out );
                    
        //            if( !m_LogFileStream.is_open() )
        //            {
        //                m_FailedToOpenLogFile = true;
        //                
        //                std::cerr << "Error: Failed to open \"" << filename << "\" file for logging.\n"
        //                            << "Logging to file disabled. \n";
        //            }
        //        }
        //    }
        //}
    }
    Logger::~Logger()
    {
        if( m_LogFileStream.is_open() )
        {
            m_LogFileStream.close();
        }
    }
    
    //bool Logger::m_FailedToOpenLogFile = false;
    //bool Logger::m_FirstRun = true;
}


bool CLogInner::setup = false;

CLogInner::CLogInner()
{
	if(!setup) {
		init();
	}
}

void CLogInner::init()
{
	std::cout << "CLogInner::init" << std::endl;
	if(setup) return;
	setup = true;
	m_ofile.open("gamelog.txt");
}

CLogInner::~CLogInner()
{
	m_ofile.close();
}

void CLogInner::write(std::string text)
{
	m_ofile<<text<<std::endl;
	m_ofile.flush();
#ifdef LINUX
	std::cout << text << std::endl;
#endif
}

CLogInner *CLog::inner = 0;
