
#include "CTimer.h"

#include <SDL.h>

namespace WhoreMasterRenewal
{

CTimer::CTimer()
{
    m_StartTicks = 0;
    m_PausedTicks = 0;
    m_Paused = false;
    m_Started = false;
}

CTimer::~CTimer()
{
    ;
}

void CTimer::Start()
{
    m_Paused = false;
    m_Started = true;
    m_StartTicks = SDL_GetTicks();
}

void CTimer::Stop()
{
    m_Paused = false;
    m_Started = false;
}

void CTimer::Pause( bool pause )
{
    if( pause )
    {
        if( m_Started && !m_Paused )
        {
            m_Paused = true;
            m_PausedTicks = SDL_GetTicks() - m_StartTicks;
        }
    }
    else
    {
        m_Paused = false;
        m_StartTicks = SDL_GetTicks() - m_PausedTicks;
        m_PausedTicks = 0;
    }
}

int CTimer::GetTicks()
{
    if( m_Started )
    {
        if( m_Paused )
            return m_PausedTicks;
        else
            return ( SDL_GetTicks() - m_StartTicks );
    }
    return 0;
}

bool CTimer::IsStarted()
{
    return m_Started;
}

bool CTimer::IsPaused()
{
    return m_Paused;
}

} // namespace WhoreMasterRenewal
