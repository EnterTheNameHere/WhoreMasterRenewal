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
    
    cChoice( const cChoice& ) = delete;
	cChoice& operator = ( const cChoice& ) = delete;
    
	int m_NumChoices = 0;	// The number of choices available
	std::string* m_Choices = nullptr;	// array of choices available
	int m_CurrChoice = -1;	// The choice selected at present
	int m_ID = 0;	// the id for this particular box
	cChoice* m_Next = nullptr;	// the next choice box in the list
	SDL_Surface* m_Background = nullptr;
	SDL_Surface* m_Border = nullptr;
	SDL_Surface* m_ElementBackground = nullptr;	// the background and border for the list elements
	SDL_Surface* m_ElementSelectedBackground = nullptr;	// the background and border for the list elements
	SDL_Surface* m_HeaderBackground = nullptr;
	int m_XPos = 0;
	int m_YPos = 0;
	int m_Width = 0;
	int m_Height = 0;

	int m_NumDrawnElements = 0;
	int m_eWidth = 0;
	int m_eHeight = 0;
	int m_Position = 0;

	bool m_ScrollDisabled = false;
};


class cChoiceManager
{
public:
	cChoiceManager();
	~cChoiceManager();
    
    cChoiceManager( const cChoiceManager& ) = delete;
	cChoiceManager& operator = ( const cChoiceManager& ) = delete;
    
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
    menu_callback_type m_callback = nullptr;
    
	cChoice* m_Parent = nullptr;
	cChoice* m_ActiveChoice = nullptr;

	cFont* m_Font = nullptr;

	std::shared_ptr<CSurface> m_UpOn = nullptr;
	std::shared_ptr<CSurface> m_DownOn = nullptr;
	std::shared_ptr<CSurface> m_UpOff = nullptr;
	std::shared_ptr<CSurface> m_DownOff = nullptr;
	std::shared_ptr<CSurface> m_CurrUp = nullptr;
	std::shared_ptr<CSurface> m_CurrDown = nullptr;
};

} // namespace WhoreMasterRenewal

#endif // CCHOICEMESSAGE_H_INCLUDED_1527
