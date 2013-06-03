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
#ifndef CEVENTS_H_INCLUDED_1531
#define CEVENTS_H_INCLUDED_1531
#pragma once

#include "Constants.h"

#include <string>
#include <vector>

class CEvent
{
public:
	unsigned char	m_Event;				// type of event
	unsigned char	m_MessageType;			// Image Type of message
	std::string			m_Message;

	//string		name;					//	name of who this event applies to, usually girl name
	//int			imageType;
	//int			imageNum;

	std::string			TitleText();			//	Default listbox Text
	unsigned int	ListboxColour();		//	Default Listbox Colour
	unsigned int	m_Ordinal;				//  Used for sort order
	bool			IsUrgent();
	bool			IsDanger();
	bool			IsWarning();
	static bool		CmpEventPredicate(CEvent eFirst, CEvent /*eSecond*/)
					{
						return eFirst.m_Ordinal < eFirst.m_Ordinal;
					}
};

class cEvents
{
public:
	cEvents();
	~cEvents();

	void			Free();
	void			Clear() {Free();}
//	void			DisplayMessages();		// No definition
	void			AddMessage(std::string message, int nImgType, int nEvent);

	CEvent			GetMessage(int id);
	int				GetNumEvents() 			{ return events.size(); }
	bool			IsEmpty()				{ return events.empty() ; }
	bool			HasUrgent();
	bool			HasDanger();
	bool			HasWarning();
	void			DoSort();


private:
	std::vector<CEvent>	events;
	bool			m_bSorted;				// flag to only allow sort once
	unsigned int	MakeOrdinal(int nEvent);
};

#endif // CEVENTS_H_INCLUDED_1531
