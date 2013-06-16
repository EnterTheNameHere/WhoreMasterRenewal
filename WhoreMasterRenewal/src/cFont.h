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
#ifndef CFONT_H_INCLUDED_1531
#define CFONT_H_INCLUDED_1531
#pragma once

#include <SDL_ttf.h> // required SDL_Color

#include <string>

namespace WhoreMasterRenewal
{

class cFont
{
public:
	cFont();
	~cFont();
    
    cFont( const cFont& ) = delete;
	cFont& operator = ( const cFont& ) = delete;
    
	void Free();

	void SetColor(unsigned char r, unsigned char g, unsigned char b);

	bool DrawText(int x, int y, SDL_Surface* destination = nullptr, bool multi = false);	// draws the text surface to the screen
	bool DrawMultilineText(int x, int y, int linesToSkip = 0, int offsetY = 0, SDL_Surface* destination = nullptr);	// draws the text surface to the screen
	bool LoadFont(std::string font, int size);
	void SetText(std::string text);
	std::string GetText();
	void GetSize(std::string text, int& width, int &height);
	int GetWidth();
	int GetHeight();
	void SetMultiline(bool multi, int width, int height);

	int IsFontFixedWidth();
	int GetFontHeight();	// returns the height in pixels of the font
	int GetFontLineSkip();	// returns the number of pixels you should have between lines
	void SetFontBold(bool Bold = true);

	int GetTotalNumberOfLines();
	int GetLinesPerBox();

private:
	TTF_Font* m_Font;
	SDL_Color m_TextColor;
	SDL_Surface* m_Message;	// for storing a single line message
	SDL_Surface* m_MultilineMessage;	// for storing multiline messages
	std::string m_Text;
	bool m_NewText;	// variable for keeping track of if it needs to be updated
	bool m_IsMultiline;
	int m_Width;
	int m_Height;
	int m_Lineskip;
	unsigned int m_NumLines;	// stores the total number of lines in the box

	// These functions are used internally to draw text to a surface
	void RenderText(std::string text = "", bool multi = false);
	void RenderMultilineText(std::string text);	// function that renders multiline text to the internal surface
	std::string UpdateLineEndings(std::string text);  // added function to fix line endings ("/n"=>"/r/n") for Windows
};

} // namespace WhoreMasterRenewal

#endif // CFONT_H_INCLUDED_1531
