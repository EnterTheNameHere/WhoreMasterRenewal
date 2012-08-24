
#ifndef WINDOWFROMXMLLOADER_HPP_INCLUDED_2158
#define WINDOWFROMXMLLOADER_HPP_INCLUDED_2158

#include <algorithm>
#include <string>

#include "pugixml.hpp"

namespace WhoreMasterRenewal
{

    class WindowFromXMLLoader
    {
    public:
        WindowFromXMLLoader()
        {}
        
        ~WindowFromXMLLoader()
        {}
        
        sfg::Window::Ptr LoadFile( const string& filename )
        {
            Logger() << "\n\nWindowFromXMLLoader::LoadFile( \"" << filename << "\" )\n";
            
            m_FileName = filename;
            
            pugi::xml_parse_result result = m_Doc.load_file( m_FileName.c_str() );
            if( !result )
            {
                std::cerr << "WindowFromXMLLoader:\n"
                        << "Error occured during parsing of file \"" << m_FileName << "\"\n"
                        << "Description: " << result.description()
                        << "Status: " << result.status
                        << "Offset: " << result.offset << "\n";
                return sfg::Window::Create();
            }
            
            ProcessXML();
            
            return m_Window;
        }
        
    private:
        sfg::Window::Ptr m_Window;
        pugi::xml_document m_Doc;
        string m_FileName;
        
        void ProcessXML()
        {
            std::cout << "ProcessXML()\n";
            
            auto windowNodes = m_Doc.children("Window");
            auto numberOfWindowNodes = std::distance( windowNodes.begin(), windowNodes.end() );
            if( numberOfWindowNodes == 0 )
            {
                std::cerr << "Warning, no <Window> tag found in \"" << m_FileName << "\".\n";
                m_Window = sfg::Window::Create();
                return;
            }
            else if( numberOfWindowNodes >= 2 )
            {
                std::cerr << "Warning, file \"" << m_FileName << "\" contains more than one <Window> tags. "
                        << "Only the first is loaded.\n";
            }
            else if( numberOfWindowNodes < 0 )
            {
                std::cerr << "WindowFromXMLLoader:\nError during processing of XML in file \""
                        << m_FileName << "\".Difference of Window nodes is lower than 0.\n";
                m_Window = sfg::Window::Create();
                return;
            }
            
            pugi::xml_node windowNode = m_Doc.child("Window");
            
            m_Window = this->ProcessWindow( windowNode );
            
            //windowNode.print( std::cout );
        }
        
        sfg::Widget::Ptr ProcessElement( const pugi::xml_node& node )
        {
            //Logger() << "Processing node \"" << node.name() << "\"\n";
            
            string name = node.name();
            if( name == "Window" )
                return this->ProcessWindow( node );
            else if( name == "Box" )
                return this->ProcessBox( node );
            else if( name == "Alignment" )
                return this->ProcessAlignment( node );
            else if( name == "Button" )
                return this->ProcessButton( node );
            else
            {
                std::stringstream text;
                text << "Unknown node \"" << node.name() << "\"";
                Logger() << text.str() << "\n";
                return sfg::Label::Create( text.str() );
            }
        }
        
        sfg::Window::Ptr ProcessWindow( const pugi::xml_node& node )
        {
            Logger() << "Starting parsing of Window\n";
            
            sfg::Window::Ptr window = sfg::Window::Create( sfg::Window::Style::NO_STYLE );
            
            // Check style
            bool titleBar = node.attribute( "titleBar" ).as_bool( true );
            bool background = node.attribute( "background" ).as_bool( true );
            bool resize = node.attribute( "resize" ).as_bool( true );
            bool shadow = node.attribute( "shadow" ).as_bool( false );
            
            int style = sfg::Window::Style::NO_STYLE;
            if( titleBar )
                style |= sfg::Window::Style::TITLEBAR;
            if( background )
                style |= sfg::Window::Style::BACKGROUND;
            if( resize )
                style |= sfg::Window::Style::RESIZE;
            if( shadow )
                style |= sfg::Window::Style::SHADOW;
            window->SetStyle( style );
            
            // Go through window childrens
            for( pugi::xml_node subNode: node.children() )
            {
                //std::cout << "Processing node \"" << node.name() << "\"\n";
                
                window->Add( this->ProcessElement( subNode ) );
            }
            
            Logger() << "Finished parsing of Window\n";
            
            return window;
        }
        
        sfg::Box::Ptr ProcessBox( const pugi::xml_node& node )
        {
            Logger() << "Starting parsing of Box\n";
            
            // Check orientation
            sfg::Box::Orientation orientation = sfg::Box::Orientation::HORIZONTAL;
            string orientationValue = node.attribute("orientation").as_string();
            std::transform( orientationValue.begin(), orientationValue.end(),
                            orientationValue.begin(), ::tolower );
            if( orientationValue == "vertical" )
                orientation = sfg::Box::Orientation::VERTICAL;
            
            // Check spacing
            float spacing = node.attribute("spacing").as_float( 0.f );
            
            std::cout << "Box:\n\torientation=" << orientation
                    << "\n\tspacing=" << spacing << "\n";
            
            sfg::Box::Ptr box = sfg::Box::Create( orientation, spacing );
            
            // Process children
            for( pugi::xml_node subNode: node.children() )
            {
                box->Pack( this->ProcessElement( subNode ) );
            }
            
            Logger() << "Finished parsing of Box\n";
            
            return box;
        }
        
        sfg::Alignment::Ptr ProcessAlignment( const pugi::xml_node& node )
        {
            Logger() << "Starting parsing of Alignment\n";
            
            sfg::Alignment::Ptr alignment = sfg::Alignment::Create();
            
            // check for scale
            float verticalScale = node.attribute( "verticalScale" ).as_float( 0.f );
            float horizontalScale = node.attribute( "horizontalScale" ).as_float( 0.f );
            
            // check for alignment
            float verticalAlignment = 0.f;
            float horizontalAlignment = 0.f;
            
            string verticalAlignmentStr = node.attribute( "verticalAlignment" ).as_string();
            string horizontalAlignmentStr = node.attribute( "horizontalAlignment" ).as_string();
            
            if( verticalAlignmentStr == "Top" )
                verticalAlignment = 0.f;
            else if( verticalAlignmentStr == "Center" )
                verticalAlignment = 0.5f;
            else if( verticalAlignmentStr == "Bottom" )
                verticalAlignment = 1.f;
            else
                verticalAlignment = node.attribute( "verticalAlignment" ).as_float( 0.f );
            
            if( horizontalAlignmentStr == "Left" )
                horizontalAlignment = 0.f;
            else if( horizontalAlignmentStr == "Center" )
                horizontalAlignment = 0.5f;
            else if( horizontalAlignmentStr == "Right" )
                horizontalAlignment = 1.f;
            else
                horizontalAlignment = node.attribute( "horizontalAlignment" ).as_float( 0.f );
            
            // Set attributes
            Logger() << "Alignment:\nscale=<" << horizontalScale << ";" << verticalScale << ">\n"
                    << "alignment=<" << horizontalAlignment << ";" << verticalAlignment << ">\n";
            
            alignment->SetScale( sf::Vector2f( horizontalScale, verticalScale ) );
            alignment->SetAlignment( sf::Vector2f( horizontalAlignment, verticalAlignment ) );
            
            // Add children
            for( pugi::xml_node subNode: node.children() )
            {
                alignment->Add( this->ProcessElement( subNode ) );
            }
            
            Logger() << "Finished parsing of Alignment\n";
            
            return alignment;
        }
        
        sfg::Button::Ptr ProcessButton( const pugi::xml_node& node )
        {
            Logger() << "Started parsing of Button\n";
            
            sfg::Button::Ptr button = sfg::Button::Create();
            
            // Set label
            button->SetLabel( node.attribute( "label" ).as_string( "" ) );
            
            // Set image
            string imagePath = node.attribute( "image" ).as_string( "" );
            if( imagePath != "" )
            {
                sf::Image image;
                image.loadFromFile( imagePath );
                button->SetImage( sfg::Image::Create( image ) );
            }
            
            Logger() << "Finished parsing of Button\n";
            
            return button;
        }
    }; // class WindowFromXMLLoader

} // namespace WhoreMasterRenewal

#endif // WINDOWFROMXMLLOADER_HPP_INCLUDED_2158
