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
#ifndef CCOLOR_H_INCLUDED_1533
#define CCOLOR_H_INCLUDED_1533
#pragma once

#include <iostream>
#include <string>
#include <SDL.h>
#include "CLog.h"
using namespace std;

extern CLog g_LogFile;
struct SDL_Color;

class cColor
{
public:
	Uint8 convertFromHex(string hex);

	void HexToSDLColor(string HexColor, SDL_Color* SDLColor);
};

#endif // CCOLOR_H_INCLUDED_1533