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
#ifndef CSURFACE_H_INCLUDED_1511
#define CSURFACE_H_INCLUDED_1511
#pragma once

#include "CResource.h" // required inheritance

#include <string>

class SDL_Surface;
class SDL_Rect;
typedef uint32_t Uint32; // Used by SDL

class CSurface : public CResource
{
public:
	CSurface();
	CSurface(std::string filename);
	CSurface(SDL_Surface* inputsurface);
	virtual ~CSurface();

	void Free();
	void FreeResources();
	void Register(bool loaded);

	void MakeColoredSurface(int width, int height, int red, int green, int blue, int BPP = 32);
	bool LoadImage(std::string filename, bool load = true);	// the value load tells the class to actually load the image or just save the filename for the resource manager and only load on use
	bool LoadSurface(SDL_Surface* inputsurface);  //for when we have an SDL surface in memory to use instead of loading from disk
	void SetColorKey(unsigned char r, unsigned char g, unsigned char b);
	void SetAlpha(bool UseAlpha);
	bool DrawSurface(int x, int y, SDL_Surface* destination = nullptr, SDL_Rect* clip = nullptr, bool resize = false, bool maintainRatio = true);
	bool DrawSprite(int x, int y);
	bool ResizeSprite(SDL_Surface* image, SDL_Rect* clip, bool maintainRatio = false);
	std::string GetFilename() {return m_Filename;}

	SDL_Surface** GetSurface() {return &m_Surface;}
	bool m_Cached;
	bool loaded;
	bool m_SaveSurface;

private:
	SDL_Surface* m_Temp;	// a temporary surface for resizing
	SDL_Surface* m_Surface;
	SDL_Surface* m_SpriteImage;	// used for resizing sprites
	std::string m_Filename;	// this is the filename and location, used to reload an unloaded resource
	bool m_UseKey;
	Uint32 m_ColorKey;
	bool m_UseAlpha;
	bool m_ColoredSurface;
};

#endif // CSURFACE_H_INCLUDED_1511
