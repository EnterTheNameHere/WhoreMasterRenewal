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

#include "cWindowManager.h"
#include "CLog.h"
#include "cInterfaceWindow.h"

namespace WhoreMasterRenewal
{

sWindow::sWindow()
{
    ;
}

sWindow::~sWindow()
{
    if( m_Next )
        delete m_Next;
    m_Next = nullptr;
    m_Interface = nullptr;
}


cWindowManager::cWindowManager()
{
    ;
}

cWindowManager::~cWindowManager()
{
    if( m_Parent )
        delete m_Parent;
    m_Parent = nullptr;
}

void cWindowManager::add_window( std::string name, cInterfaceWindowXML* win)
{
    windows[name] = win;
}

// remove function from the stack
void cWindowManager::Pop()
{
    if( m_Parent != nullptr )
    {
        sWindow *InterfacePtr = m_Parent;
        m_Parent = m_Parent->m_Next;
        InterfacePtr->m_Next = nullptr;
        delete InterfacePtr;
        InterfacePtr = nullptr;
    }
}

void cWindowManager::PopToWindow(cInterfaceWindow* Interface)
{
    if(m_Parent != nullptr)
    {
        while(m_Parent->m_Interface != Interface)
            Pop();
    }
}

void cWindowManager::UpdateCurrent()
{
    if(!m_Parent) {
        return;
    }
    if(m_Parent->xmlfunc)
        m_Parent->XmlFunction(m_Parent->m_Interface);
    else
        m_Parent->Function();
}

void cWindowManager::UpdateMouseMovement(int x, int y)
{
    if(m_Parent)
        m_Parent->m_Interface->UpdateWindow(x, y);
}

void cWindowManager::UpdateMouseDown(int x, int y)
{
    if(m_Parent)
        m_Parent->m_Interface->MouseDown(x, y);
}

void cWindowManager::UpdateMouseClick(int x, int y, bool mouseWheelDown , bool mouseWheelUp )
{
    if(m_Parent)
        m_Parent->m_Interface->Click(x, y, mouseWheelDown, mouseWheelUp);
}

void cWindowManager::UpdateKeyInput(char key, bool upper )
{
    if(m_Parent)
        m_Parent->m_Interface->UpdateEditBoxes(key, upper);
}

bool cWindowManager::HasEditBox()
{
    if(!m_Parent)
        return false;
    
    return m_Parent->m_Interface->HasEditBox();
}

cInterfaceWindow* cWindowManager::GetWindow()
{
    if(!m_Parent)
        return nullptr;
    
    return m_Parent->m_Interface;
}

void cWindowManager::Draw()
{
    if(m_Parent)
        m_Parent->m_Interface->Draw();
}



void cWindowManager::push(std::string window_name)
{
/*
 *	check the screen exists
 */
	if(windows.find(window_name) == windows.end()) {
		g_LogFile.ss()
			<< "cWindowManager::Push: can't find window "
			<< "named '" << window_name << "'"
		;
		g_LogFile.ssend();
		return;
	}
/*
 *	look up the manager object
 */
	cInterfaceWindowXML *wpt = windows[window_name];
/*
 *	push the static handler and the window pointer onto the stack
 */
	push(process_funcxml(wpt->handler_func), (cInterfaceWindow*)wpt);
}

void cInterfaceWindowXML::handler_func(cInterfaceWindowXML *wpt)
{
	wpt->process();
}

void cWindowManager::Push(process_func Process, cInterfaceWindow* Interface)
{
	// Don't push a NULL value
	if(Process == nullptr)  {
		return;
	}
	// Allocate a new process and push it on stack
	sWindow *InterfacePtr = new sWindow();
	InterfacePtr->m_Next = m_Parent;
	m_Parent = InterfacePtr;
	//if(xmlf)
	//	InterfacePtr->XmlFunction=void (_cdelc *)(cInterfaceWindow *)Process;
	//else
	InterfacePtr->Function = Process;
	InterfacePtr->m_Interface = Interface;
	InterfacePtr->xmlfunc=false;
}

void cWindowManager::push(process_funcxml Process, cInterfaceWindow* Interface)
{
	// Don't push a NULL value
	if(Process == nullptr)  {
		return;
	}
	// Allocate a new process and push it on stack
	sWindow *InterfacePtr = new sWindow();
	InterfacePtr->m_Next = m_Parent;
	m_Parent = InterfacePtr;
	//if(xmlf)
	//	InterfacePtr->XmlFunction=void (_cdelc *)(cInterfaceWindow *)Process;
	//else
	InterfacePtr->XmlFunction = Process;
	InterfacePtr->m_Interface = Interface;
	InterfacePtr->xmlfunc=true;
}

} // namespace WhoreMasterRenewal
