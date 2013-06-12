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

#include "cGetStringScreenManager.h"
#include "InterfaceGlobals.h"
#include "cInterfaceEvent.h"
#include "cInterfaceWindow.h"
#include "cWindowManager.h"
#include "cMessageBox.h"
#include "InterfaceIDs.h"
#include "InterfaceProcesses.h"

namespace WhoreMasterRenewal
{

cGetStringScreenManager_Inner *cGetStringScreenManager::instance = nullptr;

cGetStringTransport::cGetStringTransport( std::string& dest )
    : m_dest( dest )
{
    ;
}
	
void cGetStringTransport::assign( std::string s )
{
    m_dest = s;
}

cGetStringTransport& cGetStringTransport::operator=( const cGetStringTransport& other )
{
    if( this != &other )
    {
        m_dest = other.m_dest;
    }
    return *this;
}

cGetStringTransport_Func::cGetStringTransport_Func(Funker munky)
    : funky( munky )
{
    ;
}

void cGetStringTransport_Func::assign( std::string /*s*/ )
{
    funky();
}

cGetStringScreenManager_Inner::cGetStringScreenManager_Inner()
{
    m_Trans = nullptr;
    m_empty_ok = true;
}

cGetStringScreenManager_Inner::~cGetStringScreenManager_Inner()
{
    if( m_Trans )
        delete m_Trans;
    m_Trans = nullptr;
}
	
void cGetStringScreenManager_Inner::process()
{
	if( g_InitWin )
	{
		g_GetString.Focused();
		g_InitWin = false;
	}

	// user hit Enter key?
	if( g_EnterKey )
		submit();

	if( g_InterfaceEvents.GetNumEvents() == 0 )
    {
		return;
	}

	if( g_InterfaceEvents.CheckButton( g_interfaceid.BUTTON_CANCEL ) )
    {
		g_ReturnText = "";
		g_InitWin = true;
		g_WinManager.Pop();
		return;
	}

	if( g_InterfaceEvents.CheckButton( g_interfaceid.BUTTON_OK ) )
    {
		submit();
	}
}

void cGetStringScreenManager_Inner::submit()
{
	g_EnterKey = false;

	g_ReturnText = g_GetString.GetEditBoxText( g_interfaceid.EDITBOX_NAME );

	if( !m_empty_ok && g_ReturnText == "" )
    {
		g_MessageQue.AddToQue( "You must enter a name", 0 );
		return;
	}
	m_empty_ok = true;

	g_InitWin = true;
	g_WinManager.Pop();

	if( !m_Trans )
    {
		return;
	}

	m_Trans->assign( g_ReturnText );
	delete m_Trans;
	m_Trans = nullptr;
}

void cGetStringScreenManager_Inner::set_dest( std::string& dest )
{
	if( m_Trans )
		delete m_Trans;
    m_Trans = new cGetStringTransport( dest );
}

void cGetStringScreenManager_Inner::set_handler( Funker funk )
{
	if( m_Trans )
		delete m_Trans;
    m_Trans = new cGetStringTransport_Func( funk );
}

void cGetStringScreenManager_Inner::empty_allowed( bool bval )
{
    m_empty_ok = bval;
}

bool cGetStringScreenManager_Inner::empty_allowed()
{
    return m_empty_ok;
}



cGetStringScreenManager::cGetStringScreenManager()
{
    if( instance )
    {
        return;
    }
    g_EnterKey = false;
    instance = new cGetStringScreenManager_Inner();
}

cGetStringScreenManager::~cGetStringScreenManager()
{
    ; /// @note We can't release instance, because we will lose information it carries... (m_Trans) - REFACTOR
}

void cGetStringScreenManager::ReleaseResources()
{
    /// @note This is HACK to release instance (Inner object). We can't do it in destructor, since
    ///       object is not global and gets constructed in code (thus destructed out of scope), but
    ///       instance carries information we will need to check later, so we have to keep it alive.
    ///       To prevent memory leak, this function needs to be called when shutting down the game.
    
    if( instance )
        delete instance;
    instance = nullptr;
}

void cGetStringScreenManager::process()
{
    instance->process();
}

void cGetStringScreenManager::set_dest( std::string& dest )
{
    instance->set_dest( dest );
}

void cGetStringScreenManager::set_handler( Funker funk )
{
    instance->set_handler( funk );
}

void cGetStringScreenManager::empty_allowed( bool bval )
{
    instance->empty_allowed( bval );
}

bool cGetStringScreenManager::empty_allowed()
{
    return instance->empty_allowed();;
}

} // namespace WhoreMasterRenewal
