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
#ifndef CCHECKBOX_H_INCLUDED_1527
#define CCHECKBOX_H_INCLUDED_1527
#pragma once

#include "cInterfaceObject.h" // required inheritance
#include "cFont.h" // required cFont

#include <string>
#include <memory>

class SDL_Surface;

namespace WhoreMasterRenewal
{

class CSurface;

class cCheckBox : public cInterfaceObject
{
public:
	cCheckBox();
	virtual ~cCheckBox();
    
    cCheckBox( const cCheckBox& ) = delete;
	cCheckBox& operator = ( const cCheckBox& ) = delete;
    
	bool CreateCheckBox(int id, int x, int y, int width, int height, std::string text, int fontsize = 12);
	bool GetState() {return m_StateOn;}
	void ButtonClicked(int x, int y);
	void SetState(bool on) {m_StateOn = on;}

	void Draw();

	std::shared_ptr<CSurface> m_Image = nullptr;
	SDL_Surface* m_Surface = nullptr;
	SDL_Surface* m_Border = nullptr;
	int m_ID = 0;
	bool m_StateOn = false;
	cCheckBox* m_Next = nullptr;	// next button on the window
	cFont m_Font = {};
	bool m_Disabled = false;
};

} // namespace WhoreMasterRenewal

#endif // CCHECKBOX_H_INCLUDED_1527
