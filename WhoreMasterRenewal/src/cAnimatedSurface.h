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
#ifndef CANIMATEDSURFACE_H_INCLUDED_1533
#define CANIMATEDSURFACE_H_INCLUDED_1533
#pragma once

#include <SDL.h> // Required SDL_Rect

#include <string>
#include <memory>

namespace WhoreMasterRenewal
{

class CSurface;

// Class to hold the data for a single animation within an image
class cAnimatedSurface
{
public:
    cAnimatedSurface();
    ~cAnimatedSurface();
    
    cAnimatedSurface( const cAnimatedSurface& ) = delete;
    cAnimatedSurface& operator = ( const cAnimatedSurface& ) = delete;
    
    void PlayOnce( bool playOnce );
    
    void Stop();
    
    bool DrawFrame( int x, int y, int width, int height, unsigned int currentTime ); // Updates animation according to speed, and then draws it on the screen
    void SetData( int xPos, int yPos, int numFrames, int speed, int width, int height, std::shared_ptr<CSurface> surface );
    void UpdateSprite( SDL_Rect& rect, int width, int height );
    
private:
    bool m_FrameDone = false;
    bool m_PlayOnce = false;
    
    int m_CurrentFrame = 0; // Current frame in a playing animation
    int m_CurrentRow = 0;   // current row playing the animation from
    int m_CurrentColumn = 0;
    int m_Speed = 0;    // Speed to play the animation (in time between draws)
    int m_NumFrames = 0;    // Number of frames in the animation
    unsigned int m_LastTime = 0;        // The last time animation was updated
    int m_Rows = 0;
    int m_Colums = 0;
    
    std::shared_ptr<CSurface> m_Surface = nullptr;    // pointer to the image where all the sprites are kept
    SDL_Surface* m_SpriteSurface = nullptr;   // pointer to the image where the current sprite is kept
    SDL_Rect m_Frames = {0, 0, 0, 0}; // Holds the data for ALL frames, since all frames have same width/height
};

// Manages a file with multiple animations
class CAnimatedSprite
{
public:
    CAnimatedSprite();
    ~CAnimatedSprite();
    
    CAnimatedSprite( const CAnimatedSprite& ) = delete;
    CAnimatedSprite& operator = ( const CAnimatedSprite& ) = delete;
    
    void Free();
    
    bool LoadAnimations( std::string imgFilename, std::string animationData );
    bool Draw( int x, int y, int width, int height, unsigned int currentTime );
    void SetAnimation( int animation );
    
private:
    int m_CurrAnimation = 0;
    cAnimatedSurface* m_Animations = nullptr;
    std::shared_ptr<CSurface> m_Image = nullptr;
};

} // namespace WhoreMasterRenewal

#endif // CANIMATEDSURFACE_H_INCLUDED_1533
