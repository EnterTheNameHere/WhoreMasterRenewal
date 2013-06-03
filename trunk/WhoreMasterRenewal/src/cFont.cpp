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

#include "cFont.h"
#include "CLog.h"
#include "CGraphics.h"
#include "sConfig.h"

#include <iostream>
#include <vector>

float FontScale = 1.0f;

cFont::cFont()
{
    m_Font = 0;
    m_MultilineMessage = m_Message = 0;
    m_Text = "";
    m_NewText = true;
    m_IsMultiline = false;
    m_Height = m_Width = 0;
    m_Lineskip = 0;
    m_NumLines = 0;
}

cFont::~cFont()
{
    Free();
}

void cFont::SetText( std::string text )
{
    m_NewText = true;
    m_Text = text;
}

// ok this works by separating strings lines and storing each line into a vector
// then it creates a surface capable of fitting all the lines with the correct width
// it then blits the lines of text to this surface so it is then ready to be drawn as normal
void cFont::RenderMultilineText( std::string text )
{
    if( m_NewText == false && m_MultilineMessage != 0 )
        return;
        
    if( text == "" && m_Text == "" )
        return;
        
    if( text == "" )
        text = m_Text;
        
    text = UpdateLineEndings( text );
    
    // first separate into lines according to width
    std::vector<std::string> lines;
    std::string temp( text ); // current line of text
    temp += " ";    // makes sure that all the text will be displayed
    int n = 0;  // current index into the string
    int q = 0;  // the next \n int the string
    int p = 0;  // holds the previous index into the string
    int charwidth, charheight;
    int width = m_Width - 10;  // pad the sides a bit, it was otherwise slightly overflowing
    
    /// @todo Filter '\r' out too
    // -- Get until either ' ' or '\0'
    while( n != -1 )
    {
        std::string strSub;
        n = temp.find( " ", p + 1 );    // -- Find the next " "
        q = temp.find( "\n", p + 1 );   // -- Find the next "\n"
        
        if( q < n && q != -1 )
        {
            strSub = temp.substr( 0, q );
            GetSize( strSub, charwidth, charheight );
            
            if( charwidth >= width || q == -1 )
            {
                strSub = temp.substr( 0, p );
                lines.push_back( strSub );  // -- Puts strSub into the lines vector
                
                if( q != -1 )
                    temp = temp.substr( p + 1, std::string::npos );
                    
                p = 0;
            }
            else
            {
                strSub = temp.substr( 0, q );
                lines.push_back( strSub );
                
                if( q != -1 )
                    temp = temp.substr( q + 1, std::string::npos );
                    
                p = 0;
            }
        }
        else
        {
            strSub = temp.substr( 0, n );
            GetSize( strSub, charwidth, charheight );
            
            if( charwidth >= width || n == -1 )
            {
                strSub = temp.substr( 0, p );
                lines.push_back( strSub );  // -- Puts strSub into the lines vector
                
                if( n != -1 )
                    temp = temp.substr( p + 1, std::string::npos );
                    
                p = 0;
            }
            else
                p = n;
        }
    }
    
    m_Lineskip = GetFontLineSkip();
    int height = lines.size() * m_Lineskip;
    
    if( m_MultilineMessage )
        SDL_FreeSurface( m_MultilineMessage );
        
    m_MultilineMessage = 0;
    
    // create a surface to render all the text too
    m_MultilineMessage = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF );
    SDL_SetAlpha( m_MultilineMessage, SDL_SRCALPHA, SDL_ALPHA_TRANSPARENT );
    
    std::string otext = m_Text;
    m_NumLines = lines.size();
    
    for( unsigned int i = 0; i < m_NumLines; i++ )
    {
        SetText( lines[i].c_str() );
        
        DrawText( 0, i * m_Lineskip, m_MultilineMessage, true );
    }
    
    m_Text = otext;
    
    if( m_Message )
        SDL_FreeSurface( m_Message );
        
    m_Message = 0;
    
    m_NewText = false;
}

void cFont::RenderText( std::string text, bool multi )
{
    if( m_NewText == false && m_Message != 0 )
        return;
        
    if( m_Font == 0 )
    {
        g_LogFile.ss() << "Error rendering font string: " << text << std::endl;
        g_LogFile.ssend();
        return;
    }
    
    if( m_Message )
        SDL_FreeSurface( m_Message );
        
    m_Message = 0;
    
    if( ( text == "" ) && ( m_Text == "" ) )
        return;
    
    if( text == "" )
        text = m_Text;
    
    // We don't want to display \r since it does square...
    if( text == "\r" )
        return;
        
    cConfig cfg;
    
    if( cfg.fonts.antialias() )
        m_Message = TTF_RenderText_Blended( m_Font, text.c_str(), m_TextColor );
    else
        m_Message = TTF_RenderText_Solid( m_Font, text.c_str(), m_TextColor );
        
    if( m_Message == 0 )
    {
        g_LogFile.write( "Error in RenderText m_Message. Text which was to be rendered: \"" + text + "\"" );
        g_LogFile.write( TTF_GetError() );
        return;
    }
    else if( multi )
        SDL_SetAlpha( m_Message, 0, 0xFF );
        
    m_NewText = false;
}

int cFont::GetWidth()
{
    if( !m_IsMultiline )
    {
        RenderText();
        return m_Message->w;
    }
    else
    {
        RenderMultilineText( "" );
        return m_MultilineMessage->w;
    }
}

int cFont::GetHeight()
{
    if( !m_IsMultiline )
    {
        RenderText();
        return m_Message->h;
    }
    else
    {
        RenderMultilineText( "" );
        return m_MultilineMessage->h;
    }
}

bool cFont::DrawText( int x, int y, SDL_Surface* destination, bool multi )
{
    if( m_Text == "" )
        return true;
        
    if( !m_Font )
        return false;
        
    RenderText( "", multi );
    
    if( m_Message )
    {
        SDL_Rect offset;
        offset.x = x;
        offset.y = y;
        
        // Draw the source surface onto the destination
        int ret = 0;
        
        if( destination )
            ret = SDL_BlitSurface( m_Message, 0, destination, &offset );
        else
            ret = SDL_BlitSurface( m_Message, 0, g_Graphics.GetScreen(), &offset );
            
        if( ret == -1 )
        {
            g_LogFile.ss() << "Error bliting string" << std::endl;
            g_LogFile.ssend();
            return false;
        }
    }
    
    return true;
}

bool cFont::DrawMultilineText( int x, int y, int linesToSkip, int offsetY, SDL_Surface* destination )
{
    if( m_Text == "" )
        return true;
        
    if( !m_Font )
        return false;
        
    RenderMultilineText( "" );
    
    if( m_MultilineMessage )
    {
        SDL_Rect offset;
        offset.x = x + 5; // pad the sides a bit, it was otherwise slightly overflowing
        offset.y = y;
        offset.w = m_Width - 10; // likewise
        
        if( m_MultilineMessage->h < m_Height )
            offset.h = m_MultilineMessage->h;
        else
            offset.h = m_Height;
            
        SDL_Rect srcRect;
        srcRect.x = 0;
        srcRect.y = linesToSkip * m_Lineskip;
        srcRect.y += offsetY;
        srcRect.h = m_Height;
        srcRect.w = m_Width;
        
        // Draw the source surface onto the destination
        int ret = 0;
        
        if( destination )
            ret = SDL_BlitSurface( m_MultilineMessage, &srcRect, destination, &offset );
        else
            ret = SDL_BlitSurface( m_MultilineMessage, &srcRect, g_Graphics.GetScreen(), &offset );
            
        if( ret == -1 )
        {
            g_LogFile.ss() << "Error bliting string" << std::endl;
            g_LogFile.ssend();
            return false;
        }
    }
    
    return true;
}

void cFont::SetColor( unsigned char r, unsigned char g, unsigned char b )
{
    m_TextColor.r = r;
    m_TextColor.g = g;
    m_TextColor.b = b;
}

/*
 * old version for reference (and in case I balls it up
 */
bool cFont::LoadFont( std::string font, int size )
{
    cConfig cfg;
    
    if( m_Font )
        TTF_CloseFont( m_Font );
        
    m_Font = 0;
    
    if( cfg.debug.log_fonts() )
    {
        g_LogFile.ss()
                << "loading font: '"
                << font
                << "' at size "
                << size
                << std::endl;
        g_LogFile.ssend();
    }
    
    if( ( m_Font = TTF_OpenFont( font.c_str(), ( int )( size * FontScale ) ) ) == 0 )
    {
        g_LogFile.write( "Error in LoadFont for font file: " + font );
        g_LogFile.write( TTF_GetError() );
        return false;
    }
    
    return true;
}

void cFont::Free()
{
    if( m_Message )
        SDL_FreeSurface( m_Message );
        
    m_Message = 0;
    
    if( m_MultilineMessage )
        SDL_FreeSurface( m_MultilineMessage );
        
    m_MultilineMessage = 0;
    
// this was consistently causing a crash on exit for me on Win7x64 - Dagoth
//  if(m_Font)
//      TTF_CloseFont(m_Font);
    m_Font = 0;
}

std::string cFont::UpdateLineEndings( std::string text )
{
#ifndef LINUX
    // for Windows, double "\n\n" newline characters were showing up as one newline and a boxy (bad) character...
    // so, here's a cheap-ass workaround to add a "\r" carriage return in front of each "\n" for Windows
    auto pos = text.find( "\n", 0 );
    
    while( pos != std::string::npos )
    {
        text.insert( pos, "\r" );
        pos = text.find( "\n", pos + 2 );
    }
#endif
    return text;
}

void cFont::GetSize( std::string text, int& width, int& height )
{
    TTF_SizeText( m_Font, text.c_str(), &width, &height );
}

void cFont::SetMultiline( bool multi, int width, int height )
{
    m_IsMultiline = multi;
    m_Width = width;
    m_Height = height;
}

int cFont::IsFontFixedWidth()
{
    return TTF_FontFaceIsFixedWidth( m_Font );
}

// returns the height in pixels of the font
int cFont::GetFontHeight()
{
    return TTF_FontHeight( m_Font );
}

// returns the number of pixels you should have between lines
int cFont::GetFontLineSkip()
{
    return TTF_FontLineSkip( m_Font );
}

void cFont::SetFontBold( bool Bold )
{
    TTF_SetFontStyle( m_Font, ( Bold ? TTF_STYLE_BOLD : TTF_STYLE_NORMAL ) );
}

int cFont::GetTotalNumberOfLines()
{
    return m_NumLines;
}

int cFont::GetLinesPerBox()
{
    if( m_Lineskip > 0 )
        return ( m_Height / m_Lineskip );
    else
        return m_Height / GetFontLineSkip();
}

std::string cFont::GetText()
{
    return m_Text;
}
