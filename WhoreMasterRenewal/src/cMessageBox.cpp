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

#include "cMessageBox.h"
#include "CGraphics.h"
#include "sConfig.h"
#include "cFont.h"
#include "InterfaceGlobals.h"
#include "InterfaceProcesses.h"

namespace WhoreMasterRenewal
{

cMessageBox::~cMessageBox()
{
    for( int i = 0; i < NUM_MESSBOXCOLOR; i++ )
    {
        if( m_Background[i] )
            SDL_FreeSurface( m_Background[i] );
            
        m_Background[i] = nullptr;
    }
    
    if( m_Border )
        SDL_FreeSurface( m_Border );
        
    m_Border = nullptr;
    
    if( m_Font )
    {
        m_Font->Free();
        delete m_Font;
    }
    
    m_Font = nullptr;
}

void cMessageBox::CreateWindow( int x, int y, int width, int height, int BorderSize, int FontSize, bool scale )
{
    float xScale = 1.0f, yScale = 1.0f;
    
    if( scale )
    {
        if( g_ScreenWidth != 800 )
            xScale = ( float )( ( float )g_ScreenWidth / ( float )800 );
            
        if( g_ScreenHeight != 600 )
            yScale = ( float )( ( float )g_ScreenHeight / ( float )600 );
    }
    
    x = ( int )( ( float )x * xScale );
    y = ( int )( ( float )y * yScale );
    width = ( int )( ( float )width * xScale );
    height = ( int )( ( float )height * yScale );
    
    m_BorderSize = BorderSize;
    m_XPos = x;
    m_YPos = y;
    m_Width = width;
    m_Height = height;
    m_Border = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0 );
    SDL_FillRect( m_Border, nullptr, SDL_MapRGB( m_Border->format, g_MessageBoxBorderR, g_MessageBoxBorderG, g_MessageBoxBorderB ) );
    
    m_Background[0] = SDL_CreateRGBSurface( SDL_SWSURFACE, width - ( BorderSize * 2 ), height - ( BorderSize * 2 ), 32, 0, 0, 0, 0 );
    SDL_FillRect( m_Background[0], nullptr, SDL_MapRGB( m_Background[0]->format, g_MessageBoxBackground0R, g_MessageBoxBackground0G, g_MessageBoxBackground0B ) );
    
    m_Background[1] = SDL_CreateRGBSurface( SDL_SWSURFACE, width - ( BorderSize * 2 ), height - ( BorderSize * 2 ), 32, 0, 0, 0, 0 );
    SDL_FillRect( m_Background[1], nullptr, SDL_MapRGB( m_Background[1]->format, g_MessageBoxBackground1R, g_MessageBoxBackground1G, g_MessageBoxBackground1B ) );
    
    m_Background[2] = SDL_CreateRGBSurface( SDL_SWSURFACE, width - ( BorderSize * 2 ), height - ( BorderSize * 2 ), 32, 0, 0, 0, 0 );
    SDL_FillRect( m_Background[2], nullptr, SDL_MapRGB( m_Background[2]->format, g_MessageBoxBackground2R, g_MessageBoxBackground2G, g_MessageBoxBackground2B ) );
    
    m_Background[3] = SDL_CreateRGBSurface( SDL_SWSURFACE, width - ( BorderSize * 2 ), height - ( BorderSize * 2 ), 32, 0, 0, 0, 0 );
    SDL_FillRect( m_Background[3], nullptr, SDL_MapRGB( m_Background[3]->format, g_MessageBoxBackground3R, g_MessageBoxBackground3G, g_MessageBoxBackground3B ) );
    
    ChangeFontSize( FontSize );
}

void cMessageBox::Draw()
{
    if( !m_Active )
        return;
        
    if( m_Background[m_Color] && m_Border )
    {
        // Draw the window
        SDL_Rect offset;
        offset.x = m_XPos;
        offset.y = m_YPos;
        
        // blit to the screen
        SDL_BlitSurface( m_Border, nullptr, g_Graphics.GetScreen(), &offset );
        
        offset.x = m_XPos + m_BorderSize;
        offset.y = m_YPos + m_BorderSize;
        SDL_BlitSurface( m_Background[m_Color], nullptr, g_Graphics.GetScreen(), &offset );
    }
    
    if( m_Font ) // draw the text
    {
        if( m_Text != m_Font->GetText() )
            m_Font->SetText( m_Text );
            
        m_Font->DrawMultilineText( m_XPos, m_YPos, m_Position );
    }
}

void cMessageBox::Advance()
{
    m_Position += m_Font->GetLinesPerBox();
    m_TextAdvance = false;
    
    if( m_Position >= m_Font->GetTotalNumberOfLines() )
        m_Active = false;
        
    m_Advance = true;
}

void cMessageBox::ChangeFontSize( int FontSize )
{
    if( m_Font )
    {
        m_Font->Free();
        delete m_Font;
    }
    
    m_Font = nullptr;
    m_Font = new cFont();
    cConfig cfg;
    m_Font->LoadFont( cfg.fonts.normal(), FontSize );
    m_Font->SetText( "" );
    m_Font->SetColor( g_MessageBoxTextR, g_MessageBoxTextG, g_MessageBoxTextB );
    m_Font->SetMultiline( true, m_Width, m_Height );
}

cMessageBox::cMessageBox()
{
    m_Color = 0;
    m_TextAdvance = false;
    m_Font = nullptr;
    m_Text = "";
    
    for( int i = 0; i < NUM_MESSBOXCOLOR; ++i )
        m_Background[i] = nullptr;
        
    m_Border = nullptr;
    m_Active = false;
    m_Advance = false;
    m_Position = 0;
}

void cMessageBox::ResetWindow( std::string text, int color )
{
    if( m_Font )
        m_Font->SetText( text );
        
    m_Text = text;
    m_Position = 0;
    m_TextAdvance = false;
    m_Color = color;
}

bool cMessageBox::IsActive()
{
    return m_Active;
}

void cMessageBox::SetActive( bool active )
{
    m_Active = active;
}

sMessage::sMessage()
{
    m_Next = nullptr;
    m_Color = 0;
    m_Text = "";
}

sMessage::~sMessage()
{
    if( m_Next ) delete m_Next;
    
    m_Next = nullptr;
}

cMessageQue::cMessageQue()
{
    m_Mess = nullptr;
    m_Last = nullptr;
}
cMessageQue::~cMessageQue()
{
    Free();
}

void cMessageQue::Free()
{
    if( m_Mess ) delete m_Mess;
    
    m_Last = nullptr;
    m_Mess = nullptr;
}

void cMessageQue::AddToQue( std::string text, int color )
{
    if( text != "" )
    {
        // Allocate a new process and push it on stack
        sMessage* Ptr = new sMessage();
        
        if( m_Last )
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

bool cMessageQue::HasNext()
{
    if( m_Mess )
        return true;
        
    return false;
}

void cMessageQue::ActivateNext()
{
    if( m_Mess )
    {
        sMessage* Ptr = m_Mess;
        m_Mess = m_Mess->m_Next;
        Ptr->m_Next = nullptr;
        
        if( m_Mess == nullptr )
            m_Last = nullptr;
            
        g_MessageBox.ResetWindow( Ptr->m_Text, Ptr->m_Color );
        g_MessageBox.SetActive( true );
        
        delete Ptr;
        Ptr = nullptr;
    }
}

} // namespace WhoreMasterRenewal
