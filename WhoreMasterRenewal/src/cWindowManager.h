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
#ifndef CWINDOWMANAGER_H_INCLUDED_1508
#define CWINDOWMANAGER_H_INCLUDED_1508
#pragma once

#include <map>
#include <string>
#include <memory>

namespace WhoreMasterRenewal
{

class cWindowManager;
extern cWindowManager g_WinManager;
extern bool g_InitWin;
class CSurface;
extern std::shared_ptr<CSurface> g_BackgroundImage;
extern std::shared_ptr<CSurface> g_BrothelImages[6];

class cInterfaceWindow;
class cInterfaceWindowXML;

typedef struct sWindow
{
    void ( *Function )() = nullptr; // pointer to the update function
    void ( *XmlFunction )( cInterfaceWindow* ) = nullptr;
    
    sWindow();
    ~sWindow();
    
    sWindow( const sWindow& ) = delete;
    sWindow& operator = ( const sWindow& ) = delete;
    
    
    bool xmlfunc = false;
    cInterfaceWindow* m_Interface = nullptr;    // pointer to the interface
    sWindow* m_Next = nullptr;
} sWindow;

typedef void ( *process_func )();
typedef void ( *process_funcxml )( cInterfaceWindow * );

class cWindowManager
{
public:
    cWindowManager();
    ~cWindowManager();
    
    cWindowManager( const cWindowManager& ) = delete;
    cWindowManager& operator = ( const cWindowManager& ) = delete;
    
    void add_window( std::string name, cInterfaceWindowXML *win );
    
    /*
     *  add the screen to the stac
     *
     *  Studio seems to handle some Push methods in special ways
     *  (Alternatively, it's broken, depending on your POV)
     */
    void push( std::string window_name );
    void Push( process_func Process, cInterfaceWindow* Interface );
    void push( process_funcxml Process, cInterfaceWindow * Interface );
    
    // remove function from the stack
    void Pop();
    
    void PopToWindow( cInterfaceWindow* Interface );
    
    void UpdateCurrent();
    
    void UpdateMouseMovement( int x, int y );
    
    void UpdateMouseDown( int x, int y );
    
    void UpdateMouseClick( int x, int y, bool mouseWheelDown = false, bool mouseWheelUp = false );
    
    void UpdateKeyInput( char key, bool upper = false );
    
    bool HasEditBox();
    
    cInterfaceWindow* GetWindow();
    
    void Draw();
    
private:
    sWindow* m_Parent = nullptr;
    std::map<std::string, cInterfaceWindowXML *> windows = {};
};

} // namespace WhoreMasterRenewal

#endif // CWINDOWMANAGER_H_INCLUDED_1508
