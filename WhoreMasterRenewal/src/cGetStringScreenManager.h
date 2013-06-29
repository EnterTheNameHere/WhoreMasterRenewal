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
#ifndef CGETSTRINGSCREENMANAGER_H_INCLUDED_1529
#define CGETSTRINGSCREENMANAGER_H_INCLUDED_1529
#pragma once

#include <string>

namespace WhoreMasterRenewal
{

extern bool g_EnterKey;

typedef void (*Funker)();

class cGetStringTransport_Base
{
public:
    virtual ~cGetStringTransport_Base() {}
    
	virtual	void assign( std::string s ) = 0;
};

class cGetStringTransport : public cGetStringTransport_Base
{
public:
	cGetStringTransport( std::string& dest );
	
	void assign( std::string s );
	
	cGetStringTransport& operator =( const cGetStringTransport& other );
	
private:
    std::string& m_dest;
};

class cGetStringTransport_Func : public cGetStringTransport_Base
{
public:
	cGetStringTransport_Func( Funker munky );
    virtual	void assign( std::string /*s*/ );
	
private:
    Funker funky;
};

class cGetStringScreenManager_Inner
{
public:
	cGetStringScreenManager_Inner();
	~cGetStringScreenManager_Inner();
	
	cGetStringScreenManager_Inner( const cGetStringScreenManager_Inner& ) = delete;
	cGetStringScreenManager_Inner& operator = ( const cGetStringScreenManager_Inner& ) = delete;
	
	void process();
	void submit();
	void set_dest( std::string& dest );
	void set_handler( Funker funk );
	void empty_allowed( bool bval );
	bool empty_allowed();
	
private:
    cGetStringTransport_Base* m_Trans = nullptr;
	bool m_empty_ok = true;
};

class cGetStringScreenManager
{
public:
	cGetStringScreenManager();
	~cGetStringScreenManager();
	
	static void ReleaseResources();
	
	void process();
	void set_dest( std::string& dest );
	void set_handler( Funker funk );
	void empty_allowed( bool bval );
	bool empty_allowed();
	
private:
    static cGetStringScreenManager_Inner* instance;
};

} // namespace WhoreMasterRenewal

#endif // CGETSTRINGSCREENMANAGER_H_INCLUDED_1529
