
#ifndef WHOREMASTERRENEWALWINDOW_HPP_INCLUDED_1402
#define WHOREMASTERRENEWALWINDOW_HPP_INCLUDED_1402
#pragma once

#define SFML_STATIC
#include <SFML/Graphics.hpp>
#define SFGUI_STATIC
#include <SFGUI/SFGUI.hpp>

#include <string>
#include <map>

#include "CLog.h"
#include "WindowFromXMLLoader.hpp"

namespace WhoreMasterRenewal
{

using std::string;

class Screen
{
public:
    Screen( sfg::Desktop& desktop )
    : m_Desktop( desktop )
    {
        //std::clog << "Screen::Screen()\n";

        m_Window->Show( false );
        desktop.Add( m_Window );
    }
    
    Screen( sfg::Desktop& desktop, sfg::Window::Ptr window )
    : m_Desktop( desktop ), m_Window( window )
    {
        m_Window->Show( false );
        desktop.Add( m_Window );
    }
    
    virtual bool IsVisible()
    {
        //std::clog << "Screen::IsVisible()";

        return m_Window->IsGloballyVisible();
    }
    
    virtual void Show()
    {
        //std::clog << "Screen::Show()\n";

        m_Window->Show( true );
    }
    
    virtual void Hide()
    {
        //std::clog << "Screen::Hide()\n";

        m_Window->Show( false );
    }
    
    virtual void Resize( const sf::FloatRect& rect )
    {
        //std::clog << "Screen::Resize()\n";

        m_Window->SetAllocation( rect );
    }
    
protected:
    sfg::Desktop& m_Desktop;
    sfg::Window::Ptr m_Window = { sfg::Window::Create( sfg::Window::Style::BACKGROUND ) };
};

class ConsoleWindow
{
public:
    ConsoleWindow( sfg::Desktop& desktop )
    : m_Desktop( desktop )
    {/*
        m_Window = sfg::Window::Create();
        m_Window->SetTitle( "Console" );
        
        sfg::Box::Ptr scrolledWindowBox = sfg::Box::Create( sfg::Box::Orientation::VERTICAL );
        sfg::Entry::Ptr textBox = sfg::Entry::Create( L"Text Box" );
        sfg::ScrolledWindow::Ptr scrolledWindow = sfg::ScrolledWindow::Create();
        
        scrolledWindow->SetScrollbarPolicy( sfg::ScrolledWindow::ScrollbarPolicy::HORIZONTAL_NEVER
                                            | sfg::ScrolledWindow::ScrollbarPolicy::VERTICAL_ALWAYS );
        scrolledWindow->AddWithViewport( scrolledWindowBox );
        scrolledWindow->SetRequisition( sf::Vector2f( 500.f, 100.f ) );
        
        //sfg::Viewport::Ptr viewport = sfg::Viewport::Create();
        //viewport->SetRequisition( sf::Vector2f( 500.f, 200.f ) );
        
        //sfg::Box::Ptr viewportBox = sfg::Box::Create( sfg::Box::Orientation::VERTICAL );
        
        for( int i = 0; i < 200; i++ )
        {
            std::string str;
            
            for( int j = 0; j < 20; j++ )
            {
                str += static_cast<char>( 65 + rand() % 26 );
            }
            
            //viewportBox->Pack( sfg::Label::Create( str.c_str() ) );
            scrolledWindowBox->Pack( sfg::Label::Create( str ) );
        }
        
        //viewport->Add( viewportBox );
        
        sfg::Box::Ptr box = sfg::Box::Create( sfg::Box::Orientation::VERTICAL );
        box->Pack( scrolledWindow, false, true );
        box->Pack( textBox, true, true );
        
        m_Window->Add( box );
        
        m_Desktop.Add( m_Window );*/
    }
    
private:
    sfg::Desktop& m_Desktop;
    sfg::Window::Ptr m_Window;
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

    void ShowScreen( const string& screenName );

private:
    sfg::SFGUI m_SFGUI;
    sfg::Desktop m_Desktop;

    sf::RenderWindow m_RenderWindow;
    
    std::map<string, Screen> m_Screens = {};
    string m_CurrentScreenName = "MainMenu";
    
    ConsoleWindow m_Console = { ConsoleWindow( m_Desktop ) };

public:
    class MessageBox
    {
    public:
        MessageBox( sfg::Desktop& desktop )
            : m_Desktop( desktop )
        {}
        
    private:
        sfg::Desktop& m_Desktop;
    };

    class ChoiceBox
    {
    public:
        ChoiceBox( sfg::Desktop& desktop )
        : m_Desktop( desktop )
        {}
        
    private:
        sfg::Desktop& m_Desktop;
    };

    class InputBox
    {
    public:
        InputBox( sfg::Desktop& desktop )
        : m_Desktop( desktop )
        {}
        
    private:
        sfg::Desktop& m_Desktop;
    };
};

} // namespace WhoreMasterRenewal

#endif // WHOREMASTERRENEWALWINDOW_HPP_INCLUDED_1402
