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
#ifndef CTEXTITEM_H_INCLUDED_1510
#define CTEXTITEM_H_INCLUDED_1510
#pragma once

#include "cInterfaceObject.h" // required inheritance
#include "cFont.h" // required cFont

#include <string>

namespace WhoreMasterRenewal
{

class cScrollBar;

class cTextItem : public cInterfaceObject
{
public:
	cTextItem();
	virtual ~cTextItem();
    
    cTextItem( const cTextItem& ) = delete;
	cTextItem& operator = ( const cTextItem& ) = delete;
    
	void CreateTextItem( int ID, int x, int y, int width, int height, std::string text, int size, bool auto_scrollbar = true, bool force_scrollbar = false );
	
	void DisableAutoScroll( bool disable );
	void ForceScrollBar( bool force );

	// does scrollbar exist, but current text fits, and scrollbar isn't being forced?
	bool NeedScrollBarHidden();
	// does scrollbar exist but is hidden, and current text doesn't fit?
	bool NeedScrollBarShown();
	// does a scrollbar need to be added?
	bool NeedScrollBar();
	int HeightTotal();
	void MouseScrollWheel( int x, int y, bool ScrollDown = true );

	bool IsOver( int x, int y );

	void ChangeFontSize( int FontSize );

	void SetText( std::string text );

	void Draw();

	void hide();
	void unhide();
    
    bool m_Hide = false;
	bool m_AutoScrollBar = true;  // automatically use scrollbar if text is too tall?
	bool m_ForceScrollBar = false;  // force scrollbar display even if text fits?
	
	//int m_CharsPerLine, m_LinesPerBox, m_CharHeight;
    std::string m_Text = "Default cTextItem::m_Text value";
	int m_ID = 0;
	cTextItem* m_Next = nullptr;
	cFont m_Font = {};

	int m_ScrollChange = 0;  // scrollbar changes will update this value; translates to skipped lines of text
	cScrollBar* m_ScrollBar = nullptr;  // pointer to the associated scrollbar, if any
};

} // namespace WhoreMasterRenewal

#endif // CTEXTITEM_H_INCLUDED_1510
