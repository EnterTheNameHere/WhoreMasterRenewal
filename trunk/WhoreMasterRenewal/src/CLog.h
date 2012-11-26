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
#ifndef CLOG_H_INCLUDED_1524
#define CLOG_H_INCLUDED_1524
#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "SFML/System/Lock.hpp"
#include "SFML/System/Mutex.hpp"

namespace WhoreMasterRenewal
{
    #define LOGGER_LOG_MESSAGE( text ) Logger() << text;
    
    class Logger
    {
    public:
        Logger( const std::string& filename = "GameLog.txt", bool append = true );
        ~Logger();
        
        template<typename T>
        inline Logger& operator << ( T& value )
        {
            //sf::Lock lock( m_Mutex );
            {
                std::cout << value;
 //               if( !m_FailedToOpenLogFile )
                if( m_LogFileStream.is_open() )
                    m_LogFileStream << value;
            }
            
            return *this;
        }
        
        inline Logger& operator << ( std::string value )
        {
            std::cout << value;
            if( m_LogFileStream.is_open() )
                m_LogFileStream << value;
            
            return *this;
        }
        
    private:
        std::ofstream m_LogFileStream;
        //sf::Mutex m_Mutex;
        
        //static bool m_FirstRun;
        //static bool m_FailedToOpenLogFile;
    };
}



struct CLogInner
{
public:
	CLogInner();
	~CLogInner();
	void init();

	void write(std::string text);
	std::ofstream	&os()	{ return m_ofile; }
	std::stringstream	&ss()	{ return m_ss; }
	void ssend() {
		write(m_ss.str());
		m_ss.str("");
	}
static	bool setup;
	std::ofstream m_ofile;
	std::stringstream m_ss;
};


class CLog
{
public:
	CLog(bool a_glob = false) {
		m_glob = a_glob;
	}
	~CLog() {
		if(!m_glob) {
			return;
		}
		if(inner) {
			delete inner;
		}
		inner = 0;
	}
	void write(std::string text)
	{
		if(!inner) inner = new CLogInner();
		inner->write(text);
	}
	std::ofstream	&os()	{
		if(!inner) inner = new CLogInner();
		return inner->m_ofile;
	}
	std::stringstream	&ss()	{
		if(!inner) inner = new CLogInner();
		return inner->m_ss;
	}
	void		ssend()	{
		if(!inner) inner = new CLogInner();
		inner->ssend();
	}
private:
	bool m_glob;
static	CLogInner *inner;
};

#endif // CLOG_H_INCLUDED_1524
