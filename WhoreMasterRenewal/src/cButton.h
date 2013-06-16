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
#ifndef CBUTTON_H_INCLUDED_1533
#define CBUTTON_H_INCLUDED_1533
#pragma once

#include "cInterfaceObject.h" // required inheritance

#include <string>
#include <memory>

namespace WhoreMasterRenewal
{

class CSurface;

class cButton : public cInterfaceObject
{
public:
	cButton();
	virtual ~cButton();
	
	cButton( const cButton& ) = delete;
	cButton& operator = ( const cButton& ) = delete;

	bool CreateButton(std::string OffImage, std::string DisabledImage, std::string OnImage, int ID, int x, int y, int width, int height, bool transparency = false,bool cached=false);
	bool IsOver(int x, int y);
	bool ButtonClicked(int x, int y);
	void SetDisabled(bool disable);

	virtual void Draw();

	void hide()	{ m_Hidden = true; }
	void unhide()	{ m_Hidden = false; }
	void toggle()	{ m_Hidden = !m_Hidden; }
	
private:
    int m_ID;
    
    std::shared_ptr<CSurface> m_OffImage;
	std::shared_ptr<CSurface> m_DisabledImage;
	std::shared_ptr<CSurface> m_OnImage;
	std::shared_ptr<CSurface> m_CurrImage;

	cButton* m_Next;	// next button on the window
	
	bool m_Disabled;
    bool m_Hidden;
};

} // namespace WhoreMasterRenewal

#endif // CBUTTON_H_INCLUDED_1533
