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
#ifndef CEDITBOX_H_INCLUDED_1531
#define CEDITBOX_H_INCLUDED_1531
#pragma once

#include "cInterfaceObject.h" // required

#include <string>

class SDL_Surface;

namespace WhoreMasterRenewal
{

class cFont;

class cEditBox : public cInterfaceObject
{
public:
	cEditBox();
	virtual ~cEditBox();
    
    cEditBox( const cEditBox& ) = delete;
	cEditBox& operator = ( const cEditBox& ) = delete;
    
	bool CreateEditBox(int ID, int x, int y, int width, int height, int BorderSize);
	bool IsOver(int x, int y);
	bool OnClicked(int x, int y);
	void ClearText();
    std::string GetText();

	void UpdateText(char key, bool upper);

	void Draw();

	int m_ID = 0;
	cFont* m_Text = nullptr;
	bool m_HasFocus = false;

	SDL_Surface* m_FocusedBackground = nullptr;
	SDL_Surface* m_Background = nullptr;
	SDL_Surface* m_Border = nullptr;
	int m_BorderSize = 0;
	
	cEditBox* m_Next = nullptr;	// next button on the window
};

} // namespace WhoreMasterRenewal

#endif // CEDITBOX_H_INCLUDED_1531
