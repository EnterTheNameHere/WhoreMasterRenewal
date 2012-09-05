/*
 * Copyright 2009, 2010, The Pink Petal Development Team.
 * The Pink Petal Devloment Team are defined as the game's coders
 * who meet on http://pinkpetal.co.cc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "main.h"
#include "InterfaceGlobals.h"
#include "GameFlags.h"
#include "InterfaceProcesses.h"
#include "sConfig.h"
#include "CSurface.h"
#include "cJobManager.h"
#include "Revision.h"
#ifndef LINUX
	#ifdef _DEBUG
// to enable leak detection uncomment the below and the first comment in main()
/*		#ifndef _CRTDBG_MAP_ALLOC
			#define _CRTDBG_MAP_ALLOC
		#endif
		#include <stdlib.h>
		#include <crtdbg.h>*/
	#endif
#else
	#include "linux.h"
#endif
#include <signal.h>
#include <sstream>

#include "IconSurface.h"
#include <string>

extern int g_ScreenWidth, g_ScreenHeight;
extern bool g_Fullscreen;
sInterfaceIDs g_interfaceid;

// Function Defs

void Shutdown();
bool Init();

int g_CurrBrothel = 0;
unsigned int g_LastSeed = 0;		// for seeding the random number generater every 3 seconds (3000 ticks)
bool eventrunning = false;
bool newWeek = false;

bool g_ShiftDown = false;
bool g_CTRLDown = false;

bool g_LeftArrow = false;
bool g_RightArrow = false;
bool g_UpArrow = false;
bool g_DownArrow = false;

bool g_EnterKey = false;

cScrollBar* g_DragScrollBar = 0;  // if a scrollbar is being dragged, this points to it
cSlider* g_DragSlider = 0;  // if a slider is being dragged, this points to it

extern CSurface* g_BrothelImages[6];
extern bool g_InitWin;
extern sGirl* MarketSlaveGirls[8];
extern int MarketSlaveGirlsDel[8];

// SDL Graphics interface
CGraphics g_Graphics;

// Resource Manager
CResourceManager rmanager;

// Events
SDL_Event vent;

// logfile
CLog g_LogFile(true);

// Trait list
cTraits g_Traits;

// Girl manager
cGirls g_Girls;
cAbstractGirls *g_GirlsPtr = &g_Girls;

// Brothel Manager
cBrothelManager g_Brothels;

// Gang Manager
cGangManager g_Gangs;

// Customer Manager
cCustomers g_Customers;

// the background image
CSurface* g_BackgroundImage = 0;

// The global trigger manager
cTriggerList g_GlobalTriggers;

// Holds the currently running script

cWindowManager g_WinManager;

// Keeping time in the game
unsigned long g_Year;
unsigned long g_Month;
unsigned long g_Day;

// the players gold
cGold g_Gold;

// Inventory manager
cInventory g_InvManager;

// TEmporary testing crap
int IDS = 0;
cRng g_Dice;

void handle_hotkeys()
{
	if(vent.key.keysym.sym == SDLK_RSHIFT || vent.key.keysym.sym == SDLK_LSHIFT)	// enable multi select
		g_ShiftDown = true;
	else if(vent.key.keysym.sym == SDLK_RCTRL || vent.key.keysym.sym == SDLK_LCTRL)	// enable multi select
		g_CTRLDown = true;

	if(vent.key.keysym.sym == SDLK_UP)
		g_UpArrow = true;
	else if(vent.key.keysym.sym == SDLK_DOWN)
		g_DownArrow = true;
	else if(vent.key.keysym.sym == SDLK_LEFT)
		g_LeftArrow = true;
	else if(vent.key.keysym.sym == SDLK_RIGHT)
		g_RightArrow = true;

	if(g_WinManager.GetWindow() != &g_MainMenu)
	{
		int br_no = 0;

		switch(vent.key.keysym.sym) {
		case SDLK_1: case SDLK_2: case SDLK_3:
		case SDLK_4: case SDLK_5: case SDLK_6:
			br_no = vent.key.keysym.sym - SDLK_1;
			if(g_Brothels.GetNumBrothels() > br_no) {
				g_CurrBrothel = br_no;
				g_InitWin = true;
			}
			break;
		case SDLK_d:
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Dungeon");
			break;
		case SDLK_s:	// slave market screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Town");
			g_WinManager.push("Slave Market");
			break;
		case SDLK_m:	// mayors office screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
/*
 *			this will make "m" go to brothel management
 *			you need "M" to go to the mayor screen now
 *			which is far less used, I think, and easy to get
 *			to from the town screen
 *
 *			we should consider some customisable keyboard mapping
 *			mechanism at some point
 */
			if(g_ShiftDown) {
				g_WinManager.push("Town");
				g_WinManager.push("Mayor");
			}
			break;
		case SDLK_b:	// bank screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Town");
			g_WinManager.push("Bank");
			break;
		case SDLK_g:	// girl management screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Girl Management");
			break;
		case SDLK_t:	// staff management screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Gangs");
			break;
		case SDLK_u:	// upgrades management screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			if(g_ShiftDown) {
				g_WinManager.push("Building Management");
			}
			else {
				g_WinManager.push("Building Setup");
			}
			break;
		case SDLK_a:	// turn summary screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.Push(Turnsummary, &g_Turnsummary);
			break;
		case SDLK_o:	// town screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Town");
			break;
		case SDLK_p:	// shop screen
			g_WinManager.PopToWindow(&g_BrothelManagement);
			g_InitWin = true;
			g_WinManager.push("Town");
			g_WinManager.push("Item Management");
			break;
		default:
			// do nothing, but the "default" clause silences an irritating warning
			break;
		}
	}
}


#define SFML_STATIC
#include <SFML/Graphics.hpp>
#define SFGUI_STATIC
#include <SFGUI/SFGUI.hpp>

/*
class HelloWorld
{
public:
    HelloWorld();
    ~HelloWorld();
    void Run();

private:
    void OnAddButtonHClicked();
    void OnAddButtonVClicked();
    void OnToggleTitlebarClicked();
    void OnHideWindowClicked();
    void OnToggleSpaceClicked();
    void OnLimitCharsToggled();
    void OnLoadThemeClicked();
    void OnAdjustementChanged();
    void OnToggleSpinner();

    sfg::SFGUI m_SFGUI;

    sfg::Window::Ptr m_MainWindow;
    sfg::Box::Ptr m_BoxButtonsH;
    sfg::Box::Ptr m_BoxButtonsV;
    sfg::Entry::Ptr m_Entry;
    sfg::Table::Ptr m_Table;
    sfg::ScrolledWindow::Ptr m_ScrolledWindow;
    sfg::Box::Ptr m_ScrolledWindowBox;
    sfg::ToggleButton::Ptr m_TitleBarToggleButton;
    sfg::CheckButton::Ptr m_LimitCheckButton;
    sfg::Scale::Ptr m_Scale;
    sfg::ComboBox::Ptr m_ComboBox;
    sfg::ProgressBar::Ptr m_ProgressBar;
    sfg::ProgressBar::Ptr m_ProgressBarVertical;
    sfg::Spinner::Ptr m_Spinner;

    sfg::Desktop m_Desktop;

    unsigned int m_FPS_Counter;
    sf::Clock m_FPS_Clock;

    sf::Texture m_BackgroundTexture;
    sf::Sprite m_BackgroundSprite;

    sf::RenderWindow m_RenderWindow;
};

class Ouchy : public sfg::EnableSharedFromThis<Ouchy>
{
public:
    typedef sfg::SharedPtr<Ouchy> Ptr;
    Ouchy( sfg::Button::Ptr button );

    void DoOuch();

    static std::vector<Ptr> m_Ouchies;

private:
    sfg::Button::Ptr m_Button;
    bool m_State;
};

std::vector<Ouchy::Ptr> Ouchy::m_Ouchies;

Ouchy::Ouchy( sfg::Button::Ptr button ) :
    m_Button( button ),
    m_State( false )
{}

void Ouchy::DoOuch()
{
    if( !m_State )
        m_Button->SetLabel( "Ouch" );
    else
        m_Button->SetLabel( "Boom" );

    m_State = !m_State;
}

HelloWorld::HelloWorld() :
    m_Desktop(),
    m_RenderWindow( sf::VideoMode( 1024, 768, 32 ), "Whore Master Renewal", sf::Style::Default, sf::ContextSettings( 16, 0, 0, 2, 1 ) )
{
    m_BackgroundTexture.create( 1024, 768 );

    sf::Uint8* pixels = new sf::Uint8[ 1024 * 768 * 4 ];
    sf::Uint8 pixelValue = 139;

    for( std::size_t index = 0; index < 1024 * 768; ++index )
    {
        pixelValue = static_cast<sf::Uint8>( pixelValue ^ ( index + 809 ) );
        pixelValue = static_cast<sf::Uint8>( pixelValue << ( index % 11 ) );
        pixelValue = static_cast<sf::Uint8>( pixelValue * 233 );

        pixels[ index * 4 + 0 ] = static_cast<sf::Uint8>( pixelValue % 16 + 72 );

        pixelValue ^= static_cast<sf::Uint8>( index );
        pixelValue = static_cast<sf::Uint8>( pixelValue * 23 );

        pixels[ index * 4 + 1 ] = static_cast<sf::Uint8>( pixelValue % 16 + 72 );

        pixelValue ^= static_cast<sf::Uint8>( index );
        pixelValue = static_cast<sf::Uint8>( pixelValue * 192 );

        pixels[ index * 4 + 2 ] = static_cast<sf::Uint8>( pixelValue % 16 + 72 );

        pixels[ index * 4 + 3 ] = 255;
    }

    m_BackgroundTexture.update( pixels );

    m_BackgroundSprite.setTexture( m_BackgroundTexture );

    delete[] pixels;
}

HelloWorld::~HelloWorld()
{
    Ouchy::m_Ouchies.clear();
}

void HelloWorld::Run()
{
    m_SFGUI.TuneUseFBO( true );
    m_SFGUI.TuneAlphaThreshold( .2f );
    m_SFGUI.TuneCull( true );

    m_MainWindow = sfg::Window::Create( sfg::Window::TITLEBAR | sfg::Window::BACKGROUND | sfg::Window::RESIZE );
    m_MainWindow->SetTitle( L"Example application" );

    sfg::Button::Ptr btnAddButtonH( sfg::Button::Create( L"Add button horizontally" ) );
    sfg::Button::Ptr btnAddButtonV( sfg::Button::Create( L"Add button vertically" ) );

    m_TitleBarToggleButton = sfg::ToggleButton::Create( L"Toggle titlebar" );
    m_TitleBarToggleButton->SetActive( true );

    {
        sf::Image addImage;
        if( addImage.loadFromFile( "Resources/Buttons/plus_24x24On.png" ) )
        {
            sfg::Image::Ptr image( sfg::Image::Create( addImage ) );
            btnAddButtonH->SetImage( image );

            image = sfg::Image::Create( addImage );
            btnAddButtonV->SetImage( image );
        }
    }

    sfg::Button::Ptr btnHideWindow( sfg::Button::Create( L"Close window" ) );
    btnHideWindow->SetId( "close" );

    {
        sf::Image closeImage;
        if( closeImage.loadFromFile( "Resources/Buttons/minus_24x24On.png" ) )
        {
            sfg::Image::Ptr image( sfg::Image::Create( closeImage ) );
            btnHideWindow->SetImage( image );
        }
    }

    sfg::Button::Ptr btnToggleSpace( sfg::Button::Create( L"Box Spacing") );
	sfg::Button::Ptr btnLoadStyle( sfg::Button::Create( L"Load theme") );

	m_Entry = sfg::Entry::Create( L"Type" );
	m_Entry->SetRequisition( sf::Vector2f( 100.f, .0f ) );
	m_Entry->AppendText( L" something!" );

	m_LimitCheckButton = sfg::CheckButton::Create( L"Limit to 4 chars" );
	m_LimitCheckButton->SetId( "limit_check" );

	sfg::Entry::Ptr password( sfg::Entry::Create() );
	password->HideText( '*' );

	// Layout.
	sfg::Box::Ptr boxtoolbar( sfg::Box::Create( sfg::Box::HORIZONTAL ) );
	boxtoolbar->SetSpacing( 5.f );
	boxtoolbar->Pack( btnAddButtonH, false );
	boxtoolbar->Pack( btnAddButtonV, false );
	boxtoolbar->Pack( m_TitleBarToggleButton, false );
	boxtoolbar->Pack( btnHideWindow, false );
	boxtoolbar->Pack( m_Entry, true );
	boxtoolbar->Pack( m_LimitCheckButton, false );

	sfg::Frame::Ptr frame1( sfg::Frame::Create( L"Toolbar 1" ) );
	frame1->Add( boxtoolbar );

	sfg::Box::Ptr boxtoolbar2( sfg::Box::Create( sfg::Box::HORIZONTAL ) );
	boxtoolbar2->SetSpacing( 5.f );
	boxtoolbar2->Pack( btnToggleSpace, false );
	boxtoolbar2->Pack( btnLoadStyle, false );

	m_BoxButtonsH = sfg::Box::Create( sfg::Box::HORIZONTAL );
	m_BoxButtonsH->SetSpacing( 5.f );

	m_BoxButtonsV = sfg::Box::Create( sfg::Box::VERTICAL );
	m_BoxButtonsV->SetSpacing( 5.f );

	sfg::Entry::Ptr username_entry( sfg::Entry::Create() );
	username_entry->SetMaximumLength( 8 );

	m_ProgressBar = sfg::ProgressBar::Create( sfg::ProgressBar::HORIZONTAL );
	m_ProgressBar->SetRequisition( sf::Vector2f( 0.f, 20.f ) );

	m_ProgressBarVertical = sfg::ProgressBar::Create( sfg::ProgressBar::VERTICAL );
	m_ProgressBarVertical->SetRequisition( sf::Vector2f( 20.f, 0.f ) );

	sfg::Separator::Ptr separatorv( sfg::Separator::Create( sfg::Separator::VERTICAL ) );

	m_Table = sfg::Table::Create();
	m_Table->Attach( sfg::Label::Create( L"Please login using your username and password (span example)." ), sf::Rect<sf::Uint32>( 0, 0, 2, 1 ), sfg::Table::FILL, sfg::Table::FILL | sfg::Table::EXPAND );
	m_Table->Attach( sfg::Label::Create( L"Username:" ), sf::Rect<sf::Uint32>( 0, 1, 1, 1 ), sfg::Table::FILL, sfg::Table::FILL );
	m_Table->Attach( username_entry, sf::Rect<sf::Uint32>( 1, 1, 1, 1 ), sfg::Table::EXPAND | sfg::Table::FILL, sfg::Table::FILL );
	m_Table->Attach( sfg::Label::Create( L"Password:" ), sf::Rect<sf::Uint32>( 0, 2, 1, 1 ), sfg::Table::FILL, sfg::Table::FILL );
	m_Table->Attach( password, sf::Rect<sf::Uint32>( 1, 2, 1, 1 ), sfg::Table::FILL, sfg::Table::FILL );
	m_Table->Attach( sfg::Button::Create( L"Login" ), sf::Rect<sf::Uint32>( 2, 1, 1, 2 ), sfg::Table::FILL, sfg::Table::FILL );
	m_Table->Attach( separatorv, sf::Rect<sf::Uint32>( 3, 0, 1, 3 ), sfg::Table::FILL, sfg::Table::FILL );
	m_Table->Attach( m_ProgressBarVertical, sf::Rect<sf::Uint32>( 4, 0, 1, 3 ), sfg::Table::FILL, sfg::Table::FILL );
	m_Table->SetRowSpacings( 5.f );
	m_Table->SetColumnSpacings( 5.f );

	m_ScrolledWindowBox = sfg::Box::Create( sfg::Box::VERTICAL );

	for( int i = 0; i < 5; i++ ) {
		sfg::Box::Ptr box = sfg::Box::Create( sfg::Box::HORIZONTAL );

		for( int j = 0; j < 20; j++ ) {
			box->Pack( sfg::Button::Create( L"One button among many" ), true );
		}

		m_ScrolledWindowBox->Pack( box, false );
	}

	m_ScrolledWindow = sfg::ScrolledWindow::Create();
	m_ScrolledWindow->SetRequisition( sf::Vector2f( .0f, 160.f ) );
	m_ScrolledWindow->SetScrollbarPolicy( sfg::ScrolledWindow::HORIZONTAL_AUTOMATIC | sfg::ScrolledWindow::VERTICAL_AUTOMATIC );
	m_ScrolledWindow->SetPlacement( sfg::ScrolledWindow::TOP_LEFT );
	m_ScrolledWindow->AddWithViewport( m_ScrolledWindowBox );

	sfg::Scrollbar::Ptr scrollbar( sfg::Scrollbar::Create() );
	scrollbar->SetRange( .0f, 100.f );

	m_Scale = sfg::Scale::Create();
	m_Scale->SetAdjustment( scrollbar->GetAdjustment() );
	m_Scale->SetRequisition( sf::Vector2f( 100.f, .0f ) );
	boxtoolbar2->Pack( m_Scale, false );

	m_ComboBox = sfg::ComboBox::Create();
	m_ComboBox->AppendItem( "Item 0" );
	m_ComboBox->AppendItem( "Item 1" );
	m_ComboBox->AppendItem( "Item 2" );
	m_ComboBox->AppendItem( "Item 3" );
	boxtoolbar2->Pack( m_ComboBox, true );

	sfg::Frame::Ptr frame2( sfg::Frame::Create( L"Toolbar 2" ) );
	frame2->Add( boxtoolbar2 );
	frame2->SetAlignment( sf::Vector2f( .8f, .0f ) );

	sfg::Separator::Ptr separatorh( sfg::Separator::Create( sfg::Separator::HORIZONTAL ) );

	sfg::Box::Ptr box_image( sfg::Box::Create( sfg::Box::HORIZONTAL ) );
	box_image->SetSpacing( 15.f );

	sfg::Fixed::Ptr fixed_container( sfg::Fixed::Create() );
	sfg::Button::Ptr fixed_button( sfg::Button::Create( L"I'm at (34,61)" ) );
	fixed_container->Put( fixed_button, sf::Vector2f( 34.f, 61.f ) );
	box_image->Pack( fixed_container, false );

	sf::Image sfgui_logo;
	sfg::Image::Ptr image = sfg::Image::Create();

	if( sfgui_logo.loadFromFile( "data/sfgui.png" ) ) {
		image->SetImage( sfgui_logo );
		box_image->Pack( image, false );
	}

	sfg::Box::Ptr spinner_box( sfg::Box::Create( sfg::Box::VERTICAL ) );

	m_Spinner = sfg::Spinner::Create();
	m_Spinner->SetRequisition( sf::Vector2f( 40.f, 40.f ) );
	m_Spinner->Start();
	sfg::ToggleButton::Ptr spinner_toggle( sfg::ToggleButton::Create( L"Spin") );
	spinner_toggle->SetActive( true );
	spinner_box->SetSpacing( 5.f );
	spinner_box->Pack( m_Spinner, false );
	spinner_box->Pack( spinner_toggle, false );

	box_image->Pack( spinner_box, false );

	sfg::Box::Ptr radio_box( sfg::Box::Create( sfg::Box::VERTICAL ) );

	sfg::RadioButton::Ptr radio1( sfg::RadioButton::Create( "Radio 1" ) );
	sfg::RadioButton::Ptr radio2( sfg::RadioButton::Create( "Radio 2", radio1->GetGroup() ) );
	sfg::RadioButton::Ptr radio3( sfg::RadioButton::Create( "Radio 3", radio2->GetGroup() ) );

	radio_box->Pack( radio1 );
	radio_box->Pack( radio2 );
	radio_box->Pack( radio3 );

	box_image->Pack( radio_box, false );

	sfg::ComboBox::Ptr aligned_combo_box( sfg::ComboBox::Create() );
	aligned_combo_box->AppendItem( L"I'm way over here" );
	aligned_combo_box->AppendItem( L"Me too" );
	aligned_combo_box->AppendItem( L"Me three" );
	aligned_combo_box->SelectItem( 0 );

	sfg::Alignment::Ptr alignment( sfg::Alignment::Create() );
	alignment->Add( aligned_combo_box );
	box_image->Pack( alignment, true );
	alignment->SetAlignment( sf::Vector2f( 1.f, .5f ) );
	alignment->SetScale( sf::Vector2f( 0.f, .01f ) );

	sfg::Box::Ptr boxmain( sfg::Box::Create( sfg::Box::VERTICAL ) );
	boxmain->SetSpacing( 5.f );
	boxmain->Pack( scrollbar, false );
	boxmain->Pack( m_ProgressBar, false );
	boxmain->Pack( frame1, false );
	boxmain->Pack( frame2, false );
	boxmain->Pack( m_BoxButtonsH, false );
	boxmain->Pack( m_BoxButtonsV, false );
	boxmain->Pack( box_image, true );
	boxmain->Pack( separatorh, false );
	boxmain->Pack( m_Table, true );
	boxmain->Pack( m_ScrolledWindow );

	sfg::Notebook::Ptr notebook1( sfg::Notebook::Create() );
	sfg::Notebook::Ptr notebook2( sfg::Notebook::Create() );
	sfg::Notebook::Ptr notebook3( sfg::Notebook::Create() );
	sfg::Notebook::Ptr notebook4( sfg::Notebook::Create() );

	notebook1->SetTabPosition( sfg::Notebook::TOP );
	notebook2->SetTabPosition( sfg::Notebook::RIGHT );
	notebook3->SetTabPosition( sfg::Notebook::BOTTOM );
	notebook4->SetTabPosition( sfg::Notebook::LEFT );

	sfg::Box::Ptr vertigo_box( sfg::Box::Create( sfg::Box::HORIZONTAL ) );
	sfg::Button::Ptr vertigo_button( sfg::Button::Create( L"Vertigo" ) );
	vertigo_box->Pack( vertigo_button, true, true );

	notebook1->AppendPage( boxmain, sfg::Label::Create( "Page Name Here" ) );
	notebook1->AppendPage( notebook2, sfg::Label::Create( "Another Page" ) );
	notebook2->AppendPage( notebook3, sfg::Label::Create( "Yet Another Page" ) );
	notebook2->AppendPage( sfg::Label::Create( L"" ), sfg::Label::Create( "Dummy Page" ) );
	notebook3->AppendPage( notebook4, sfg::Label::Create( "And Another Page" ) );
	notebook3->AppendPage( sfg::Label::Create( L"" ), sfg::Label::Create( "Dummy Page" ) );
	notebook4->AppendPage( vertigo_box, sfg::Label::Create( "And The Last Page" ) );
	notebook4->AppendPage( sfg::Label::Create( L"" ), sfg::Label::Create( "Dummy Page" ) );

	m_MainWindow->Add( notebook1 );

	// Signals.
	btnAddButtonH->GetSignal( sfg::Widget::OnLeftClick ).Connect( &HelloWorld::OnAddButtonHClicked, this );
	btnAddButtonV->GetSignal( sfg::Widget::OnLeftClick ).Connect( &HelloWorld::OnAddButtonVClicked, this );
	m_TitleBarToggleButton->GetSignal( sfg::Widget::OnLeftClick ).Connect( &HelloWorld::OnToggleTitlebarClicked, this );
	btnHideWindow->GetSignal( sfg::Widget::OnLeftClick ).Connect( &HelloWorld::OnHideWindowClicked, this );
	btnToggleSpace->GetSignal( sfg::Widget::OnLeftClick ).Connect( &HelloWorld::OnToggleSpaceClicked, this );
	m_LimitCheckButton->GetSignal( sfg::ToggleButton::OnToggle ).Connect( &HelloWorld::OnLimitCharsToggled, this );
	btnLoadStyle->GetSignal( sfg::Widget::OnLeftClick ).Connect( &HelloWorld::OnLoadThemeClicked, this );
	m_Scale->GetAdjustment()->GetSignal( sfg::Adjustment::OnChange ).Connect( &HelloWorld::OnAdjustementChanged, this );
	spinner_toggle->GetSignal( sfg::Widget::OnLeftClick ).Connect( &HelloWorld::OnToggleSpinner, this );

	m_MainWindow->SetPosition( sf::Vector2f( 100.f, 100.f ) );

	// Another window
	sfg::Window::Ptr second_window( sfg::Window::Create( sfg::Window::TITLEBAR | sfg::Window::BACKGROUND | sfg::Window::RESIZE ) );
	second_window->SetId( "second_window" );
	second_window->SetTitle( "Resize this window to see ad-hoc wrapping." );
	sfg::Box::Ptr box( sfg::Box::Create( sfg::Box::VERTICAL, 5.f ) );

	sfg::Label::Ptr lipsum = sfg::Label::Create(
		"Nullam ut ante leo. Quisque consequat condimentum pulvinar. "
		"Duis a enim sapien, ut vestibulum est. Vestibulum commodo, orci non gravida. "
		"Aliquam sed pretium lacus. "
		"Nullam placerat mauris vel nulla sagittis pellentesque. "
		"Suspendisse in justo dui.\n"
		"Ut dolor massa, gravida eu facilisis convallis, convallis sed odio.\n"
		"Nunc placerat consequat vehicula."
	);

	lipsum->SetRequisition( sf::Vector2f( 400.f, 0.f ) );
	lipsum->SetLineWrap( true );

	box->Pack( lipsum );
	second_window->Add( box );
	second_window->SetPosition( sf::Vector2f( 10.f, 10.f ) );
	second_window->SetId( "second_window" );
	m_Desktop.Add( second_window );

	// Add window to desktop
	m_Desktop.Add( m_MainWindow );

	// Play around with resource manager.
	sf::Font my_font;
	my_font.loadFromFile( "comic.ttf" );
	m_Desktop.GetEngine().GetResourceManager().AddFont( "custom_font", my_font, false ); // false -> do not manage!

	// Set properties.
	m_Desktop.SetProperty( "Button#close:Normal", "Color", sf::Color::Yellow );
	m_Desktop.SetProperty( "Button#close", "FontName", "comic.ttf" );
	m_Desktop.SetProperty( "Button#close", "FontSize", 15.f );
	m_Desktop.SetProperty( "Window#second_window > Box > Label", "FontName", "custom_font" );
	m_Desktop.SetProperty( "Window#second_window > Box > Label", "FontSize", 18.f );

	m_FPS_Counter = 0;
	m_FPS_Clock.restart();

	sf::Clock clock;
	sf::Clock frame_time_clock;

	sf::Int64 frame_times[5000];
	std::size_t frame_times_index = 0;

    sf::Event event;

    while( m_RenderWindow.isOpen() )
    {
        while( m_RenderWindow.pollEvent( event ) )
        {
            if( event.type == sf::Event::Closed )
            {
                m_RenderWindow.close();
            }
            else if( event.type == sf::Event::Resized )
            {
                m_Desktop.UpdateViewRect( sf::FloatRect( 0.f, 0.f, static_cast<float>( event.size.width ), static_cast<float>( event.size.height ) ) );
            }

            m_Desktop.HandleEvent( event );
        }

        m_RenderWindow.draw( m_BackgroundSprite );

        sf::Uint64 microseconds = clock.getElapsedTime().asMicroseconds();

        if( microseconds > 5000 )
        {
            m_Desktop.Update( static_cast<float>( microseconds ) / 1000000.f );
            clock.restart();
        }

        m_SFGUI.Display( m_RenderWindow );

        m_RenderWindow.display();

        sf::Int64 frameTime = frame_time_clock.getElapsedTime().asMicroseconds();
        frame_time_clock.restart();

        frame_times[ frame_times_index ] = frameTime;
        frame_times_index = ( frame_times_index + 1 ) % 5000;

        if( m_FPS_Clock.getElapsedTime().asMicroseconds() >= 1000000 ) {
			m_FPS_Clock.restart();

			sf::Int64 total_time = 0;

			for( std::size_t index = 0; index < 5000; ++index ) {
				total_time += frame_times[index];
			}

			std::stringstream sstr;
			sstr << "SFGUI test -- FPS: " << m_FPS_Counter << " -- Frame Time (microsecs): min: "
			<< *std::min_element( frame_times, frame_times + 5000 ) << " max: "
			<< *std::max_element( frame_times, frame_times + 5000 ) << " avg: "
			<< static_cast<float>( total_time ) / 5000.f;

			m_RenderWindow.setTitle( sstr.str() );

			m_FPS_Counter = 0;
		}

		++m_FPS_Counter;
    }
}

void HelloWorld::OnAddButtonHClicked() {
	sfg::Button::Ptr button( sfg::Button::Create( L"New ->" ) );

	Ouchy::Ptr ouchy( new Ouchy( button ) );
	Ouchy::m_Ouchies.push_back( ouchy );

	button->GetSignal( sfg::Widget::OnLeftClick ).Connect( &Ouchy::DoOuch, ouchy.get() );

	m_BoxButtonsH->Pack( button, true );
}

void HelloWorld::OnAddButtonVClicked() {
	sfg::Button::Ptr button( sfg::Button::Create( L"<- New" ) );

	Ouchy::Ptr ouchy( new Ouchy( button ) );
	Ouchy::m_Ouchies.push_back( ouchy );

	button->GetSignal( sfg::Widget::OnLeftClick ).Connect( &Ouchy::DoOuch, ouchy.get() );

	m_BoxButtonsV->Pack( button, false );
}

void HelloWorld::OnToggleTitlebarClicked() {
	m_MainWindow->SetStyle( m_MainWindow->GetStyle() ^ sfg::Window::TITLEBAR );
}

void HelloWorld::OnHideWindowClicked() {
	m_MainWindow->Show( !m_MainWindow->IsLocallyVisible() );
}

void HelloWorld::OnToggleSpaceClicked() {
	if( m_ScrolledWindowBox->GetSpacing() > .0f ) {
		m_ScrolledWindowBox->SetSpacing( .0f );
	}
	else {
		m_ScrolledWindowBox->SetSpacing( 40.f );
	}
}

void HelloWorld::OnLimitCharsToggled() {
	if( m_LimitCheckButton->IsActive() ) {
		m_Entry->SetMaximumLength( 4 );
	}
	else {
		m_Entry->SetMaximumLength( 0 );
	}
}

void HelloWorld::OnLoadThemeClicked() {
	m_Desktop.LoadThemeFromFile( "example.theme" );
}

void HelloWorld::OnAdjustementChanged() {
	m_ProgressBar->SetFraction( m_Scale->GetValue() / 100.f );
	m_ProgressBarVertical->SetFraction( m_Scale->GetValue() / 100.f );
}

void HelloWorld::OnToggleSpinner() {
	if( !m_Spinner->Started() ) {
		m_Spinner->Start();
	}
	else {
		m_Spinner->Stop();
	}
}
*/

int main_old(int ac, char* av[])
{
	#ifndef LINUX
		#ifdef _DEBUG
			//_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
			//_CrtSetBreakAlloc(16477);
		#endif
	#endif

	cConfig cfg;
	bool running = true;
	bool mouseDown = false;

	g_LogFile.ss()
		<< "Startup: checking for variant config: argc = " << ac << ".\n";
	if(ac > 1) {
		g_LogFile.ss() << "	attempting to load '" << av[1] << "'\n";
		cfg.reload(av[1]);
	}

	CLog log = log;
	log.write("calling init");
	// INit the program
	if(!Init())
		return 1;

	g_WinManager.Push(MainMenu, &g_MainMenu);

	while(running)
	{
        while(SDL_PollEvent(&vent))
		{
			if(vent.type == SDL_QUIT)
				running = false;
			else if(vent.type == SDL_MOUSEBUTTONUP)
			{
				if(mouseDown == true)
				{
					if (g_DragScrollBar != 0)
					{
						g_DragScrollBar->SetTopValue(g_DragScrollBar->m_ItemTop);
						g_DragScrollBar = 0;
					}
					else if(g_DragSlider != 0)
					{
						g_DragSlider->EndDrag();
						g_DragSlider = 0;
					}
					else if(g_MessageBox.IsActive())
						g_MessageBox.Advance();
					else if(g_ChoiceManager.IsActive())
						g_ChoiceManager.ButtonClicked(vent.motion.x, vent.motion.y);
					else
						g_WinManager.UpdateMouseClick(vent.motion.x, vent.motion.y);
					mouseDown = false;
				}
			}
			else if(vent.type == SDL_MOUSEBUTTONDOWN)
			{
				if(vent.button.button == SDL_BUTTON_WHEELDOWN)
				{
					g_WinManager.UpdateMouseClick(vent.motion.x, vent.motion.y, true, false);
				}
				else if(vent.button.button == SDL_BUTTON_WHEELUP)
				{
					g_WinManager.UpdateMouseClick(vent.motion.x, vent.motion.y, false, true);
				}
				else if(vent.button.button == SDL_BUTTON_LEFT)
				{
					//srand(SDL_GetTicks());
					if(mouseDown == false)
						mouseDown = true;
					g_WinManager.UpdateMouseDown(vent.motion.x, vent.motion.y);
				}
/*
 *				horizontal mouse scroll events happen here,
 *				as do right and middle clicks.
 */
				else {
					// do nothing ...
				}
			}
			else if(vent.type == SDL_KEYUP)
			{
				if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
				{
					if(vent.key.keysym.sym == SDLK_RSHIFT || vent.key.keysym.sym == SDLK_LSHIFT)	// enable multi select
						g_ShiftDown = false;
					else if(vent.key.keysym.sym == SDLK_RCTRL || vent.key.keysym.sym == SDLK_LCTRL)	// enable multi select
						g_CTRLDown = false;

					if(vent.key.keysym.sym == SDLK_UP)
						g_UpArrow = false;
					else if(vent.key.keysym.sym == SDLK_DOWN)
						g_DownArrow = false;
					else if(vent.key.keysym.sym == SDLK_LEFT)
						g_LeftArrow = false;
					else if(vent.key.keysym.sym == SDLK_RIGHT)
						g_RightArrow = false;
				}
			}
			else if(vent.type == SDL_KEYDOWN)
			{
				if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
				{
					if(g_WinManager.HasEditBox())
					{
						if(vent.key.keysym.sym == SDLK_BACKSPACE)
							g_WinManager.UpdateKeyInput('-');
						else if(vent.key.keysym.sym == SDLK_RETURN)
							g_EnterKey = true;
						else if((vent.key.keysym.sym >= 97 && vent.key.keysym.sym <= 122) || vent.key.keysym.sym == 39 || vent.key.keysym.sym == 32 || (vent.key.keysym.sym >= 48 && vent.key.keysym.sym <= 57) || ((vent.key.keysym.sym >= 256 && vent.key.keysym.sym <= 265)))
						{
							if(vent.key.keysym.sym >= 256)
							{
								if(vent.key.keysym.sym == 256)
									vent.key.keysym.sym = SDLK_0;
								else if(vent.key.keysym.sym == 257)
									vent.key.keysym.sym = SDLK_1;
								else if(vent.key.keysym.sym == 258)
									vent.key.keysym.sym = SDLK_2;
								else if(vent.key.keysym.sym == 259)
									vent.key.keysym.sym = SDLK_3;
								else if(vent.key.keysym.sym == 260)
									vent.key.keysym.sym = SDLK_4;
								else if(vent.key.keysym.sym == 261)
									vent.key.keysym.sym = SDLK_5;
								else if(vent.key.keysym.sym == 262)
									vent.key.keysym.sym = SDLK_6;
								else if(vent.key.keysym.sym == 263)
									vent.key.keysym.sym = SDLK_7;
								else if(vent.key.keysym.sym == 264)
									vent.key.keysym.sym = SDLK_8;
								else if(vent.key.keysym.sym == 265)
									vent.key.keysym.sym = SDLK_9;
							}

							if(vent.key.keysym.mod & KMOD_LSHIFT || vent.key.keysym.mod & KMOD_RSHIFT || vent.key.keysym.mod & KMOD_CAPS)
								g_WinManager.UpdateKeyInput((char)vent.key.keysym.sym, true);
							else
								g_WinManager.UpdateKeyInput((char)vent.key.keysym.sym);
						}
					}
					else	// hotkeys
					{
						handle_hotkeys();
					}
				}
			}
			else if(vent.type == SDL_MOUSEMOTION)
			{
				if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
				{
					// if dragging a scrollbar, send movements to it exclusively until mouseup
					if (g_DragScrollBar != 0)
						g_DragScrollBar->DragMove(vent.motion.y);
					// if dragging a slider, send movements to it exclusively until mouseup
					else if(g_DragSlider != 0)
						g_DragSlider->DragMove(vent.motion.x);
					// update interface
					else
						g_WinManager.UpdateMouseMovement(vent.motion.x, vent.motion.y);
				}
				else
					g_ChoiceManager.IsOver(vent.motion.x, vent.motion.y);
			}
		}

		//		if(!sleeping)
//		{
			// Clear the screen
			g_Graphics.Begin();

			// draw the background image
			SDL_Rect clip;
			clip.x = 0;
			clip.y = 0;
			clip.w = g_ScreenWidth;
			clip.h = g_ScreenHeight;
			g_BackgroundImage->DrawSurface(clip.x,clip.y,0,&clip,true);

			// Draw the interface
			g_WinManager.Draw();

			if(!g_MessageBox.IsActive() && g_MessageQue.HasNext())
				g_MessageQue.ActivateNext();

			if(eventrunning && !g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())	// run any events that are being run
				GameEvents();

			// Run the interface
			if(!g_MessageBox.IsActive() && !g_ChoiceManager.IsActive())
				g_WinManager.UpdateCurrent();

			// Draw Any message boxes
			if(g_MessageBox.IsActive())
			{
				g_MessageBox.Draw();
			}
			else if(g_ChoiceManager.IsActive() && !g_MessageQue.HasNext())
				g_ChoiceManager.Draw();

			rmanager.CullOld(g_Graphics.GetTicks());

			g_Graphics.End();
/*		}
		else
			SDL_Delay(1000);
*/	}

	Shutdown();
	return 0;
}

void Shutdown()
{
	g_LogFile.write("Shutting Down");
	g_Graphics.Free();

	delete g_BackgroundImage;

	for(int i=0; i<6; i++)
	{
		if(g_BrothelImages[i])
		{
			delete g_BrothelImages[i];
			g_BrothelImages[i] = 0;
		}
	}

	for(int i=0; i<8; i++)
	{
		if(MarketSlaveGirls[i] && MarketSlaveGirlsDel[i] == -1)
			delete MarketSlaveGirls[i];
		MarketSlaveGirls[i] = 0;
	}

	g_Brothels.Free();
	g_Customers.Free();
	g_Girls.Free();
	g_Traits.Free();
	g_InvManager.Free();

	FreeInterface();

	rmanager.Free();
	#ifdef _DEBUG
	cJobManager::freeJobs();
	#else
	cJobManager::free();
	#endif
}

bool Init()
{
	g_LogFile.write("Initializing Graphics");
/*
 *	build the caption string
 */
	stringstream ss;
	ss << "Whore Master v"
	   << g_MajorVersion
	   << "."
	   << g_MinorVersionA
	   << g_MinorVersionB
	   << "."
	   << g_StableVersion
	   << " BETA"
	   << " Svn: " << svn_revision
	;
/*
 *	init the graphics, with the caption on the titlebar
 */
	if(!g_Graphics.InitGraphics(ss.str(), 0,0,32))
	{
		g_LogFile.write("ERROR - Initializing Graphics");
		return false;
	}

	g_LogFile.write("Graphics Initialized");
	// Load the universal background image
	g_BackgroundImage = new ImageSurface("background", "");
	g_LogFile.write("Background Image Set");

	LoadInterface();	// Load the interface
	g_LogFile.write("Interface Loaded");

	InitGameFlags();	// Init the game flags
	g_LogFile.write("Game Flags Initialized");

	// Load the brothel images
	for(int i=0; i<6; i++)
	{
		if(g_BrothelImages[i])
		{
			delete g_BrothelImages[i];
			g_BrothelImages[i] = 0;
		}
/*
 *		I think this should work - kept the old line below
 *		reference
 */
		char buffer[32];
		g_BrothelImages[i] = new ImageSurface("Brothel", toString(i).c_str());
		//g_BrothelImages[i]->LoadImage(file,false);
	}
	g_LogFile.write("Brothel Images Set");

	return true;
}

// trivial change to test Revision.h



#include "WhoreMasterRenewalWindow.hpp"

using namespace WhoreMasterRenewal;

int main( int argc, char* argv[] )
{
    try
    {
        std::clog << "main()\n";
        
        WhoreMasterRenewalWindow window;
        
        std::clog << "main() : pre window.Run()\n";
        
        window.Run();
        
        std::clog << "main() : post window.Run()\n";
    }
    catch( exception ex )
    {
        std::clog << "Exception caught:\n" << ex.what();
    }
    return 0;
}
