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

#include <SDL_ttf.h>
#include <string>

class cFont
{
public:
	cFont();
	~cFont();

	void Free();

	void SetColor(unsigned char r, unsigned char g, unsigned char b);

	// need to undefine the stupid windows headers macro DrawText
    #ifdef DrawText
    #undef DrawText
    #endif
	bool DrawText(int x, int y, SDL_Surface* destination = 0, bool multi = false);	// draws the text surface to the screen
	bool DrawMultilineText(int x, int y, int linesToSkip = 0, int offsetY = 0, SDL_Surface* destination = 0);	// draws the text surface to the screen
	bool LoadFont(std::string font, int size);
	void SetText(std::string text);
	std::string GetText() {return m_Text;}
	void GetSize(std::string text, int &width, int &height){TTF_SizeText(m_Font, text.c_str(), &width, &height);}
	int GetWidth();
	int GetHeight();
	void SetMultiline(bool multi, int width, int height){m_IsMultiline=multi;m_Width = width;m_Height=height;}

	int IsFontFixedWidth(){return TTF_FontFaceIsFixedWidth(m_Font);}
	int GetFontHeight(){return TTF_FontHeight(m_Font);}	// returns the height in pixels of the font
	int GetFontLineSkip(){return TTF_FontLineSkip(m_Font);}	// returns the number of pixels you should have between lines
	void SetFontBold(bool Bold = true){TTF_SetFontStyle(m_Font, (Bold ? TTF_STYLE_BOLD : TTF_STYLE_NORMAL) );}

	int GetTotalNumberOfLines(){return m_NumLines;}
	int GetLinesPerBox(){if(m_Lineskip>0)return (m_Height/m_Lineskip);else return m_Height/GetFontLineSkip();}

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

#endif // CFONT_H_INCLUDED_1531
