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
#ifndef CINTERFACEOBJECT_H_INCLUDED_1526
#define CINTERFACEOBJECT_H_INCLUDED_1526
#pragma once

namespace WhoreMasterRenewal
{

class cInterfaceObject
{
public:
    cInterfaceObject();
    virtual ~cInterfaceObject();
    
	void SetPosition( int x, int y, int width, int height );
	virtual void Draw();
	int GetXPos();
	int GetYPos();
	int GetWidth();
	int GetHeight();

protected:
	int m_XPos = 0;
	int m_YPos = 0;
	int m_Width = 0;
	int m_Height = 0;
};

} // namespace WhoreMasterRenewal

#endif // CINTERFACEOBJECT_H_INCLUDED_1526
