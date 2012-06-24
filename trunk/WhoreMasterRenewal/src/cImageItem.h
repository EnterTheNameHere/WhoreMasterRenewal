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

#include "CSurface.h"
#include "cAnimatedSurface.h"
#include "cInterfaceObject.h"

class CSurface;

class cImageItem : public cInterfaceObject
{
	bool m_Hidden;
public:
	cImageItem() {
		m_Image 	= 0;
		m_Next		= 0;
		m_Surface	= 0;
		m_AnimatedImage = 0;
		m_loaded	= false;
		m_Hidden	= false;
	}
	~cImageItem();

	bool CreateImage(int id, string filename, int x, int y, int width, int height, bool statImage = false, int R = 0, int G = 0, int B = 0);
	bool CreateAnimatedImage(int id, string filename, string dataFilename, int x, int y, int width, int height);

	void Draw();

	cAnimatedSurface* m_AnimatedImage;
	CSurface* m_Image;
	SDL_Surface* m_Surface;
	int m_ID;
	bool m_loaded;
	cImageItem* m_Next;	// next button on the window

	void hide()	{ m_Hidden = true; }
	void unhide()	{ m_Hidden = false; }
	void toggle()	{ m_Hidden = !m_Hidden; }
};

#endif // CIMAGEITEM_H_INCLUDED_1527
