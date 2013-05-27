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
#ifndef CGRAPHICS_H_INCLUDED_1527
#define CGRAPHICS_H_INCLUDED_1527
#pragma once

#include "CTimer.h" // required CTimer

#include <string>

class SDL_Surface;

class CGraphics
{
public:
	CGraphics();
	~CGraphics();

	bool InitGraphics(std::string caption, int Width, int Height, int BPP);
	void Free();
	bool End();	// End Drawing Stuff
	void Begin(); // begins drawing stuff

	// Accessors
	SDL_Surface* GetScreen();
	unsigned int GetTicks();

	int GetWidth();
	int GetHeight();

private:
	// Screen
	SDL_Surface* m_Screen;

	// screen attributes
	int m_ScreenWidth;
	int m_ScreenHeight;
	int m_ScreenBPP;

	// Time
	unsigned int m_CurrentTime;
	CTimer m_FPS;
};

#endif // CGRAPHICS_H_INCLUDED_1527
