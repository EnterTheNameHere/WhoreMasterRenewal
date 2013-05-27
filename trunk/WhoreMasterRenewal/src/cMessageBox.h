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
#ifndef CMESSAGEBOX_H_INCLUDED_1522
#define CMESSAGEBOX_H_INCLUDED_1522
#pragma once

#include <queue>
#include <string>

class cMessageQue;
extern cMessageQue g_MessageQue;

class cFont;
class SDL_Surface;

const int NUM_MESSBOXCOLOR = 4;

// separate to text boxes and edit boxes these boxes will display text and on a user click advance to the next box
// to continue displaying the message
class cMessageBox
{
public:
	cMessageBox();
	~cMessageBox();

	void CreateWindow(int x = 32, int y = 416, int width = 736, int height = 160, int BorderSize = 1, int FontSize = 16, bool scale = true);
	void ChangeFontSize(int FontSize = 16);
	void Draw();
	void Advance();
	void ResetWindow(std::string text, int color);
	bool IsActive();
	void SetActive(bool active);

private:
	int m_XPos, m_YPos, m_Height, m_Width, m_BorderSize;

	bool m_Active;
	bool m_Advance;

	SDL_Surface* m_Background[NUM_MESSBOXCOLOR];
	SDL_Surface* m_Border;

	cFont* m_Font;
    std::string m_Text;	// contains the entire text string
	int m_Position;	// where we are up too

	bool m_TextAdvance;
	int m_Color;	// used to determine which color to use
};

extern cMessageBox g_MessageBox;

typedef struct sMessage
{
    std::string m_Text;
	int m_Color;
	sMessage* m_Next;

	sMessage() {m_Next=0;m_Color=0;m_Text="";}
	~sMessage() {if(m_Next) delete m_Next; m_Next=0;}
}sMessage;

class cMessageQue
{
public:
	cMessageQue() {m_Mess=0;m_Last=0;}
	~cMessageQue() {Free();}

	void Free()	{if(m_Mess) delete m_Mess; m_Last=m_Mess=0;}

	void AddToQue(std::string text, int color)
	{
		if(text != "")
		{
			// Allocate a new process and push it on stack
			sMessage* Ptr = new sMessage();

			if(m_Last)
			{
				m_Last->m_Next = Ptr;
				m_Last = Ptr;
			}
			else
				m_Mess = m_Last = Ptr;
			Ptr->m_Text = text;
			Ptr->m_Color = color;
		}
	}

	bool HasNext()
	{
		if(m_Mess)
			return true;
		return false;
	}

	void ActivateNext()
	{
		if(m_Mess)
		{
			sMessage* Ptr = m_Mess;
			m_Mess = m_Mess->m_Next;
			Ptr->m_Next = 0;
			if(m_Mess == 0)
				m_Last = 0;

			g_MessageBox.ResetWindow(Ptr->m_Text, Ptr->m_Color);
			g_MessageBox.SetActive(true);

			delete Ptr;
			Ptr = 0;
		}
	}

private:
	sMessage* m_Mess;
	sMessage* m_Last;
};

#endif // CMESSAGEBOX_H_INCLUDED_1522
