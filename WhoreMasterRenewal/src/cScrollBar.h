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
#ifndef CSCROLLBAR_H_INCLUDED_1512
#define CSCROLLBAR_H_INCLUDED_1512
#pragma once

#include "cInterfaceObject.h" // required inheritance

#include <string>

class SDL_Surface;
class SDL_Rect;

namespace WhoreMasterRenewal
{

class cScrollBar;
extern cScrollBar* g_DragScrollBar;

typedef uint8_t Uint8; // Used by SDL

class cScrollBar : public cInterfaceObject
{
public:
    cScrollBar();
    virtual ~cScrollBar();
    
    cScrollBar( const cScrollBar& ) = delete;
    cScrollBar& operator = ( const cScrollBar& ) = delete;
    
    bool CreateScrollBar( int ID, int x, int y, int width, int height, int visibleitems );
    void UpdateScrollBar();  // update size of draggable bar based on total items vs. visible items
    bool IsOver( int x, int y );
    bool MouseDown( int x, int y );
    void DragMove( int y ); // dragging of bar was initiated, so handle movement
    bool ButtonClicked( int x, int y, bool mouseWheelDown = false, bool mouseWheelUp = false );
    void SetTopValue( int itemnum ); // update bar position based on top visible item
    void SetDisabled( bool disable )
    {
        m_Disabled = disable;
        
        if( disable )
            m_ImgBar = m_ImgBarDisabled;
        else
            m_ImgBar = m_ImgBarOff;
    }
    
    virtual void Draw();
    
    void hide() { m_Hidden = true; }
    void unhide() { m_Hidden = false; }
    void toggle() { m_Hidden = !m_Hidden; }
    
    int m_BarHeight = 0;  // height of draggable bar
    int m_SectionHeight = 0;  // height of entire section, not just draggable bar
    int m_BarTop = 0;  // top Y offset of draggable bar
    int m_ItemTop = 0;  // top item visible = 0; correlates to m_BarTop
    int m_ItemsTotal = 0;  // total items
    int m_ItemsVisible = 0;  // visible items
    int m_ItemsTotalLast = 0;  // track last items total, to determine if bar size update is needed
    int m_DragInitYPos = 0;  // track position drag was initiated from
    
    int m_ScrollAmount = 1;  // how much to change the position from mouse scroll up/down events
    int m_PageAmount = 3;  // how much to change the position from clicking in the scroll area above or below the draggable bar
    
    bool m_UpdateSelf = true;  // whether control should update own position when sending updates to parent
    
    bool m_Disabled = false;
    bool m_Hidden = false;
    
    int m_ID = 0;
    
    int* ParentPosition = nullptr;  // pointer to callback value of parent with updated position

private:
    void LoadInitial();
    void LogScrollBarError( std::string description );
    
    
    
    static SDL_Surface* m_ImgBarBG;  // static class-wide shared images
    static SDL_Surface* m_ImgBarOn;  // ...
    static SDL_Surface* m_ImgBarOff;
    static SDL_Surface* m_ImgBarDisabled;
    static SDL_Surface* m_ImgNotches;
    static Uint8 m_NotchOffset;  // Y offset for drawing bar notches
    static SDL_Surface* m_ImgButtonUpOn;
    static SDL_Surface* m_ImgButtonUpOff;
    static SDL_Surface* m_ImgButtonUpDisabled;
    static SDL_Surface* m_ImgButtonDownOn;
    static SDL_Surface* m_ImgButtonDownOff;
    static SDL_Surface* m_ImgButtonDownDisabled;
    
    SDL_Surface* m_ImgBar = nullptr;  // points to the appropriate bar surface above (on, off, or disabled)
    SDL_Surface* m_ImgButtonUp = nullptr;  // same for "Up" button surface
    SDL_Surface* m_ImgButtonDown = nullptr;  // same for "Down" button surface
    SDL_Rect* m_RectBGTop = nullptr;  // rectangle for top half of background to be blitted
    SDL_Rect* m_RectBGBottom = nullptr;  // rectangle for bottom half of background to be blitted
    SDL_Rect* m_RectTop = nullptr;  // rectangle for top half of bar to be blitted
    SDL_Rect* m_RectBottom = nullptr;  // rectangle for bottom half of bar to be blitted
};

} // namespace WhoreMasterRenewal

#endif // CSCROLLBAR_H_INCLUDED_1512
