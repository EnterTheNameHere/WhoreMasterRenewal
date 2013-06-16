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
#ifndef CINTERFACEEVENT_H_INCLUDED_1526
#define CINTERFACEEVENT_H_INCLUDED_1526
#pragma once

namespace WhoreMasterRenewal
{

class cInterfaceEventManager;
extern cInterfaceEventManager g_InterfaceEvents;

const int EVENT_BUTTONCLICKED = 1;
const int EVENT_SELECTIONCHANGE = 2;
const int EVENT_CHECKBOXCLICKED = 3;
const int EVENT_SLIDERCHANGE = 4;
//need to think about this

class cInterfaceEvent
{
public:
	cInterfaceEvent();
	~cInterfaceEvent();
    
    cInterfaceEvent( const cInterfaceEvent& ) = delete;
	cInterfaceEvent& operator = ( const cInterfaceEvent& ) = delete;
    
	int m_EventID;
	int m_ObjectID;

	cInterfaceEvent* m_Next;
};

class cInterfaceEventManager
{
public:
	cInterfaceEventManager();
	~cInterfaceEventManager();
	
	cInterfaceEventManager( const cInterfaceEventManager& ) = delete;
	cInterfaceEventManager& operator = ( const cInterfaceEventManager& ) = delete;
	
/*
 *	a bit of shorthand - helps keep the code cleaner
 */
	bool CheckButton( int ObjectID );
	bool CheckListbox( int ObjectID );
	bool CheckCheckbox( int ObjectID );
	bool CheckSlider( int ObjectID );
	bool CheckEvent( int EventID, int ObjectID );
	
	int GetNumEvents();
	void AddEvent( int ID, int Object );
	void ClearEvents();

private:
	int m_NumEvents;
	cInterfaceEvent* m_Events;
};

} // namespace WhoreMasterRenewal

#endif // CINTERFACEEVENT_H_INCLUDED_1526
