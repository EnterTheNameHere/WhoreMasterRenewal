
#ifndef WINDOWMANAGER_HPP_INCLUDED_1402
#define WINDOWMANAGER_HPP_INCLUDED_1402
#pragma once

#include <SFML/Graphics.hpp>

namespace WhoreMasterRenewal
{

class ScreenManager
{
public:
    ScreenManager( sf::RenderWindow* );
    ~ScreenManager();
    
    ScreenManager( const ScreenManager& );
    ScreenManager& operator = ( const ScreenManager& );
    
private:
    sf::RenderWindow* m_RenderWindow = {nullptr};
};

} // namespace WhoreMasterRenewal

#endif // WINDOWMANAGER_HPP_INCLUDED_1402
