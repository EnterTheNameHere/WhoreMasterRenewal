
#include "WindowManager.hpp"
#include "Constants.h"
#include "libRocketSFMLInterface/RenderInterfaceSFML.h"
#include "libRocketSFMLInterface/ShellFileInterface.h"
#include "libRocketSFMLInterface/SystemInterfaceSFML.h"

#include <SFML/Graphics.hpp>
#include <Rocket/Core.h>
#include <Rocket/Controls.h>
#include <Rocket/Debugger/Debugger.h>
#include <Rocket/Controls/DataSource.h>
#include <Rocket/Core/Types.h>

#include <sstream>
#include <iostream>

/// @todo What if m_Context is nullptr?

namespace WhoreMasterRenewal
{

//{ EventHandling

class EventHandler
{
public:
    EventHandler() = default;
    virtual ~EventHandler()
    {}
    
    virtual void ProcessEvent( Rocket::Core::Event& event, const Rocket::Core::String& value ) = 0;
};

typedef std::map< Rocket::Core::String, EventHandler* > EventHandlerMap;
EventHandlerMap eventHandlers;

static EventHandler* currentEventHandler = nullptr;
static std::string currentWindow = "";

class EventManager
{
public:
    EventManager() = delete;
    ~EventManager() = delete;
    
    static void Shutdown()
    {
        Logger() << "EventManager::Shutdown\n";
        
        for( EventHandlerMap::iterator it = eventHandlers.begin(); it != eventHandlers.end(); ++it )
        {
            Logger() << "Deleting event handler \"" << (*it).first.CString() << "\".\n";
            delete (*it).second;
        }
        
        eventHandlers.clear();
        currentEventHandler = nullptr;
        currentWindow = "";
        
        Logger() << "EventManager successfuly shut down.\n";
    }
    
    static void RegisterEventHandler( const Rocket::Core::String& handlerName, EventHandler* handler )
    {
        Logger() << "EventManager::RegisterEventHandler (" << handlerName.CString() << ")\n";
        
        EventHandlerMap::iterator it = eventHandlers.find( handlerName );
        if( it != eventHandlers.end() )
        {
            Logger() << "Deleting event handler \"" << (*it).first.CString() << "\"\n";
            delete (*it).second;
        }
        
        eventHandlers[handlerName] = handler;
    }
    
    static void ProcessEvent( Rocket::Core::Event& event, const Rocket::Core::String& value )
    {
        Logger() << "EventManager::ProcessEvent (" << value.CString() << ")\n";
        //Logger() << Helper::DumpRocketEvent( event );
        
        Rocket::Core::StringList commands;
        Rocket::Core::StringUtilities::ExpandString( commands, value, ';' );
        
        for( size_t i = 0; i < commands.size(); i++ )
        {
            Rocket::Core::StringList values;
            Rocket::Core::StringUtilities::ExpandString( values, commands[i], ' ' );
            
            if( values.empty() )
                return;
            
            if( values[0] == "goto" && values.size() > 1 )
            {
                if( LoadWindow( values[1] ) )
                    event.GetTargetElement()->GetOwnerDocument()->Close();
            }
            else if( values[0] == "load" && values.size() > 1 )
            {
                LoadWindow( values[1] );
            }
            else if( values[0] == "close" )
            {
                Rocket::Core::ElementDocument* targetDocument = nullptr;
                
                //if( values.size() > 1 )
                    //targetDocument = m_Context->GetDocument( values[1].CString() );
                //else
                    //targetDocument = event.GetTargetElement()->GetOwnerDocument();
                
                if( targetDocument != nullptr )
                    targetDocument->Close();
            }
            else if( values[0] == "unimplemented" )
            {
                Rocket::Core::Log::Message( Rocket::Core::Log::Type::LT_WARNING, "Unimplemented" );
            }
            else if( values[0] == "quitGame" )
            {
                // TODO Need access to WindowManager
            }
            else
            {
                if( currentEventHandler != nullptr )
                    currentEventHandler->ProcessEvent( event, commands[i] );
            }
        }
    }
    
    static bool LoadWindow( const Rocket::Core::String& windowName )
    {
        Logger() << "EventManager::LoadWindow (" << windowName.CString() << ")\n";
        
        EventHandler* oldEventHandler = currentEventHandler;
        EventHandlerMap::iterator it = eventHandlers.find( windowName );
        if( it != eventHandlers.end() )
            currentEventHandler = (*it).second;
        else
            currentEventHandler = nullptr;
        
        /*if( resizeEvent == nullptr )
            resizeEvent = new ResizeEvent();*/
        
        //Rocket::Core::String documentPath = Rocket::Core::String("../../WhoreMasterRenewal/Resources/Interface/") + windowName + Rocket::Core::String(".rml");
        //Rocket::Core::ElementDocument* document = m_Context->LoadDocument( documentPath );
        //if( document == nullptr )
        //{
            //currentEventHandler = oldEventHandler;
            //return false;
        //}
        
        /*document->AddEventListener( "resize", resizeEvent );*/
        
        //Rocket::Core::Element* title = document->GetElementById("title");
        //if( title != nullptr )
            //title->SetInnerRML( document->GetTitle() );
        //document->Focus();
        //document->Show();
        
        //Logger() << "Size: [" << document->GetClientHeight() << "," << document->GetClientWidth() << "].\n";
        
        //currentWindow = windowName.CString();
        
        //document->RemoveReference();
        
        //int currentNumDocs = m_Context->GetNumDocuments();
        //std::stringstream sstream;
        //sstream << "Current number of documents: " << currentNumDocs << "\n";
        //for( int i = 0; i < currentNumDocs; i++ )
        //{
            //sstream << "[" << m_Context->GetDocument( i )->GetId().CString() << "] ";
        //}
        //sstream << "\n";
        
        //Logger() << sstream.str().c_str();
        
        return true;
    }
    
    static void ReloadWindow()
    {
        Logger() << "EventManager::ReloadWindow() currentWindow=\"" << currentWindow.c_str() << "\"\n";
        
        //m_Context->GetDocument( currentWindow.c_str() )->Close();
        Rocket::Core::Factory::ClearStyleSheetCache();
        LoadWindow( currentWindow.c_str() );
    }
};

class Event : public Rocket::Core::EventListener
{
public:
    Event( const Rocket::Core::String& value )
        : m_Value( value )
    {
        Logger() << "Event::const (" << value.CString() << ")\n";
    }
    virtual ~Event()
    {}
    
    virtual void ProcessEvent( Rocket::Core::Event& event ) override
    {
        Logger() << "Event::ProcessEvent [m_Value=" << m_Value.CString() << "]\n";
        EventManager::ProcessEvent( event, m_Value );
    }
    
    virtual void OnDetach( Rocket::Core::Element* ) override
    {
        Logger() << "Event::OnDetach [m_Value=" << m_Value.CString() << "]\n";
        delete this;
    }
    
private:
    Rocket::Core::String m_Value;
};

class EventListenerInstancerI : public Rocket::Core::EventListenerInstancer
{
public:
    EventListenerInstancerI() : EventListenerInstancer()
    {
        Logger() << "EventListenerInstancer::const\n";
    }
    virtual ~EventListenerInstancerI()
    {}
    
    virtual Rocket::Core::EventListener* InstanceEventListener( const Rocket::Core::String& value ) override
    {
        Logger() << "EventListenerInstancer::InstanceEventListener (" << value.CString() << ")\n";
        return new Event( value );
    }
    
    virtual void Release() override
    {
        Logger() << "EventListenerInstancer::Release\n";
        delete this;
    }
};

//}





//{ Helpers

template<typename Char, typename CharTraits = std::char_traits<Char> >
std::basic_ostream<Char, CharTraits>& operator << ( std::basic_ostream<Char, CharTraits>& out, const Rocket::Core::StringBase<Char>& rocketString )
{
    out << rocketString.CString();
    return out;
}

template<typename Char>
Logger& operator << ( Logger& out, const Rocket::Core::StringBase<Char>& rocketString )
{
    out << rocketString.CString();
    return out;
}

class Helper
{
public:
    static std::string DumpRocketEvent( Rocket::Core::Event& event )
    {
        std::stringstream ssDebugText;
        
        ssDebugText << "====== RocketEvent Dump: ======\n";
        ssDebugText << "Event Name: \"" << event.GetType() << "\"\n";
        
        ssDebugText << "Target:  <" << event.GetTargetElement()->GetTagName();
        if( !event.GetTargetElement()->GetId().Empty() )
        {
            ssDebugText << " id=\"" << event.GetTargetElement()->GetId() << "\"";
        }
        ssDebugText << ">\n";
        
        ssDebugText << "Current: <" << event.GetCurrentElement()->GetTagName();
        if( !event.GetCurrentElement()->GetId().Empty() )
        {
            ssDebugText << " id=\"" << event.GetCurrentElement()->GetId() << "\"";
        }
        ssDebugText << ">\n";
        
        //ssDebugText << event.GetTargetElement()->GetTagName() <<
        //    "[" << event.GetTargetElement()->GetId() << "]: "
        //    << event.GetType() << "\nParameters: " << event.GetParameters()->Size() << "\n";
        
        Rocket::Core::Variant* val;
        Rocket::Core::String key;
        int pos = 0;
        while( event.GetParameters()->Iterate( pos, key, val ) )
        {
            //ssDebugText << "[" << pos << "]" << key << ": ";
            switch( val->GetType() )
            {
            case Rocket::Core::Variant::Type::BYTE:
                //ssDebugText << val->Get<Rocket::Core::byte>();
                break;
            case Rocket::Core::Variant::Type::CHAR:
                //ssDebugText << val->Get<char>();
                break;
            case Rocket::Core::Variant::Type::COLOURB:
                //ssDebugText << "RGBA=";
                //ssDebugText << val->Get<Rocket::Core::Colourb>().red << ";";
                //ssDebugText << val->Get<Rocket::Core::Colourb>().green << ";";
                //ssDebugText << val->Get<Rocket::Core::Colourb>().blue << ";";
                //ssDebugText << val->Get<Rocket::Core::Colourb>().alpha;
                break;
            case Rocket::Core::Variant::Type::COLOURF:
                //ssDebugText << "RGBA=";
                //ssDebugText << val->Get<Rocket::Core::Colourf>().red << ";";
                //ssDebugText << val->Get<Rocket::Core::Colourf>().green << ";";
                //ssDebugText << val->Get<Rocket::Core::Colourf>().blue << ";";
                //ssDebugText << val->Get<Rocket::Core::Colourf>().alpha;
                break;
            case Rocket::Core::Variant::Type::FLOAT:
                //ssDebugText << val->Get<float>();
                break;
            case Rocket::Core::Variant::Type::INT:
                //ssDebugText << val->Get<int>();
                break;
            case Rocket::Core::Variant::Type::NONE:
                //ssDebugText << "NONE";
                break;
            case Rocket::Core::Variant::Type::SCRIPTINTERFACE:
                //ssDebugText << "SCRIPTINTERFACE";
                break;
            case Rocket::Core::Variant::Type::STRING:
                //ssDebugText << val->Get<Rocket::Core::String>();
                break;
            case Rocket::Core::Variant::Type::VECTOR2:
                //ssDebugText << "VECTOR2"; // TODO:
                break;
            case Rocket::Core::Variant::Type::VOIDPTR:
                //ssDebugText << "VOIDPTR";
                break;
            case Rocket::Core::Variant::Type::WORD:
                //ssDebugText << val->Get<Rocket::Core::word>();
                break;
                
            default:
                //ssDebugText << "Unknown";
                break;
            }
            
            //ssDebugText << " ";
        }
        
        ssDebugText << "===============================\n";
        
        return ssDebugText.str();
    }
};

class DebugGirl
{
public:
    DebugGirl( std::string name, int age, int looks, int health, int mood, int tired, std::string dayJob, std::string nightJob )
        : m_Name( name ), m_Age( age ), m_Looks( looks ), m_Health( health ), m_Mood( mood ), m_Tired( tired ), m_DayJob( dayJob ), m_NightJob( nightJob )
    {}
    virtual ~DebugGirl() = default;
    
    std::string GetName()
    {
        return m_Name;
    }
    void SetName( std::string name )
    {
        m_Name = name;
    }
    
    int GetAge()
    {
        return m_Age;
    }
    void SetAge( int age )
    {
        m_Age = age;
    }
    
    int GetLooks()
    {
        return m_Looks;
    }
    void SetLooks( int looks )
    {
        m_Looks = looks;
    }
    
    int GetHealth()
    {
        return m_Health;
    }
    void SetHealth( int health )
    {
        m_Health = health;
    }
    
    int GetMood()
    {
        return m_Mood;
    }
    void SetMood( int mood )
    {
        m_Mood = mood;
    }
    
    int GetTired()
    {
        return m_Tired;
    }
    void SetTired( int tired )
    {
        m_Tired = tired;
    }
    
    std::string GetDayJob()
    {
        return m_DayJob;
    }
    void SetDayJob( std::string dayJob )
    {
        m_DayJob = dayJob;
    }
    
    std::string GetNightJob()
    {
        return m_NightJob;
    }
    void SetNightJob( std::string nightJob )
    {
        m_NightJob = nightJob;
    }
    
private:
    std::string m_Name = {""};
    int m_Age = {0};
    int m_Looks = {0};
    int m_Health = {0};
    int m_Mood = {0};
    int m_Tired = {0};
    
    std::string m_DayJob = {"Free Time"};
    std::string m_NightJob = {"Free Time"};
};

class Job
{
public:
    Job( std::string jobType, std::string jobName ) :
        m_JobType( jobType ), m_JobName( jobName )
    {}
    
    virtual ~Job()
    {}
    
    std::string GetJobType()
    {
        return m_JobType;
    }
    
    void SetJobType( std::string jobType )
    {
        m_JobType = jobType;
    }
    
    std::string GetJobName()
    {
        return m_JobName;
    }
    
    void SetJobName( std::string jobName )
    {
        m_JobName = jobName;
    }
    
private:
    std::string m_JobType;
    std::string m_JobName;
};

class DebugGirlsList : public Rocket::Controls::DataSource
{
public:
    static void Initialise()
    {
        Logger() << "DebugGirlsList::Initialise().\n";
        
        new DebugGirlsList();
    }
    
    static void Shutdown()
    {
        Logger() << "DebugGirlsList::Shutdown().\n";
        
        delete m_Instance;
    }
    
    virtual void GetRow( Rocket::Core::StringList& row, const Rocket::Core::String& table, int rowIndex, const Rocket::Core::StringList& columns ) override
    {
        Rocket::Core::String columnsList;
        Rocket::Core::StringUtilities::JoinString( columnsList, columns, ',' );
        Logger() << "DebugGirlsList::GetRow()\nTable = \"" << table.CString() << "\", row = " << rowIndex << "\n"
            << "columns = [" << columnsList.CString() << "]\n";
        
        if( table == "girls" )
        {
            for( size_t i = 0; i < columns.size(); i++ )
            {
                if( columns[i] == "name" )
                {
                    row.push_back( Rocket::Core::String( m_Girls[rowIndex].GetName().c_str() ) );
                }
                else if( columns[i] == "age" )
                {
                    row.push_back( Rocket::Core::String( 4, "%d", m_Girls[rowIndex].GetAge() ) );
                }
                else if( columns[i] == "looks" )
                {
                    row.push_back( Rocket::Core::String( 4, "%d", m_Girls[rowIndex].GetLooks() ) );
                }
                else if( columns[i] == "health" )
                {
                    row.push_back( Rocket::Core::String( 4, "%d", m_Girls[rowIndex].GetHealth() ) );
                }
                else if( columns[i] == "mood" )
                {
                    row.push_back( Rocket::Core::String( 4, "%d", m_Girls[rowIndex].GetMood() ) );
                }
                else if( columns[i] == "tired" )
                {
                    row.push_back( Rocket::Core::String( 4, "%d", m_Girls[rowIndex].GetTired() ) );
                }
                else if( columns[i] == "dayjob" )
                {
                    row.push_back( Rocket::Core::String( m_Girls[rowIndex].GetDayJob().c_str() ) );
                }
                else if( columns[i] == "nightjob" )
                {
                    row.push_back( Rocket::Core::String( m_Girls[rowIndex].GetNightJob().c_str() ) );
                }
            }
        }
        else if( table == "jobtypes" || table == "jobnames" )
        {
            for( size_t i = 0; i < columns.size(); i++ )
            {
                if( columns[i] == "jobtype" )
                {
                    row.push_back( Rocket::Core::String( m_JobTypes[rowIndex].c_str() ) );
                }
                else if( columns[i] == "jobname" )
                {
                    row.push_back( Rocket::Core::String( m_JobNames[rowIndex].c_str() ) );
                }
            }
        }
        else if( table == "traits" )
        {
            for( size_t i = 0; i < columns.size(); i++ )
            {
                if( columns[i] == "traitname" )
                {
                    row.push_back( Rocket::Core::String( m_Traits[rowIndex].c_str() ) );
                }
            }
        }
        
        Rocket::Core::String rowValuesList;
        Rocket::Core::StringUtilities::JoinString( rowValuesList, row, ',' );
        
        Logger() << "row values = [" << rowValuesList.CString() << "]\n";
    }
    
    virtual int GetNumRows( const Rocket::Core::String& table ) override
    {
        Logger() << "DebugGirlsList::GetNumRows( " << table.CString() << " ).\n";
        
        if( table == "girls" )
        {
            return m_Girls.size();
        }
        else if( table == "jobtypes" )
        {
            return m_JobTypes.size();
        }
        else if( table == "jobnames" )
        {
            return m_JobNames.size();
        }
        else if( table == "traits" )
        {
            return m_Traits.size();
        }
        else
        {
            return 0;
        }
    }
    
private:
    DebugGirlsList() : Rocket::Controls::DataSource( "DebugGirlsList" )
    {
        Logger() << "DebugGirlsList::const().\n";
        
        m_Instance = this;
    }
    
    virtual ~DebugGirlsList()
    {
        Logger() << "DebugGirlsList::destr().\n";
        
        m_Instance = nullptr;
    }
    
    static DebugGirlsList* m_Instance;
    
    std::vector<DebugGirl> m_Girls = {
        DebugGirl( "Soifon", 20, 10, 100, 100, 21, "Free Time", "Security" ),
        DebugGirl( "Yoruichi Shihouin", 26, 31, 100, 100, 2, "Security", "Explore Catacombs" ),
        DebugGirl( "Sheryl Nome", 17, 72, 100, 100, 0, "Advertising", "Waitress" )
    };
    
    std::vector<Job> m_Jobs = {
        Job( "General", "Free Time" ),
        Job( "General", "Cleaning" ),
        Job( "General", "Security" ),
        Job( "General", "Advertising" ),
        Job( "General", "Customer Service" ),
        Job( "General", "Matron" ),
        Job( "Brothel", "Whore in Brothel" ),
        Job( "Brothel", "Whore on Streets" ),
        Job( "Brothel", "Stripper in Brothel" ),
        Job( "Brothel", "Masseure in Brothel" ),
        Job( "Gambling Hall", "Whore for Gamblers" ),
        Job( "Gambling Hall", "Game Dealer" ),
        Job( "Gambling Hall", "Entertainer" ),
        Job( "Gambling Hall", "XXX Entertainer" ),
        Job( "Bar", "Barmaid" ),
        Job( "Bar", "Waitress" ),
        Job( "Bar", "Stripper in Bar" ),
        Job( "Bar", "Whore in Bar" ),
        Job( "Bar", "Singer" )
    };
    
    std::vector<std::string> m_JobTypes = {
        std::string( "General" ),
        std::string( "Brothel" ),
        std::string( "Gambling Hall" ),
        std::string( "Bar" )
    };
    
    std::vector<std::string> m_JobNames = {
        "Free Time",
        "Cleaning",
        "Security",
        "Advertising",
        "Customer Service",
        "Matron"
    };
    
    std::vector<std::string> m_Traits =
    {
        "Aggresive",
        "Assassin",
        "Basic Magic",
        "Fleet of Foot",
        "Inhuman Lifespan",
        "Iron Will",
        "Lesbian",
        "Merciless",
        "Small Boobs",
        "Strong",
        "Tough",
        "Tsundere",
        "Normal Sex LV0",
        "Anal Sex LV0",
        "Oral Sex LV0",
        "BDSM Sex LV0",
        "Exhibitionist LV0",
        "Lesbian LV0.1",
        "Group Sex LV0"
    };
};

DebugGirlsList* DebugGirlsList::m_Instance = nullptr;

class DebugEvent : public Rocket::Core::EventListener
{
public:
    virtual ~DebugEvent()
    {}
    
    virtual void ProcessEvent( Rocket::Core::Event& event ) override
    {
        Logger() << Helper::DumpRocketEvent( event ).c_str();
    }
};

//}





WindowManager::WindowManager()
        : WindowManager( std::make_shared<sf::RenderWindow>() )
{}
WindowManager::WindowManager( std::shared_ptr<sf::RenderWindow> windowPtr )
    : m_RenderWindow( windowPtr )
{
    Logger() << "WindowManager created\n";
}

WindowManager::~WindowManager()
{
    if( m_RenderWindow )
        m_RenderWindow = nullptr;
    
    Logger() << "WindowManager destructed\n";
}

void WindowManager::SetupWindow()
{
    std::stringstream windowTitleBuilder;
    windowTitleBuilder <<
        "Whore Master: Renewal [" <<
        g_MajorVersion << "," <<
        g_MinorVersion << "," <<
        g_PatchVersion << "," <<
        g_MetadataVersion << "]";
    
    sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();
    //TODO: custom width and height
    m_RenderWindow->create( videoMode, windowTitleBuilder.str() );
}

void WindowManager::Run()
{
    if( !m_RenderWindow->isOpen() )
        SetupWindow();
    
    // If window is not open now, we have a problem
    if( !m_RenderWindow->isOpen() )
    {
        auto currentVideoMode = sf::VideoMode::getDesktopMode();
        Logger() << "Error: cannot create rendering window\n" <<
            "Current desktop video mode: " <<
            currentVideoMode.width << "x" <<
            currentVideoMode.height << ":" <<
            currentVideoMode.bitsPerPixel << "bpp\n" <<
            "Available fullscreen modes:\n";
        
        auto fullscreenVideoModes = sf::VideoMode::getFullscreenModes();
        for( auto videoMode : fullscreenVideoModes )
        {
            Logger() << videoMode.width << "x" <<
                        videoMode.height << ":" <<
                        videoMode.bitsPerPixel << "bpp\n";
        }
        
        return;
    }
    
    // Setup libRocket
    RocketSFMLRenderInterface sfRenderInterface;
    RocketSFMLSystemInterface sfSystemInterface;
    ShellFileInterface fileInterface( "Resources/" );
    
    sfRenderInterface.SetWindow( m_RenderWindow );
    
    Rocket::Core::SetRenderInterface( &sfRenderInterface );
    Rocket::Core::SetFileInterface( &fileInterface );
    Rocket::Core::SetSystemInterface( &sfSystemInterface );
    
    if( !Rocket::Core::Initialise() )
    {
        Logger() << "Error: Cannot initialise libRocket framework.\n";
        return;
    }
    
    Rocket::Controls::Initialise();
    
    m_Context = Rocket::Core::CreateContext(
            "context",
            Rocket::Core::Vector2i( m_RenderWindow->getSize().x, m_RenderWindow->getSize().y )
        );
    
    if( !Rocket::Debugger::Initialise( m_Context ) )
    {
        Logger() << "Warning: Cannot initialise libRocket Debugger.\n";
    }
    
    // TODO load fonts from Resources/Fonts
    /*
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/PT Sans Bold Italic.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/PT Sans Bold.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/PT Sans Italic.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/PT Sans.ttf" );
    
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSans.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSans-Bold.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSans-Oblique.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSans-BoldOblique.ttf" );
    
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSansMono.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSansMono-Bold.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSansMono-Oblique.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSansMono-BoldOblique.ttf" );
    
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSerif.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSerif-Bold.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSerif-BoldItalic.ttf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/DejaVuSerif-Italic.ttf" );
    
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/Delicious-Roman.otf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/Delicious-Bold.otf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/Delicious-BoldItalic.otf" );
    Rocket::Core::FontDatabase::LoadFontFace( "Fonts/Delicious-Italic.otf" );
    */
    
    EventListenerInstancerI* eventListenerInstancer = new EventListenerInstancerI();
    Rocket::Core::Factory::RegisterEventListenerInstancer( eventListenerInstancer );
    eventListenerInstancer->RemoveReference();
    
    m_Context->AddEventListener( "resize", new DebugEvent() );
    
    this->ShowWindow( "MainMenu" );
    
    while( m_RenderWindow->isOpen() )
    {
        sf::Event event;
        
        m_RenderWindow->clear();
        m_Context->Render();
        m_RenderWindow->display();
        
        while( m_RenderWindow->pollEvent( event ) )
        {
            switch( event.type )
            {
            case sf::Event::EventType::Closed:
                m_RenderWindow->close();
                break;
            
            case sf::Event::EventType::Resized:
                Logger() << "Resized Event: " << event.size.width << "x" << event.size.height << "\n";
                sfRenderInterface.Resize();
                m_Context->SetDimensions( Rocket::Core::Vector2i( event.size.width, event.size.height ) );
                Logger() << "Dimensions set...\n";
                //Logger() << "Context size:  " << m_Context->GetDimensions().x << "x" << m_Context->GetDimensions().y << "\n";
                //Logger() << "Window size:   " << m_RenderWindow->getSize().x << "x" << m_RenderWindow->getSize().y << "\n";
                break;
            
            case sf::Event::EventType::MouseMoved:
                m_Context->ProcessMouseMove( event.mouseMove.x, event.mouseMove.y, sfSystemInterface.GetKeyModifiers( event ) );
                break;
                
            case sf::Event::EventType::MouseButtonPressed:
                m_Context->ProcessMouseButtonDown( event.mouseButton.button, sfSystemInterface.GetKeyModifiers( event ) );
                break;
                
            case sf::Event::EventType::MouseButtonReleased:
                m_Context->ProcessMouseButtonUp( event.mouseButton.button, sfSystemInterface.GetKeyModifiers( event ) );
                break;
                
            case sf::Event::EventType::MouseWheelMoved:
                m_Context->ProcessMouseWheel( event.mouseWheel.delta * (-1), sfSystemInterface.GetKeyModifiers( event ) );
                break;
                
            case sf::Event::EventType::TextEntered:
                m_Context->ProcessTextInput( static_cast<Rocket::Core::word>( event.text.unicode ) );
                break;
                
            case sf::Event::EventType::KeyPressed:
                m_Context->ProcessKeyDown( sfSystemInterface.TranslateKey( event.key.code ), sfSystemInterface.GetKeyModifiers( event ) );
                break;
                
            case sf::Event::EventType::KeyReleased:
                if( event.key.code == sf::Keyboard::Key::Tilde )
                    Rocket::Debugger::SetVisible( !Rocket::Debugger::IsVisible() );
                if( event.key.code == sf::Keyboard::Key::R && event.key.control )
                    EventManager::ReloadWindow();
                
                m_Context->ProcessKeyUp( sfSystemInterface.TranslateKey( event.key.code ), sfSystemInterface.GetKeyModifiers( event ) );
                break;
                
            default: // Ignore rest of events
                break;
            }
        }
        
        m_Context->Update();
    }
    
    m_Context->RemoveReference();
    EventManager::Shutdown();
    Rocket::Core::Shutdown();
}

void WindowManager::ShowWindow( const Rocket::Core::String& windowName )
{
    Logger() << "ShowWindow( \"" << windowName << "\")\n";
    
    // TODO: Load path from settings
    Rocket::Core::String documentPath = "../../WhoreMasterRenewal/Resources/Interface/" + windowName + ".rml";
    Rocket::Core::ElementDocument* document = m_Context->LoadDocument( documentPath );
}

} // namespace WhoreMasterRenewal
