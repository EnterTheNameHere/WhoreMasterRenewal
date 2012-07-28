
#ifndef WHOREMASTERRENEWALWINDOW_HPP_1402
#define WHOREMASTERRENEWALWINDOW_HPP_1402
#pragma once

#define SFML_STATIC
#include <SFML/Graphics.hpp>
#define SFGUI_STATIC
#include <SFGUI/SFGUI.hpp>

#include <string>
#include <map>

using std::string;


class Screen
{
public:
    Screen( sfg::Desktop& desktop )
        : m_Desktop( desktop )
    {
        std::clog << "Screen::Screen()\n";
    };

    virtual void Show()
    {
        std::clog << "Screen::Show()\n";

        m_Desktop.RemoveAll();
        m_Desktop.Add( m_Window );
    };

    virtual void Resize( const sf::FloatRect& rect )
    {
        std::clog << "Screen::Resize()\n";

        m_Window->SetAllocation( rect );
    }

protected:
    sfg::Desktop& m_Desktop;
    sfg::Window::Ptr m_Window = { sfg::Window::Create( sfg::Window::Style::BACKGROUND ) };
};


class MainMenuScreen : public Screen
{
public:
    MainMenuScreen( sfg::Desktop &desktop );

private:
    sfg::Button::Ptr m_NewGameButton;
    sfg::Button::Ptr m_LoadGameButton;
    sfg::Button::Ptr m_QuitGameButton;

    sf::Texture m_BackgroundTexture;
};

class WhoreMasterRenewalWindow
{
public:
    WhoreMasterRenewalWindow();
    ~WhoreMasterRenewalWindow();

    void Run();
    
    void ResizeAllScreens( const sf::FloatRect& rect );
    void ResizeAllScreens( const sf::View& view );
    void ResizeAllScreens( const sf::Vector2u& size );

private:
    sfg::SFGUI m_SFGUI;
    sfg::Desktop m_Desktop;
    
    sf::RenderWindow m_RenderWindow;

    std::map<string, Screen> m_Screens =
    {
        { "MainMenu", MainMenuScreen( m_Desktop ) }
    };
    string m_CurrentScreenName = "MainMenu";
};

#endif // WHOREMASTERRENEWALWINDOW_HPP_1402
