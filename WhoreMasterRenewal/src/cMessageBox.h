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

#include <string>

class SDL_Surface;

namespace WhoreMasterRenewal
{

class cMessageBox;
extern cMessageBox g_MessageBox;
class cMessageQue;
extern cMessageQue g_MessageQue;

class cFont;

const int NUM_MESSBOXCOLOR = 4;

// separate to text boxes and edit boxes these boxes will display text and on a user click advance to the next box
// to continue displaying the message
class cMessageBox
{
public:
    cMessageBox();
    ~cMessageBox();
    
    cMessageBox( const cMessageBox& ) = delete;
    cMessageBox& operator = ( const cMessageBox& ) = delete;
    
    void CreateWindow( int x = 32, int y = 416, int width = 736, int height = 160, int BorderSize = 1, int FontSize = 16, bool scale = true );
    void ChangeFontSize( int FontSize = 16 );
    void Draw();
    void Advance();
    void ResetWindow( std::string text, int color );
    bool IsActive();
    void SetActive( bool active );
    
private:
    int m_XPos = 0;
    int m_YPos = 0;
    int m_Height = 0;
    int m_Width = 0;
    int m_BorderSize = 0;
    
    bool m_Active = false;
    bool m_Advance = false;
    
    SDL_Surface* m_Background[NUM_MESSBOXCOLOR];
    SDL_Surface* m_Border = nullptr;
    
    cFont* m_Font = nullptr;
    std::string m_Text = "Default cMessageBox::m_Text value"; // contains the entire text string
    int m_Position = 0; // where we are up too
    
    bool m_TextAdvance = false;
    int m_Color = 0; // used to determine which color to use
};

typedef struct sMessage
{
    sMessage();
    ~sMessage();
    
    sMessage( const sMessage& ) = delete;
    sMessage& operator = ( const sMessage& ) = delete;
    
    std::string m_Text = "Default sMessage::m_Text value";
    int m_Color = 0;
    sMessage* m_Next = nullptr;
} sMessage;

class cMessageQue
{
public:
    cMessageQue();
    ~cMessageQue();
    
    cMessageQue( const cMessageQue& ) = delete;
    cMessageQue& operator = ( const cMessageQue& ) = delete;
    
    void Free();
    
    void AddToQue( std::string text, int color );
    
    bool HasNext();
    
    void ActivateNext();
    
private:
    sMessage* m_Mess = nullptr;
    sMessage* m_Last = nullptr;
};

} // namespace WhoreMasterRenewal

#endif // CMESSAGEBOX_H_INCLUDED_1522
