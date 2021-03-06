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

#include <string>

struct SDL_Color;

namespace WhoreMasterRenewal
{

typedef uint8_t	Uint8; // Used by SDL

class cColor
{
public:
	static Uint8 convertFromHex( std::string hex );
	static void HexToSDLColor( std::string HexColor, SDL_Color* SDLColor );
};

} // namespace WhoreMasterRenewal

#endif // CCOLOR_H_INCLUDED_1533
