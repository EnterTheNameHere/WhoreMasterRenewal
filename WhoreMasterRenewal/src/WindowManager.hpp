#ifndef WINDOWMANAGER_HPP_INCLUDED_0854
#define WINDOWMANAGER_HPP_INCLUDED_0854

#include <memory>

#include <SFML/Graphics.hpp>

#include "Logger.hpp"
#include "Helper.hpp"

namespace WhoreMasterRenewal
{

class WindowManager
{
public:
    WindowManager() :
        WindowManager( std::make_shared<sf::RenderWindow>() )
    {}
    
    WindowManager( std::shared_ptr<sf::RenderWindow> renderWindow )
        : m_RenderWindow( renderWindow )
    {
        Logger() << "WindowManager: created...\n";
    }
    
    ~WindowManager()
    {
        if( m_RenderWindow )
            m_RenderWindow = nullptr;
    }
    
    void Initialise()
    {
        Logger() << "WindowManager: Initialise...\n";
        
        sf::VideoMode mode = sf::VideoMode::getDesktopMode();
        // TODO: read window title from config
        m_RenderWindow->create( mode, "WindowTitle" );
    }
    
    void Run()
    {
        if( !m_RenderWindow->isOpen() )
            this->Initialise();
        
        sf::Image sizes;
        sizes.loadFromFile( "Resources/Images/Sizes.jpeg" );
        Logger() << formatString( "Image::getSize [{%}x{%}]\n", sizes.getSize().x, sizes.getSize().y );
        
        sf::Texture sizesTexture;
        sizesTexture.loadFromImage( sizes );
        Logger() << formatString( "Texture::getSize [{%}x{%}]\n", sizesTexture.getSize().x, sizesTexture.getSize().y );
        
        sf::Sprite sizesBackground( sizesTexture );
        
        while( m_RenderWindow->isOpen() )
        {
            
            m_RenderWindow->clear();
            m_RenderWindow->draw( sizesBackground );
            m_RenderWindow->display();
            
            sf::Event event;
            while( m_RenderWindow->pollEvent( event ) )
            {
                
                switch( event.type )
                {
                case sf::Event::Closed:
                    // TODO: Closing event
                    m_RenderWindow->close();
                    break;
                
                case sf::Event::Resized:
                {
                    Logger() << formatString( "New size: [{%}x{%}]\n",
                                             event.size.width,
                                             event.size.height );
                    /*
                    sf::View view = m_RenderWindow->getView();
                    Logger() << formatString( "View: [{%}x{%}]\n",
                                             view.getSize().x,
                                             view.getSize().y );
                    
                    sf::IntRect viewport = m_RenderWindow->getViewport( view );
                    Logger() << formatString( "Viewport: [{%},{%}][{%}x{%}]\n",
                                             viewport.left,
                                             viewport.top,
                                             viewport.width,
                                             viewport.height );
                    
                    view.setSize( static_cast<float>( event.size.width),
                                 static_cast<float>( event.size.height ) );
                    Logger() << formatString( "New view: [{%}x{%}]\n",
                                             view.getSize().x,
                                             view.getSize().y );
                    m_RenderWindow->setView( view );*/
                }
                    break;
                    
                case sf::Event::GainedFocus:
                    // TODO: GotFocus event
                    break;
                
                case sf::Event::LostFocus:
                    // TODO: LostFocus event
                    break;
                    
                case sf::Event::KeyPressed:
                case sf::Event::KeyReleased:
                case sf::Event::MouseButtonPressed:
                case sf::Event::MouseButtonReleased:
                case sf::Event::MouseEntered:
                case sf::Event::MouseLeft:
                case sf::Event::MouseMoved:
                case sf::Event::MouseWheelMoved:
                case sf::Event::TextEntered:
                    break;
                    
                default: // We ignore the rest of events eg. Joystick events
                    break;
                }
            }
            
        }
        
        this->Shutdown();
    }
    
    void Shutdown()
    {
        Logger() << "WindowManager: Shutdown...\n";
    }
    
private:
    std::shared_ptr<sf::RenderWindow> m_RenderWindow = nullptr;
};

} // namespace WhoreMasterRenewal

#endif // WINDOWMANAGER_HPP_INCLUDED_0854
