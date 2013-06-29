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
#ifndef ICONSURFACE_H_INCLUDED_1505
#define ICONSURFACE_H_INCLUDED_1505
#pragma once

#include "CLog.h"
#include "DirPath.h"
#include "CSurface.h"

#include <string>

namespace WhoreMasterRenewal
{

class IconSurface : public CSurface
{
public:
	IconSurface( std::string name );
	IconSurface( std::string name, const char* pt, const char* ext = ".jpg" );
};

class ButtonSurface : public CSurface
{
public:
	ButtonSurface( std::string name);
	ButtonSurface( std::string name, const char *pt, const char *ext = ".jpg" );
};

class ImageSurface : public CSurface
{
public:
	ImageSurface( std::string name );
	ImageSurface( std::string name, const char *pt, const char *ext = ".jpg" );
};

} // namespace WhoreMasterRenewal

#endif // ICONSURFACE_H_INCLUDED_1505
