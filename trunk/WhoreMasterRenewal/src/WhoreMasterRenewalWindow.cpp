
#include "WhoreMasterRenewalWindow.hpp"

#include <map>

namespace WhoreMasterRenewal
{

WhoreMasterRenewalWindow::WhoreMasterRenewalWindow()
    :
        //TODO: use custom resolution from config
        m_RenderWindow( sf::VideoMode( 1024, 768, 32 ), "Whore Master Renewal" )
{
    //std::clog << "WhoreMasterRenewalWindow::WhoreMasterRenewalWindow()\n";
}

WhoreMasterRenewalWindow::~WhoreMasterRenewalWindow()
{
    //std::clog << "WhoreMasterRenewalWindow::~WhoreMasterRenewalWindow()\n";
}

void WhoreMasterRenewalWindow::ResizeAllScreens( const sf::FloatRect& rect )
{
    //std::clog << "WhoreMasterRenewalWindow::ResizeAllScreens( const sf::FloatRect& rect )\n";
    /*std::clog << "\trect [left:" << rect.left
                    << ", top:" << rect.top
                    << ", width:" << rect.width
                    << ", height:" << rect.height << "]\n";*/
}

void WhoreMasterRenewalWindow::ResizeAllScreens( const sf::View& view )
{
    //std::clog << "WhoreMasterRenewalWindow::ResizeAllScreens( const sf::View& view )\n";
}

void WhoreMasterRenewalWindow::ResizeAllScreens( const sf::Vector2u& size )
{
    //std::clog << "WhoreMasterRenewalWindow::ResizeAllScreens( const sf::Vector2u& size )\n";
}

// TODO: Handling missing screens - show error, leave current screen?
void WhoreMasterRenewalWindow::ShowScreen( const string& screenName )
{
    //std::clog << "WhoreMasterRenewalWindow::ShowScreen( string screenName )\n";

    // Find and hide current Screen
}

void WhoreMasterRenewalWindow::Run()
{
    //std::clog << "WhoreMasterRenewalWindow::Run()\n";

    m_RenderWindow.resetGLStates();
    m_RenderWindow.setFramerateLimit(60);
    
    this->ResizeAllScreens( m_RenderWindow.getSize() );
    this->ShowScreen( "GirlManagement" );
    
    sf::Clock fpsClock;
    unsigned int fpsCount = 0;
    
    sf::Clock clock;
    sf::Event event;
    
    while( m_RenderWindow.isOpen() )
    {
        while( m_RenderWindow.pollEvent( event ) )
        {
            if( event.type == sf::Event::EventType::Closed )
            {
                m_RenderWindow.close();
            }
            else if( event.type == sf::Event::EventType::Resized )
            {
                sf::FloatRect rect( 0.f, 0.f, static_cast<float>( event.size.width ), static_cast<float>( event.size.height ) );
                this->ResizeAllScreens( rect );
            }
        }

        m_RenderWindow.clear( sf::Color( 255, 0, 0, 255 ) );

        clock.restart();

        m_RenderWindow.display();

        if( fpsClock.getElapsedTime().asSeconds() > 1.f )
        {
            std::stringstream sstream;
            sstream << "Whore Master Renewal [" << fpsCount << " FPS]";

            m_RenderWindow.setTitle( sstream.str() );

            fpsCount = 0;
            fpsClock.restart();
        }

        fpsCount++;
    }
}

} // namespace WhoreMasterRenewal
