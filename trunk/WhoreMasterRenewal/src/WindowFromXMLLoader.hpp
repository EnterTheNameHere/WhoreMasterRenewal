#ifndef WINDOWFROMXMLLOADER_HPP_INCLUDED_2158
#define WINDOWFROMXMLLOADER_HPP_INCLUDED_2158
#pragma once

#include <algorithm>
#include <string>

#include "ResizableImage.hpp"

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
        
        /**
        * Searches for one child inside the node and calls ProcessElement for found node.
        * Issues a warning if more than one or none children are found.
        * 
        * @param node The node where to search for the child
        * @return Shared pointer to the child widget instance
        *
        **/
        sfg::Widget::Ptr TryAddChild( const pugi::xml_node& node )
        {
            auto numberOfNodes = std::distance( node.children().begin(), node.children().end() );
            if( numberOfNodes == 0 )
            {
                std::stringstream ss;
                ss << "Error: No child found inside <" << node.name() << "> tag.\n";
                ss << "Path to the node: \"" << node.path() << "\".";
                
                Logger() << ss.str() << "\n";
                return sfg::Label::Create( ss.str() );
            }
            else if( numberOfNodes > 1 )
            {
                std::stringstream ss;
                ss << "Warning: More than one child found inside <" << node.name() << "> tag. "
                    << "Only one child is expected.\n";
                ss << "Only the first child <" << node.first_child().name() << "> will be processed, rest will be ignored.\n";
                ss << "Path to the node: \"" << node.path() << "\".";
                
                Logger() << ss.str() << "\n";
            }
            
            return ProcessElement( node.first_child() );
        }
        
        void ProcessCommonAttributes( const pugi::xml_node& node, sfg::Widget::Ptr widget )
        {
            float minWidth = node.attribute( "minWidth" ).as_float( 0.0 );
            float minHeight = node.attribute( "minHeight" ).as_float( 0.0 );
            
            Logger() << "Setting requisition: [" << minHeight << ";" << minWidth << "]\n";
            widget->SetRequisition( sf::Vector2f( minWidth, minHeight ) );
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
            else if( name == "Label" )
                return this->ProcessLabel( node );
            else if( name == "Image" )
                return this->ProcessImage( node );
            else if( name == "ResizableImage" )
                return this->ProcessResizableImage( node );
            else if( name == "Table" )
                return this->ProcessTable( node );
            else if( name == "Frame" )
                return this->ProcessFrame( node );
            else if( name == "Notepad" )
                return this->ProcessNotebook( node );
            else
            {
                std::stringstream text;
                text << "Unknown node \"" << node.name() << "\"";
                Logger() << text.str() << "\n";
                return sfg::Label::Create( text.str() );
            }
        }
        
        sfg::Notebook::Ptr ProcessNotebook( const pugi::xml_node& node )
        {
            Logger() << "Starting parsing of Notebook\n";
            sfg::Notebook::Ptr notebook = sfg::Notebook::Create();
            
            // Check for Pages
            {
                for( pugi::xml_node& subNode: node.children() )
                {
                    string name = subNode.name();
                    if( name == "Page" )
                    {
                        string pageName = subNode.attribute( "name" ).as_string();
                        
                        notebook->AppendPage( this->TryAddChild( subNode ), sfg::Label::Create( pageName ) );
                    }
                    else
                    {
                        std::stringstream ss;
                        ss << "Error: unknown <" << name << "> tag found inside <Notebook>.\n";
                        ss << "Didn't You forget to put widget inside <Notebook><Page> </Page></Notebook> tags?";
                        
                        Logger() << ss.str() << "\n";
                        notebook->AppendPage( sfg::Label::Create( ss.str() ), sfg::Label::Create( "Unknown" ) );
                    }
                }
            }
            
            this->ProcessCommonAttributes( node, notebook );
            
            Logger() << "Finished parsing of Notebook\n";
            return notebook;
        }
        
        sfg::Frame::Ptr ProcessFrame( const pugi::xml_node& node )
        {
            Logger() << "Starting parsing of Frame\n";
            
            sfg::Frame::Ptr frame = sfg::Frame::Create();
            frame->Add( this->TryAddChild( node ) );
            
            this->ProcessCommonAttributes( node, frame );
            
            Logger() << "Finished parsing of Frame\n";
            return frame;
        }
        
        sfg::Table::Ptr ProcessTable( const pugi::xml_node& node )
        {
            Logger() << "Starting parsing of Table\n";
            
            sfg::Table::Ptr newTable = sfg::Table::Create();
            
            // Process Attach-ments, warn if other tags found
            for( pugi::xml_node attachNode: node.children() )
            {
                string name = attachNode.name();
                if( name == "Attach" )
                {
                    // Check for attributes
                    int rowNumber = attachNode.attribute("row").as_int( 0 );
                    int columnNumber = attachNode.attribute("column").as_int( 0 );
                    int rowSpan = attachNode.attribute("rowSpan").as_int( 1 );        // How many column spaces it should occupy
                    int columnSpan = attachNode.attribute("columnSpan").as_int( 1 );  // How many row spaces it should occupy
                    
                    // TODO: x: EXPAND|FILL
                    //       y: EXPAND|FILL
                    //       padding
                    
                    auto numberOfNodes = std::distance( attachNode.children().begin(), attachNode.children().end() );
                    if( numberOfNodes != 1 )
                    {
                        Logger() << "Warning, None or more than one tags found inside <Table><Attach> </Attach></Table> tag.\n";
                        newTable->Attach( sfg::Label::Create( L"Warning: None of more than one tags found inside <Attach>." ), sf::Rect<sf::Uint32>( columnNumber, rowNumber, columnSpan, rowSpan ) );
                    }
                    else
                    {
                        Logger() << "Adding element to Table: row=" << rowNumber << " column=" << columnNumber << " rowSpan=" << rowSpan << " columnSpan=" << columnSpan << " \n";
                        newTable->Attach( this->ProcessElement( attachNode.first_child() ), sf::Rect<sf::Uint32>( columnNumber, rowNumber, columnSpan, rowSpan ) );
                    }
                }
                else
                {
                    Logger() << "Warning, <Table></Table> contains unsupported tag <" << attachNode.name() << " />. Didn't You want to put it inside <Attach></Attach> tags?\n";
                    newTable->Attach( sfg::Label::Create( "Error: Unsupported tag found." ), sf::Rect<sf::Uint32>( 0, 0, 1, 1 ) );
                }
            }
            
            return newTable;
        }
        
        sfg::ResizableImage::Ptr ProcessResizableImage( const pugi::xml_node& node )
        {
            Logger() << "Starting parsing of ResizableImage\n";
            
            bool pathEmpty = node.attribute("path").empty();
            
            sf::Image sfImage;
            
            if( !pathEmpty )
            {
                string imagePath = node.attribute("path").as_string();
                sfImage.loadFromFile( imagePath );
            }
            else
            {
                Logger() << "Warning, resizable image path not specified\n";
            }
            
            sfg::ResizableImage::Ptr sfgResizableImage = sfg::ResizableImage::Create( sfImage );
            
            // Should we keep aspect
            bool keepAspect = node.attribute("keepAspect").as_bool( true );
            sfgResizableImage->SetKeepAspect( keepAspect );
            
            // Get requested size of image
            unsigned int x = node.attribute("setWidth").as_uint();
            unsigned int y = node.attribute("setHeight").as_uint();
            
            // And resize the image
            sfgResizableImage->Resize( sf::Vector2u( x, y ) );
            
            Logger() << "Finished parsing of ResizableImage\n";
            return sfgResizableImage;
        }
        
        sfg::Image::Ptr ProcessImage( const pugi::xml_node& node )
        {
            Logger() << "Starting parsing of Image\n";
            
            bool pathEmpty = node.attribute("path").empty();
            
            sf::Image sfImage;
            
            if( !pathEmpty )
            {
                string imagePath = node.attribute("path").as_string();
                sfImage.loadFromFile( imagePath );
            }
            else
            {
                Logger() << "Warning, image path not specified\n";
            }
            
            Logger() << "Finished parsing of Image\n";
            sfg::Image::Ptr sfgImage = sfg::Image::Create( sfImage );
            
            return sfgImage;
        }
                
        sfg::Label::Ptr ProcessLabel( const pugi::xml_node& node )
        {
            Logger() << "Starting parsing of Label\n";
            
            string text = node.text().as_string();
            
            Logger() << "Creating Label:\n";
            Logger() << "text=\"" << text << "\"\n";
            
            Logger() << "Finished parsing of Label\n";
            return sfg::Label::Create( text );
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
            for( pugi::xml_node packNode: node.children() )
            {
                string name = packNode.name();
                if( name == "Pack" )
                {
                    bool expand = packNode.attribute( "expand" ).as_bool( true );
                    bool fill = packNode.attribute( "fill" ).as_bool( true );
                    
                    Logger() << "Adding element to Box. expand='" << expand << "' fill='" << fill << "'\n";
                    
                    box->Pack( this->ProcessElement( packNode.first_child() ), expand, fill );
                }
                else
                {
                    Logger() << "Warning: Unsupported tag found inside <Box></Box>. Didn't You forget to put widget inside <Pack></Pack>?\n";
                    box->Pack( sfg::Label::Create( L"Unsupported tag inside Box. Didn't You forget to put widget inside <Pack></Pack>?" ) );
                }
            }
            
            ProcessCommonAttributes( node, box );
            
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

            alignment->Add( this->TryAddChild( node ) );
            
            ProcessCommonAttributes( node, alignment );
            
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
