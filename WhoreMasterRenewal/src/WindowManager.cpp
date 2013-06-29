
#include "WindowManager.hpp"

namespace WhoreMasterRenewal
{

ScreenManager::ScreenManager( sf::RenderWindow* window )
    : m_RenderWindow( window )
{
    
}

ScreenManager::~ScreenManager()
{
    if( m_RenderWindow != nullptr )
        delete m_RenderWindow;
}

} // namespace WhoreMasterRenewal
