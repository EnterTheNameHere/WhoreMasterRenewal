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

class IconSurface : public CSurface {
public:
	IconSurface(std::string name)
	: CSurface(ImagePath(name + ".png"))
	{
	}
	IconSurface(std::string name, const char *pt, const char *ext = ".jpg")
	: CSurface()
	//: CSurface(ImagePath((name + pt) + ext).c_str())
	{
		CLog log;

		std::string full = "";
		full += name;
		full += pt;
		full += ext;
		ImagePath dp(full);

		log.ss() << "IconSurface::IconSurface\n"
			 << "	name = " << name << "\n"
			 << "	ext  = " << ext  << "\n"
			 << "	full = " << full  << "\n"
			 << "	dp   = " << dp.c_str()  << "\n"
		;
		log.ssend();

		LoadImage(dp.c_str(), true);
	}
};

class ButtonSurface : public CSurface {
public:
	ButtonSurface (std::string name)
	: CSurface(ButtonPath(name + ".png"))
	{
	}
	ButtonSurface (std::string name, const char *pt, const char *ext = ".jpg")
	: CSurface(ButtonPath((name + pt) + ext).c_str())
	{
	}
};

class ImageSurface : public CSurface {
public:
	ImageSurface (std::string name)
	: CSurface(ImagePath(name + ".png"))
	{
	}
	ImageSurface (std::string name, const char *pt, const char *ext = ".jpg")
	: CSurface(ImagePath((name + pt) + ext).c_str())
	{
	}
};

#endif // ICONSURFACE_H_INCLUDED_1505
