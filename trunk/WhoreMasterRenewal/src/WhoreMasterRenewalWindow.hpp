
#ifndef WHOREMASTERRENEWALWINDOW_HPP_INCLUDED_1402
#define WHOREMASTERRENEWALWINDOW_HPP_INCLUDED_1402
#pragma once

#define SFML_STATIC
#include <SFML/Graphics.hpp>
#define SFGUI_STATIC
#include <SFGUI/SFGUI.hpp>

#include <string>
#include <map>

namespace WhoreMasterRenewal
{

using std::string;

class Screen
{
public:
    Screen( sfg::Desktop& desktop )
    : m_Desktop( desktop )
    {
        std::clog << "Screen::Screen()\n";

        m_Window->Show( false );
        desktop.Add( m_Window );
    }
    
    virtual bool IsVisible()
    {
        std::clog << "Screen::IsVisible()";

        return m_Window->IsGloballyVisible();
    }
    
    virtual void Show()
    {
        std::clog << "Screen::Show()\n";

        m_Window->Show( true );
    }
    
    virtual void Hide()
    {
        std::clog << "Screen::Hide()\n";

        m_Window->Show( false );
    }
    
    virtual void Resize( const sf::FloatRect& rect )
    {
        std::clog << "Screen::Resize()\n";

        m_Window->SetAllocation( rect );
    }
    
protected:
    sfg::Desktop& m_Desktop;
    sfg::Window::Ptr m_Window = { sfg::Window::Create( sfg::Window::Style::BACKGROUND ) };
};

class SlaveMarketScreen : public Screen
{
public:
    SlaveMarketScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // CurrentBrothel   SlaveMarket      List
        // Text Image
        //
        //
        //
        //
        // List                              Text
        //
        //
        //      BuySelectedSlave MoreDetails Back
    }
};

class JailScreen : public Screen
{
public:
    JailScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Icon  TownPrison     Text
        // List
        //
        //
        //
        //
        //                      Release
        //                      More Details
        //                      Back
    }
};

class BankScreen : public Screen
{
public:
    BankScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Icon  Town
        // Text
        //
        // Deposit
        // Withdraw
        // Deposit All
        // Back
    }
};

class HouseScreen : public Screen
{
public:
    HouseScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Icon  YourHouse
        //
        // Text
        //
        //
        //
        //
        //      Back
    }
};

class TownHallScreen : public Screen
{
public:
    TownHallScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Icon  TownHall
        //
        // Text
        // 
        // 
        // SetBriberyRate  Back
    }
};

class TownScreen : public Screen
{
public:
    TownScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Image                TownHall
        //              YourHouse       TownBank  Brothel6
        //                  Brothel4
        //          Brothel3             Jail
        //                      Brothel5
        //      Brothel1
        //                          Shop
        //          SlaveMarket
        //                      Brothel2
        //
        //
        // Back  WalkAroundTown   CurrentBrothel
    }
};


class TurnSummaryScreen : public Screen
{
public:
    TurnSummaryScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Category  Image
        // List
        //
        // Item
        // List
        //
        // Event
        //
        //
        //
        //                      <-Prev Next->
        //                      GoToSelected
        //                      NextWeek>>
        //                      Back
    }
};

class DungeonScreen : public Screen
{
public:
    DungeonScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        //              Text
        // List                     GirlDetails
        //                          StopFeeding
        //                          AllowFood
        //                          BrandAsSlave
        //                          SellSlave
        //                          Torture
        //                          InteractWith
        //                          Release
        //                          ReleaseAllGirls
        //                          ReleaseAllCustomers
        //
        //                          Back
    }
};

class GangManagementScreen : public Screen
{
public:
    GangManagementScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // ControlledGangs  WeeklyCost        AvailableMissions
        // List                               List
        //
        // Fire                               Text
        //
        // Hire  List
        //
        // WeaponLevel     HealPotions        Nets
        // +               Buy20 AutoBuy      Buy20 AutoBuy
    }
};

class InventoryScreen : public Screen
{
public:
    InventoryScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Owner        CurrentBrothel      Owner
        // List         CategoryFilter      List
        //              List
        //        
        //              PlayerGold
        //              <- Transfer ->
        // Inventory                        Inventory
        // List         Text                List
        // 
        //
        //
        //             Equip/Use  Equip/Use
        //             Unequip    Unequip
        //                     Back
    }
};

class GirlGalleryScreen : public Screen
{
public:
    GirlGalleryScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Image        Anal
        //              BDSM
        //              NormalSex
        //              Bestiality
        //              Group
        //              Lesbian
        //              Pregnant
        //              Death/Attacked
        //              Profile
        //              Back
        //              <-Prev Next->
    }
};

class GirlDetailsScreen : public Screen
{
public:
    GirlDetailsScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Name  Image                                                  JobTypes
        // Text                                                         List
        //                                                              Jobs
        //                                                              TabList
        //                                                              Traits
        //                                                              List
        //
        // MoreDetails                                                  HousePercentage
        // UseAntiPreg                                                  Slider
        // Prev  Next          ManageInventory          InteractWith    ViewGallery
        // AccomUp  AccomDown  SendToDungeon/Release    TakeGold        Back
        
        // InteractWith:
        // Shows Message "What You would like to do with her?"
        // Shows Message "Select choice"
        //                List
    }
};


class TransferGirlsScreen : public Screen
{
public:
    TransferGirlsScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Brothels                 Brothels
        // List     List    List    List
        //
        //
        //
        //
        //               <-
        //               ->
        //
        //
        //
        //
        //              Back
    }
};


class GirlManagementScreen : public Screen
{
public:
    GirlManagementScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Label                   Image
        // List                    Text
        //
        //
        //                         GirlDetails
        //                         TransferGirls
        //                         FreeSlave
        //                         SellSlave
        // JobTypes Jobs           FireGirl
        // List     TabList  Text  Back
    }
};


class BrothelSetupScreen : public Screen
{
public:
    BrothelSetupScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        //                      CurrentBrothel
        // 
        // AntiPregPotions      BarStaff            CasinoStaff
        // Buy10  Buy20         Hire  Fire          Hire  Fire
        // AutoBuy  YouHave
        //
        // Slider                           AddRoom  Add5Rooms
        // AdvertisingBudget
        //
        // SexRestrictions   ProhibitAnal  BDSM      Lesbian
        //                   Bestiality    GroupSex  NormalSex
        //
        //                      Back
    }
};

class BrothelMainScreen : public Screen
{
public:
    BrothelMainScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        // Label         Stats
        // Image
        //               Prev Next
        //               Girls
        //               Gangs
        //               Brothel
        //
        //               Dungeon
        //
        //               Town
        //
        // Quit Save   Summary NextWeek
    }
};

class NewGameScreen : public Screen
{
public:
    NewGameScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        
    }
};

class LoadGameScreen : public Screen
{
public:
    LoadGameScreen( sfg::Desktop& desktop )
        : Screen( desktop )
    {
        
    }
};

class MainMenuScreen : public Screen
{
public:
    MainMenuScreen( sfg::Desktop& desktop );

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

    void ShowScreen( const string& screenName );

private:
    sfg::SFGUI m_SFGUI;
    sfg::Desktop m_Desktop;

    sf::RenderWindow m_RenderWindow;

    std::map<string, Screen> m_Screens =
    {
        { "MainMenu", MainMenuScreen( m_Desktop ) },
        { "SlaveMarket", SlaveMarketScreen( m_Desktop ) },
        { "Jail", JailScreen( m_Desktop ) },
        { "Bank", BankScreen( m_Desktop ) },
        { "House", HouseScreen( m_Desktop ) },
        { "TownHall", TownHallScreen( m_Desktop ) },
        { "Town", TownScreen( m_Desktop ) },
        { "TurnSummary", TurnSummaryScreen( m_Desktop ) },
        { "Dungeon", DungeonScreen( m_Desktop ) },
        { "GangManagement", GangManagementScreen( m_Desktop ) },
        { "Inventory", InventoryScreen( m_Desktop ) },
        { "GirlGallery", GirlGalleryScreen( m_Desktop ) },
        { "GirlDetails", GirlDetailsScreen( m_Desktop ) },
        { "TransferGirls", TransferGirlsScreen( m_Desktop ) },
        { "GirlManagement", GirlManagementScreen( m_Desktop ) },
        { "BrothelSetup", BrothelSetupScreen( m_Desktop ) },
        { "Brothel", BrothelMainScreen( m_Desktop ) },
        { "NewGame", NewGameScreen( m_Desktop ) },
        { "LoadGame", LoadGameScreen( m_Desktop ) }
    };
    string m_CurrentScreenName = "MainMenu";

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
