
#ifndef WHOREMASTERRENEWALWINDOW_HPP_INCLUDED_1402
#define WHOREMASTERRENEWALWINDOW_HPP_INCLUDED_1402
#pragma once

#include <SFML/Graphics.hpp>

#include <string>
#include <map>

#include "CLog.h"

namespace WhoreMasterRenewal
{

class Screen
{
public:
    Screen()
    {}
};

class ConsoleWindow
{
public:
    ConsoleWindow()
    {}
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

    void ShowScreen( const std::string& screenName );

private:
    sf::RenderWindow m_RenderWindow;
    
    std::map<std::string, Screen> m_Screens = {};
    std::string m_CurrentScreenName = "MainMenu";
    
    ConsoleWindow m_Console = { ConsoleWindow() };

public:
    class MessageBox
    {
    public:
        MessageBox()
        {}
    };

    class ChoiceBox
    {
    public:
        ChoiceBox()
        {}
    };

    class InputBox
    {
    public:
        InputBox()
        {}
    };
};

} // namespace WhoreMasterRenewal

#endif // WHOREMASTERRENEWALWINDOW_HPP_INCLUDED_1402
