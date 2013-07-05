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

#include "CSurface.h"
#include "CLog.h"
#include "CGraphics.h"

#include <SDL_image.h>
#include <SDL_rotozoom.h>

namespace WhoreMasterRenewal
{

CSurface::CSurface() : CResource()
{
    ;
}

CSurface::CSurface( SDL_Surface* inputsurface ) : CResource()
{
    LoadSurface( inputsurface );
}

CSurface::CSurface( std::string filename ) : CResource(),
    m_Filename( filename )
{
    ;
}

CSurface::~CSurface()
{
    m_SaveSurface = false;
    Free();
}

void CSurface::Free()
{
    //g_LogFile.ss() << "CSurface::~Free() [" << this << "] Filename \"" << m_Filename << "\"";
    //g_LogFile.ssend();
    
    if( m_SaveSurface )
        return;
    
//    if( m_Temp )
//        SDL_FreeSurface( m_Temp );
    
    m_Temp = nullptr;
    
//    if( m_Surface )
//        SDL_FreeSurface( m_Surface );
    
    m_Surface = nullptr;
    
//    if( m_SpriteImage )
//        SDL_FreeSurface( m_SpriteImage );
    
    m_SpriteImage = nullptr;
    
    //g_LogFile.ss() << "CSurface::Free() finished [" << this << "] Filename \"" << m_Filename << "\"";
    //g_LogFile.ssend();
}

void CSurface::FreeResources()
{
    if( m_ColoredSurface )
        return;
        
    Free();
}

bool CSurface::LoadImage( std::string filename, bool load )
{
    m_Filename = filename;
    
    if( load == false )
    {
        return true;
    }
    
    SDL_Surface* loadedImage = nullptr;
    
    // Load image
//    loadedImage = IMG_Load( filename.c_str() );
    
    if( loadedImage )
    {
        // Convert image to screen BPP
//        if( m_UseAlpha )
//            m_Surface = SDL_DisplayFormatAlpha( loadedImage );
//        else
//            m_Surface = SDL_DisplayFormat( loadedImage );
            
        // Free old surface
//        SDL_FreeSurface( loadedImage );
    }
    else
    {
        g_LogFile.write( "CSurface LoadImage loadedImage error with filename: " + m_Filename );
//        g_LogFile.write( SDL_GetError() );
//        g_LogFile.write( IMG_GetError() );
        return false;
    }
    
    if( !m_Surface )
    {
        g_LogFile.write( "CSurface LoadImage !m_Surface error with filename: " + m_Filename );
//        g_LogFile.write( SDL_GetError() );
//        g_LogFile.write( IMG_GetError() );
        return false;
    }
    
//    if( m_UseKey && !m_UseAlpha )
//        SDL_SetColorKey( m_Surface, SDL_SRCCOLORKEY, m_ColorKey );
    
    return true;
}

bool CSurface::LoadSurface( SDL_Surface* inputsurface )
{
    SDL_Surface* loadedImage = inputsurface;
    
    // Convert image to screen BPP
//    if( m_UseAlpha )
//        m_Surface = SDL_DisplayFormatAlpha( loadedImage );
//    else
//        m_Surface = SDL_DisplayFormat( loadedImage );
    
    if( !m_Surface )
    {
        g_LogFile.write( "CSurface LoadSurface !m_Surface error with filename: " + m_Filename );
//        g_LogFile.write( SDL_GetError() );
//        g_LogFile.write( IMG_GetError() );
        return false;
    }
    
//    if( m_UseKey && !m_UseAlpha )
//        SDL_SetColorKey( m_Surface, SDL_SRCCOLORKEY, m_ColorKey );
    
    loaded = true;
    
    return true;
}

void CSurface::SetColorKey( unsigned char r, unsigned char g, unsigned char b )
{
//    m_ColorKey = SDL_MapRGB( g_Graphics.GetScreen()->format, r, g, b );
}

void CSurface::SetAlpha( bool UseAlpha )
{
    m_UseAlpha = UseAlpha;
}

bool CSurface::ResizeSprite( SDL_Surface* image, SDL_Rect* clip, bool maintainRatio )
{
    double scaleX, scaleY;
    
//    if( m_SpriteImage ) // free old image
//        SDL_FreeSurface( m_SpriteImage );
        
    m_SpriteImage = nullptr;
    
    if( maintainRatio == true )
    {
        if( clip->w != image->w && clip->h != image->h )
        {
            if( image->w > image->h ) // if the width is larger so scale down based on the width but keep aspect ratio
                scaleX = scaleY = ( ( double )clip->w / ( double )image->w );
            else    // assume the height is larger
                scaleX = scaleY = ( ( double )clip->h / ( double )image->h );
                
//            m_SpriteImage = zoomSurface( image, scaleX, scaleY, 1 );
        }
    }
    else
    {
        if( clip->w != image->w || clip->h != image->h )
        {
            scaleX = ( ( double )clip->w / ( double )image->w );
            scaleY = ( ( double )clip->h / ( double )image->h );
//            m_SpriteImage = zoomSurface( image, scaleX, scaleY, 1 );
        }
    }
    
    return true;
}

bool CSurface::DrawSprite( int x, int y )
{
    if( m_SpriteImage == nullptr )
    {
        g_LogFile.ss() <<   "ERROR - Draw Sprite, null surface";
        g_LogFile.ssend();
        return false;
    }
    
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    
//    SDL_BlitSurface( m_SpriteImage, nullptr, g_Graphics.GetScreen(), &offset );
    
    return true;
}

bool CSurface::DrawSurface( int x, int y, SDL_Surface* destination, SDL_Rect* clip, bool resize, bool maintainRatio )
{
    double scaleX = 0;
    double scaleY = 0;
    
    /*
     *  bit of a kludge to stop buttons with a missing
     *  disabled image from spamming the log file
     */
    if( m_Filename == ".\\Resources\\Buttons\\"
            || m_Filename == "./Resources/Buttons/" )
    {
        return false;
    }
    
    if( m_Surface == nullptr )
    {
        if( m_ColoredSurface == true )
        {
            g_LogFile.ss() <<   "ERROR - Colored surface null: '"
                           << m_Filename << "'"
                           ;
            g_LogFile.ssend();
            return false;
        }
        
//        if( m_Temp )
//            SDL_FreeSurface( m_Temp );
            
        m_Temp = nullptr;
        
        if( !loaded )
        {
//            if( !LoadImage( m_Filename ) )
//            {
//                g_LogFile.ss()
//                        <<  "ERROR - Loading Image '"
//                        <<  m_Filename
//                        <<  "'"
//                        ;
//                g_LogFile.ssend();
//                return false;
//            }
            
            if( m_Cached )
            {
                loaded = true;
            }
        }
    }
    
    m_TimeUsed = g_Graphics.GetTicks();
    
    if( clip && resize )
    {
        if( m_Temp == nullptr )
        {
            if( maintainRatio == true )
            {
                if( clip->w != m_Surface->w && clip->h != m_Surface->h )
                {
                    if( m_Surface->w > m_Surface->h ) // if the width is larger so scale down based on the width but keep aspect ratio
                        scaleX = scaleY = ( ( double )clip->w / ( double )m_Surface->w );
                    else    // assume the height is larger
                        scaleX = scaleY = ( ( double )clip->h / ( double )m_Surface->h );
                        
//                    m_Temp = zoomSurface( m_Surface, scaleX, scaleY, 1 );
                }
            }
            else
            {
                if( clip->w != m_Surface->w || clip->h != m_Surface->h )
                {
                    scaleX = ( ( double )clip->w / ( double )m_Surface->w );
                    scaleY = ( ( double )clip->h / ( double )m_Surface->h );
//                    m_Temp = zoomSurface( m_Surface, scaleX, scaleY, 1 );
                }
            }
        }
        else
        {
            if( maintainRatio == true )
            {
                if( m_Temp->w != clip->w && m_Temp->h != clip->h )
                {
//                    if( m_Temp ) // free old image
//                        SDL_FreeSurface( m_Temp );
                        
                    m_Temp = nullptr;
                    
                    if( m_Surface->w > m_Surface->h ) // if the width is larger so scale down based on the width but keep aspect ratio
                        scaleX = scaleY = ( ( double )clip->w / ( double )m_Surface->w );
                    else    // assume the height is larger
                        scaleX = scaleY = ( ( double )clip->h / ( double )m_Surface->h );
                        
//                    m_Temp = zoomSurface( m_Surface, scaleX, scaleY, 1 );
                }
            }
            else
            {
                if( m_Temp->w != clip->w || m_Temp->h != clip->h )
                {
//                    if( m_Temp ) // free old image
//                        SDL_FreeSurface( m_Temp );
                        
                    m_Temp = nullptr;
                    
                    scaleX = ( ( double )clip->w / ( double )m_Surface->w );
                    scaleY = ( ( double )clip->h / ( double )m_Surface->h );
//                    m_Temp = zoomSurface( m_Surface, scaleX, scaleY, 1 );
                }
            }
        }
    }
    
    // create and setup a SDL offset rectangle
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    
    // Draw the source surface onto the destination
    int error = 0;
    
    if( destination )
    {
//        if( m_Temp )
//            error = SDL_BlitSurface( m_Temp, clip, destination, &offset );
//        else
//            error = SDL_BlitSurface( m_Surface, clip, destination, &offset );
    }
    else    // blit to the screen
    {
//        if( m_Temp )
//            error = SDL_BlitSurface( m_Temp, clip, g_Graphics.GetScreen(), &offset );
//        else
//            error = SDL_BlitSurface( m_Surface, clip, g_Graphics.GetScreen(), &offset );
    }
    
    if( error == -1 )
    {
//        g_LogFile.ss() << "Error Blitting surface (" << m_Filename << ") - " << SDL_GetError();
        g_LogFile.ssend();
        return false;
    }
    
    return true;
}

void CSurface::MakeColoredSurface( int width, int height, int red, int green, int blue, int BPP )
{
    m_ColoredSurface = true;
//    m_Surface = SDL_CreateRGBSurface( SDL_SWSURFACE, width, height, BPP, 0, 0, 0, 0 );
//    SDL_FillRect( m_Surface, nullptr, SDL_MapRGB( m_Surface->format, red, green, blue ) );
}

} // namespace WhoreMasterRenewal
