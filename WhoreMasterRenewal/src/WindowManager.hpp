
#ifndef WINDOWMANAGER_HPP_INCLUDED_1402
#define WINDOWMANAGER_HPP_INCLUDED_1402
#pragma once

#include "Logger.hpp"

#include <Rocket/Core/String.h>

#include <memory>

//{ Forward Declarations
namespace sf
{
    class RenderWindow;
}
namespace Rocket
{
    namespace Core
    {
        class Context;
        class Event;
    }
}
//}

namespace WhoreMasterRenewal
{


class WindowManager
{
public:
    WindowManager(); /// @brief Constructs default sf::RenderWindow for You
    WindowManager( std::shared_ptr<sf::RenderWindow> windowPtr ); /// @brief You can specify Your own sf::RenderWindow to draw to
    
    WindowManager( const WindowManager& other ) = default;
    WindowManager& operator = ( const WindowManager& other ) = default;
    
    ~WindowManager();
    
    void SetupWindow(); /// @brief Set window style and other properties and make sure it's ready to be displayed...
    
    void Run(); /// @brief Run the window loop. It will manage the user input from now on...
    
    void ShowWindow( const Rocket::Core::String& windowName ); /// @brief Load (if not loaded) windowName from RML file and show it.
    
private:
    void ProcessEvent( Rocket::Core::Event& event, Rocket::Core::String& value ); /// @brief Handler for Events
    
    std::shared_ptr<sf::RenderWindow> m_RenderWindow = nullptr;
    Rocket::Core::Context* m_Context = nullptr;
};


} // namespace WhoreMasterRenewal

#endif // WINDOWMANAGER_HPP_INCLUDED_1402
