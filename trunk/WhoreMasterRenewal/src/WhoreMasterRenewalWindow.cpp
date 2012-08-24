
#include "WhoreMasterRenewalWindow.hpp"

#include <map>

namespace WhoreMasterRenewal
{

MainMenuScreen::MainMenuScreen( sfg::Desktop& desktop )
    : Screen( desktop )
{
    //std::clog << "MainMenuScreen::MainMenuScreen()\n";

    m_NewGameButton = sfg::Button::Create( L"New Game" );
    m_LoadGameButton = sfg::Button::Create( L"Load Game" );
    m_QuitGameButton = sfg::Button::Create( L"Quit Game" );
/*
    sf::Image image;
    image.loadFromFile( "Resources/Buttons/NewGameOff.png" );
    m_NewGameButton->SetImage( sfg::Image::Create( image ) );

    image.loadFromFile( "Resources/Buttons/LoadGameOff.png" );
    m_LoadGameButton->SetImage( sfg::Image::Create( image ) );

    image.loadFromFile( "Resources/Buttons/QuitGameOff.png" );
    m_QuitGameButton->SetImage( sfg::Image::Create( image ) );
*/
    sfg::Box::Ptr outerBox = sfg::Box::Create( sfg::Box::Orientation::HORIZONTAL );
    sfg::Box::Ptr innerBox = sfg::Box::Create( sfg::Box::Orientation::VERTICAL );
    innerBox->Pack( m_NewGameButton );
    innerBox->Pack( m_LoadGameButton );
    innerBox->Pack( m_QuitGameButton );

    sfg::Alignment::Ptr alignment = sfg::Alignment::Create();
    
    alignment->SetScale( sf::Vector2f( 0.f, 0.f ) );
    alignment->Add( innerBox );
    alignment->SetAlignment( sf::Vector2f( 0.5f, 0.5f ) );

    outerBox->Pack( alignment );

    m_Window->Add( outerBox );
}


WhoreMasterRenewalWindow::WhoreMasterRenewalWindow()
    :
        m_Desktop(),
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

    std::map<string, Screen>::iterator it;
    for( it = m_Screens.begin(); it != m_Screens.end(); it++ )
    {
        it->second.Resize( rect );
    }
}

void WhoreMasterRenewalWindow::ResizeAllScreens( const sf::View& view )
{
    //std::clog << "WhoreMasterRenewalWindow::ResizeAllScreens( const sf::View& view )\n";

    sf::FloatRect rect = view.getViewport();
    this->ResizeAllScreens( rect );
}

void WhoreMasterRenewalWindow::ResizeAllScreens( const sf::Vector2u& size )
{
    //std::clog << "WhoreMasterRenewalWindow::ResizeAllScreens( const sf::Vector2u& size )\n";

    sf::FloatRect rect( 0.f, 0.f, static_cast<float>( size.x ), static_cast<float>( size.y ) );
    this->ResizeAllScreens( rect );
}

// TODO: Handling missing screens - show error, leave current screen?
void WhoreMasterRenewalWindow::ShowScreen( const string& screenName )
{
    //std::clog << "WhoreMasterRenewalWindow::ShowScreen( string screenName )\n";

    // Find and hide current Screen
    std::map<string, Screen>::iterator it = this->m_Screens.find( this->m_CurrentScreenName );
    if( it != this->m_Screens.end() )
    {
        // Calling Show for current screen has meaning only if it was somehow hidden
        if( screenName == m_CurrentScreenName )
        {
            if( !it->second.IsVisible() )
                it->second.Show();
            return;
        }

        it->second.Hide();

    }
    else
        std::cerr << __FILE__ << " (" << __LINE__ << "): "
                << "Screen \"" << screenName << "\" not found. Cannot hide it.\n";

    // Set new Screen name and show Screen
    this->m_CurrentScreenName = screenName;

    it = this->m_Screens.find( this->m_CurrentScreenName );
    if( it != this->m_Screens.end() )
        it->second.Show();
    else
        std::cerr << __FILE__ << " (" << __LINE__ << "): "
                << "Screen \"" << screenName << "\" not found. Cannot show it.\n";
}

void WhoreMasterRenewalWindow::Run()
{
    //std::clog << "WhoreMasterRenewalWindow::Run()\n";

    m_RenderWindow.resetGLStates();

    m_SFGUI.TuneAlphaThreshold( .2f );
    m_SFGUI.TuneUseFBO( true );
    m_SFGUI.TuneCull( true );
    
    m_RenderWindow.setFramerateLimit(60);
    
    WindowFromXMLLoader loader;
    
    std::pair<string,Screen> pair( "MainMenu", Screen( m_Desktop, loader.LoadFile( "Resources\\Interface\\Screens\\MainMenu.xml" ) ) );
    m_Screens.insert( pair );
    //loader.LoadFile( "Resources\\Interface\\Screens\\Brothel.xml" );
    //loader.LoadFile( "Resources\\Interface\\Screens\\GirlManagement.xml" );
    //loader.LoadFile( "Resources\\Interface\\Screens\\GirlDetails.xml" );
    
    this->ResizeAllScreens( m_RenderWindow.getSize() );
    this->ShowScreen( "MainMenu" );

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
                m_Desktop.UpdateViewRect( rect );
                this->ResizeAllScreens( rect );
            }

            m_Desktop.HandleEvent( event );
        }

        m_RenderWindow.clear( sf::Color( 255, 0, 0, 255 ) );

        m_Desktop.Update( clock.getElapsedTime().asSeconds() );
        clock.restart();

        m_SFGUI.Display( m_RenderWindow );
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
