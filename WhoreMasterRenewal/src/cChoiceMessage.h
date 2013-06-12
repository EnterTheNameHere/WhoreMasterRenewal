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
#ifndef CCHOICEMESSAGE_H_INCLUDED_1527
#define CCHOICEMESSAGE_H_INCLUDED_1527
#pragma once

#include <string>
#include <memory>

class SDL_Surface;

namespace WhoreMasterRenewal
{

class cChoiceManager;
extern cChoiceManager g_ChoiceManager;

class cFont;
class cChoice;
class CSurface;

typedef void (*menu_callback_type)(int);

class cChoice	// represents a list of text selections and the currently selected text
{
public:
	cChoice();
	~cChoice();

	int m_NumChoices;	// The number of choices available
	std::string* m_Choices;	// array of choices available
	int m_CurrChoice;	// The choice selected at present
	int m_ID;	// the id for this particular box
	cChoice* m_Next;	// the next choice box in the list
	SDL_Surface* m_Background;
	SDL_Surface* m_Border;
	SDL_Surface* m_ElementBackground;	// the background and border for the list elements
	SDL_Surface* m_ElementSelectedBackground;	// the background and border for the list elements
	SDL_Surface* m_HeaderBackground;
	int m_XPos;
	int m_YPos;
	int m_Width;
	int m_Height;

	int m_NumDrawnElements;
	int m_eWidth;
	int m_eHeight;
	int m_Position;

	bool m_ScrollDisabled;
};


class cChoiceManager
{
public:
	cChoiceManager();
	~cChoiceManager();

	void Free();
	void CreateChoiceBox(int x, int y, int width, int height, int ID, int numChoices, int itemHeight, int MaxStrLen = 0);
	void CreateChoiceBoxResize(int ID, int numChoices);
	void BuildChoiceBox(int ID, int MaxStrLen);
	void AddChoice(int ID, std::string text, int choiceID);
	void Draw();
	int GetChoice(int ID);
	void SetActive(int ID);
	bool IsActive();

	bool IsOver(int x, int y);
	bool ButtonClicked(int x, int y);

	void set_callback(menu_callback_type func) {
		m_callback = func;
	}
	bool find_active(int x, int y);

private:
    menu_callback_type m_callback;
    
	cChoice* m_Parent;
	cChoice* m_ActiveChoice;

	cFont* m_Font;

	std::shared_ptr<CSurface> m_UpOn;
	std::shared_ptr<CSurface> m_DownOn;
	std::shared_ptr<CSurface> m_UpOff;
	std::shared_ptr<CSurface> m_DownOff;
	std::shared_ptr<CSurface> m_CurrUp;
	std::shared_ptr<CSurface> m_CurrDown;
};

} // namespace WhoreMasterRenewal

#endif // CCHOICEMESSAGE_H_INCLUDED_1527
