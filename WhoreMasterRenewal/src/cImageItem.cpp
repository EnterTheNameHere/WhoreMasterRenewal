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

#include "cImageItem.h"
#include "CLog.h"
#include "CGraphics.h"
#include "CSurface.h"
#include "cAnimatedSurface.h"

namespace WhoreMasterRenewal
{

extern CGraphics g_Graphics;

cImageItem::cImageItem() : cInterfaceObject()
{
    ;
}

cImageItem::~cImageItem()
{
    //g_LogFile.ss() << "cImageItem::~cImageItem() [" << this << "] id: \"" << m_ID << "\"";
    //g_LogFile.ssend();
    
    if( m_Next )
        delete m_Next;
        
    m_Next = nullptr;
    
    if( m_Image && m_loaded )
        m_Image.reset();
        
    if( m_Surface )
        SDL_FreeSurface( m_Surface );
        
    m_Surface = nullptr;
    
    if( m_AnimatedImage && m_loaded )
        delete m_AnimatedImage;
        
    m_AnimatedImage = nullptr;
    
    //g_LogFile.ss() << "cImageItem::~cImageItem() finished [" << this << "] id: \"" << m_ID << "\"";
    //g_LogFile.ssend();
}

bool cImageItem::CreateImage( int id, std::string filename, int x, int y, int width, int height, bool statImage, int R, int G, int B )
{
    m_ID = id;
    
    if( statImage )
        {
            m_Surface = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, 32, 0, 0, 0, 0 );
            SDL_FillRect( m_Surface, nullptr, SDL_MapRGB( m_Surface->format, R, G, B ) );
        }
        
    SetPosition( x, y, width, height );
    
    if( filename != "" )
        {
            m_loaded = true;
            m_Image.reset( new CSurface( filename ) );
            m_Image->SetAlpha( true );
        }
    else
        m_loaded = false;
        
    m_Next = nullptr;
    
    return true;
}

bool cImageItem::CreateAnimatedImage( int id, std::string filename, std::string dataFilename, int x, int y, int width, int height )
{
    m_ID = id;
    SetPosition( x, y, width, height );
    
    if( filename != "" )
        {
            m_loaded = true;
            m_Image.reset( new CSurface( filename ) );
            m_Image->SetAlpha( true );
            
            // load the animation
            m_AnimatedImage = new cAnimatedSurface();
            int numFrames, speed, aniwidth, aniheight;
            std::ifstream input;
            input.open( dataFilename.c_str() );
            
            if( !input )
                {
                    g_LogFile.ss() << "Incorrect data file given for animation - " << dataFilename;
                    g_LogFile.ssend();
                    return false;
                }
            else
                input >> numFrames >> speed >> aniwidth >> aniheight;
                
            m_AnimatedImage->SetData( 0, 0, numFrames, speed, aniwidth, aniheight, m_Image );
            input.close();
        }
    else
        {
            g_LogFile.ss() << "Incorrect image file given for animation";
            g_LogFile.ssend();
            m_loaded = false;
            return false;
        }
        
    m_Next = nullptr;
    
    return true;
}

void cImageItem::Draw()
{
    if( m_Hidden )
        return;
        
    if( m_AnimatedImage )
        {
            SDL_Rect rect;
            rect.y = rect.x = 0;
            rect.w = m_Width;
            rect.h = m_Height;
            
            m_AnimatedImage->DrawFrame( m_XPos, m_YPos, m_Width, m_Height, g_Graphics.GetTicks() );
        }
    else if( m_Image )
        {
            SDL_Rect rect;
            rect.y = rect.x = 0;
            rect.w = m_Width;
            rect.h = m_Height;
            
            m_Image->DrawSurface( m_XPos, m_YPos, nullptr, &rect, true );
        }
    else if( m_Surface )
        {
            // Draw the window
            SDL_Rect offset;
            offset.x = m_XPos;
            offset.y = m_YPos;
            
            // blit to the screen
            SDL_BlitSurface( m_Surface, nullptr, g_Graphics.GetScreen(), &offset );
        }
}

} // namespace WhoreMasterRenewal
