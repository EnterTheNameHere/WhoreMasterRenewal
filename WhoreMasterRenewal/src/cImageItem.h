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
#ifndef CIMAGEITEM_H_INCLUDED_1527
#define CIMAGEITEM_H_INCLUDED_1527
#pragma once

#include "cInterfaceObject.h" // required inheritance

#include <string>
#include <memory>

struct SDL_Surface;

namespace WhoreMasterRenewal
{

class CSurface;
class cAnimatedSurface;

class cImageItem : public cInterfaceObject
{
	bool m_Hidden;
public:
	cImageItem();
	virtual ~cImageItem();
	
	cImageItem( const cImageItem& ) = delete;
	cImageItem& operator = ( const cImageItem& ) = delete;

	bool CreateImage(int id, std::string filename, int x, int y, int width, int height, bool statImage = false, int R = 0, int G = 0, int B = 0);
	bool CreateAnimatedImage(int id, std::string filename, std::string dataFilename, int x, int y, int width, int height);

	void Draw();

	cAnimatedSurface* m_AnimatedImage;
	SDL_Surface* m_Surface;
	int m_ID;
	bool m_loaded;
	cImageItem* m_Next;	// next button on the window
    std::shared_ptr<CSurface> m_Image;
	void hide()	{ m_Hidden = true; }
	void unhide()	{ m_Hidden = false; }
	void toggle()	{ m_Hidden = !m_Hidden; }
};

} // namespace WhoreMasterRenewal

#endif // CIMAGEITEM_H_INCLUDED_1527
