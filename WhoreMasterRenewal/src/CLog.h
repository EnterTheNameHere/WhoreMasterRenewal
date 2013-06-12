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

#include <fstream>
#include <sstream>
#include <string>

namespace WhoreMasterRenewal
{

class CLog;
extern CLog g_LogFile;

struct CLogInner
{
public:
	CLogInner();
	~CLogInner();
	void init();

	void write( std::string text );
	std::ofstream& os();
	std::stringstream& ss();
	void ssend();
    
private:
    static bool setup;
	std::ofstream m_ofile;
	std::stringstream m_ss;
};

class CLog
{
public:
	CLog( bool a_glob = false );
	~CLog();
	
	void write( std::string text );
	std::stringstream& ss();
	void ssend();
	
private:
	bool m_glob;
    static CLogInner* inner;
};

} // namespace WhoreMasterRenewal

#endif // CLOG_H_INCLUDED_1524
