#include "InterfaceWindowXML.hpp"
#include "XmlMisc.h"
#include "CLog.h"
#include "cXmlWidget.h"
#include "DirPath.h"

namespace WhoreMasterRenewal
{

cInterfaceWindowXML::cInterfaceWindowXML() : cInterfaceWindow()
{
    ;
}

void cInterfaceWindowXML::load()
{
    TiXmlDocument doc( m_filename );
    
    if( !doc.LoadFile() )
    {
        g_LogFile.ss()
                << "cInterfaceWindowXML: "
                << "Can't load screen definition from '"
                << m_filename << "'" << std::endl
                ;
        g_LogFile.ss()
                << "Error: line "
                << doc.ErrorRow()
                << ", col "
                << doc.ErrorCol()
                << ": "
                << doc.ErrorDesc()
                << std::endl
                ;
        g_LogFile.ssend();
        return;
    }
    
    /*
     *  get the docuement root
     */
    TiXmlElement* el, *root_el = doc.RootElement();
    
    /*
     *  loop over the elements attached to the root
     */
    for( el = root_el->FirstChildElement();
            el ;
            el = el->NextSiblingElement()
       )
    {
        std::string tag = el->ValueStr();
        /*
         *      now, depending on the tag name...
         */
        
        if( tag == "Define" )
        {
            define_widget( el );
            continue;
        }
        
        if( tag == "Widget" )
        {
            place_widget( el );
            continue;
        }
        
        if( tag == "Window" )
        {
            read_window_definition( el );
            continue;
        }
        
        if( tag == "Text" )
        {
            read_text_item( el );
            continue;
        }
        
        if( tag == "Button" )
        {
            read_button_definition( el );
            continue;
        }
        
        if( tag == "Image" )
        {
            read_image_definition( el );
            continue;
        }
        
        if( tag == "ListBox" )
        {
            read_listbox_definition( el );
            continue;
        }
        
        if( tag == "Checkbox" )
        {
            read_checkbox_definition( el );
            continue;
        }
        
        if( tag == "Slider" )
        {
            read_slider_definition( el );
            continue;
        }
        
        g_LogFile.ss() << "Error: unexpected tag in '" << m_filename << "': '" << tag << "' ..." ;
        g_LogFile.ssend();
    }
}

void cInterfaceWindowXML::read_text_item( TiXmlElement* el )
{
    std::string name, text;
    XmlUtil xu( m_filename );
    int id, x, y, w, h, font_size;
    bool auto_scrollbar = true, force_scrollbar = false;
    
    xu.get_att( el, "Name",  name );
    xu.get_att( el, "Text",  text );
    xu.get_att( el, "XPos",  x );
    xu.get_att( el, "YPos",  y );
    xu.get_att( el, "Width", w );
    xu.get_att( el, "Height",    h );
    xu.get_att( el, "FontSize",  font_size );
    xu.get_att( el, "AutoScrollbar", auto_scrollbar, Optional ); // automatically use scrollbar if text doesn't fit; otherwise, don't
    xu.get_att( el, "ForceScrollbar", force_scrollbar, Optional );  // force scrollbar display even when not needed (shown disabled, grayed out)
    /*
     *  create the text item
     */
    AddTextItem( id, x, y, w, h, text, font_size, auto_scrollbar, force_scrollbar );
    /*
     *  make a note of the ID
     */
    register_id( id, name );
}

void cInterfaceWindowXML::define_widget( TiXmlElement* base_el )
{
    TiXmlElement* el;
    std::string widget_name;
    XmlUtil xu( m_filename );
    
    /*
     *  first get the widget name
     */
    if( !xu.get_att( base_el, "Widget", widget_name ) )
    {
        g_LogFile.ss()  << "Error in " << m_filename << ": "
                        << "'Define' tag with no 'Widget' attribute"
                        ;
        g_LogFile.ssend();
        return;
    }
    
    /*
     *  then create a new widget for that name
     */
    cXmlWidget* widget = new_widget( widget_name );
    
    /*
     *  this is like reading the file again, in minature
     */
    for( el = base_el->FirstChildElement();
            el ;
            el = el->NextSiblingElement()
       )
    {
        std::string tag = el->ValueStr();
        
        g_LogFile.ss() << "define widget: '" << tag << "'";
        g_LogFile.ssend();
        
        if( tag == "Text" )
        {
            widget_text_item( el, *widget );
            continue;
        }
        
        if( tag == "Button" )
        {
            widget_button_item( el, *widget );
            continue;
        }
        
        if( tag == "Image" )
        {
            widget_image_item( el, *widget );
            continue;
        }
        
        if( tag == "ListBox" )
        {
            widget_listbox_item( el, *widget );
            continue;
        }
        
        if( tag == "Checkbox" )
        {
            widget_checkbox_item( el, *widget );
            continue;
        }
        
        if( tag == "Widget" )
        {
            widget_widget( el, *widget );
            continue;
        }
        
        if( tag == "Slider" )
        {
            widget_slider_item( el, *widget );
            continue;
        }
        
        g_LogFile.ss()  << "Warning: Unhandled widget tag: '"
                        << tag
                        << "'"
                        ;
        g_LogFile.ssend();
    }
}

void cInterfaceWindowXML::place_widget( TiXmlElement* el, std::string /*suffix*/ )
{
    int x, y;
    std::string seq, name;
    bool cache;
    XmlUtil xu( m_filename );
    
    /*
     *  we need the base co-ords for the widget and the
     *  sequence number to generate new names
     */
    if( xu.get_att( el, "Definition", name ) == 0 )
    {
        g_LogFile.ss()  << "Error in " << m_filename << ": "
                        << "'Widget' tag found with no 'Definition' attribute"
                        ;
        g_LogFile.ssend();
        return;
    }
    
    g_LogFile.ss()  << "Placing Widget '" << name << "'";
    g_LogFile.ssend();
    
    xu.get_att( el, "XPos",  x );
    xu.get_att( el, "YPos",  y );
    xu.get_att( el, "Seq",   seq );
    xu.get_att( el, "Cache", cache, true );
    add_widget( name, x, y, seq );
}

void cInterfaceWindowXML::add_widget( std::string widget_name, int x, int y, std::string seq )
{
    int id;
    
    cXmlWidget* widget = find_widget( widget_name );
    
    if( widget == nullptr )
    {
        g_LogFile.ss()  << "Error: can't find definition for widget '"
                        << widget_name
                        << "'"
                        ;
        g_LogFile.ssend();
        return;
    }
    
    /*
     *  now loop over the widget components
     */
    for( int i = 0; i < widget->size(); i++ )
    {
        sXmlWidgetPart& xw = ( *widget )[i];
        std::string tag = xw.type;
        std::string name = xw.name + seq;
        /*
         *      the OO way to do this is to subclass
         *      sXmlWidgetPart and have each class have its own
         *      add() routine.
         *
         *      but then I need to store a vector of pointers
         *      because the vector class creates a new, empty instance
         *      and copies in the data, meaning that subclass information
         *      gets discarded for instance vectors.
         *
         *      and storing pointers means needing to delete them
         *      but without getting them deleted on each copy ...
         *
         *      all told it's less fuss to use a big IF, really
         */
        //g_LogFile.ss() << "TAG = '" << tag << "'";
        //g_LogFile.ssend();
        
        g_LogFile.ss()  << "add_widget: x = "
                        << x
                        << ", xw.x = "
                        << xw.x
                        << ", y = "
                        << y
                        << ", xw.y = "
                        << xw.y
                        ;
        g_LogFile.ssend();
        
        int full_x = x + xw.x;
        int full_y = y + xw.y;
        
        if( tag == "Button" )
        {
            g_LogFile.ss() << "adding button: " << xw.off << std::endl;
            g_LogFile.ssend();
            AddButton(
                ButtonPath( xw.off ),
                ButtonPath( xw.disabled ),
                ButtonPath( xw.on ),
                id,
                full_x, full_y, xw.w, xw.h,
                xw.alpha,
                xw.scale,
                xw.cache
            );
            register_id( id, name );
            HideButton( id, xw.hide );
        }
        else if( tag == "Image" )
        {
            g_LogFile.ss() << "adding image: " << xw.file << std::endl;
            g_LogFile.ssend();
            DirPath dp = ImagePath( xw.file );
            AddImage( id, dp,
                      full_x, full_y, xw.w, xw.h, xw.stat,
                      xw.r, xw.g, xw.b
                    );
            register_id( id, name );
            HideImage( id, xw.hide );
        }
        else if( tag == "Slider" )
        {
            AddSlider( id,
                       full_x, full_y, xw.w, xw.r, xw.g, xw.h, xw.b, xw.events
                     );
            register_id( id, name );
            HideSlider( id, xw.hide );
            DisableSlider( id, xw.stat );
        }
        else if( tag == "Text" )
        {
            AddTextItem( id,
                         full_x, full_y, xw.w, xw.h,
                         xw.text, xw.fontsize,
                         xw.alpha, xw.hide
                       );
            register_id( id, name );
            HideText( id, xw.hide );
        }
        else if( tag == "Checkbox" )
        {
            AddCheckbox( id,
                         full_x, full_y, xw.w, xw.h,
                         xw.text, xw.fontsize
                       );
            register_id( id, name );
        }
        else if( tag == "Widget" )
        {
            g_LogFile.ss()  << "Placing nested widget at "
                            << full_x
                            << ", "
                            << full_y
                            ;
            g_LogFile.ssend();
            add_widget( xw.name, full_x, full_y, xw.seq + seq );
        }
        else
        {
            g_LogFile.ss()
                    << "Error: unepected tag in widget '"
                    << tag << ": "
                    << "'."
                    ;
            g_LogFile.ssend();
        }
    }
}

void cInterfaceWindowXML::read_window_definition( TiXmlElement* el )
{
    XmlUtil xu( m_filename );
    int x, y, w, h, border_size;
    xu.get_att( el, "XPos",  x );
    xu.get_att( el, "YPos",  y );
    xu.get_att( el, "Width", w );
    xu.get_att( el, "Height",    h );
    xu.get_att( el, "Border",    border_size );
    CreateWindow( x, y, w, h, border_size );
}

void cInterfaceWindowXML::read_listbox_definition( TiXmlElement* el )
{
    std::string name;
    XmlUtil xu( m_filename );
    bool events = true, multi = false, show_headers = false, header_div = true, header_sort = true;
    int id, x, y, w, h, border_size = 1;
    
    xu.get_att( el, "Name",  name );
    xu.get_att( el, "XPos",  x );
    xu.get_att( el, "YPos",  y );
    xu.get_att( el, "Width", w );
    xu.get_att( el, "Height",    h );
    xu.get_att( el, "Border",    border_size,    Optional );
    xu.get_att( el, "Events",    events,     Optional );
    xu.get_att( el, "Multi", multi,  Optional );                // Multi-select
    xu.get_att( el, "ShowHeaders", show_headers, Optional );    // Show Headers, for multi-column listboxes
    xu.get_att( el, "HeaderDiv", header_div, Optional );        // Show dividers between headers
    xu.get_att( el, "HeaderClicksSort", header_sort, Optional ); // Sort list when user clicks on column header?
    
    AddListBox( id, x, y, w, h, border_size, events, multi, show_headers, header_div, header_sort );
    register_id( id, name );
    
    // Check for column definitions
    TiXmlElement* sub_el;
    int column_count = 0, column_offset[LISTBOX_COLUMNS];
    std::string column_name[LISTBOX_COLUMNS], column_header[LISTBOX_COLUMNS];
    bool column_skip[LISTBOX_COLUMNS];
    
    for( sub_el = el->FirstChildElement();
            sub_el;
            sub_el = sub_el->NextSiblingElement()
       )
    {
        std::string tag = sub_el->ValueStr();
        
        g_LogFile.ss() << "define listbox element: '" << tag << "'";
        g_LogFile.ssend();
        
        // XML definition can arrange columns in any order and even leave some columns out if desired
        // Listbox itself (via DefineColumns) keeps track of what order they're to be displayed in based on this
        if( tag == "Column" )  // hey, we've got a column definition in this listbox
        {
            column_offset[column_count] = 0;
            column_skip[column_count] = false;
            xu.get_att( sub_el, "Name", column_name[column_count] ); // Careful, Name is how we ID columns
            xu.get_att( sub_el, "Header", column_header[column_count], Optional ); // Text header of column
            xu.get_att( sub_el, "Offset", column_offset[column_count], Optional ); // X Offset for column position
            xu.get_att( sub_el, "Skip", column_skip[column_count], Optional ); // hide column? not really used
            column_count++;
        }
        else
        {
            g_LogFile.ss()  << "Warning: Unhandled listbox element: '" << tag << "'";
            g_LogFile.ssend();
        }
    }
    
    // If we have columns defined, go ahead and give the listbox all the gory details
    if( column_count > 0 )
        DefineColumns( id, column_name, column_header, column_offset, column_skip, column_count );
}

void cInterfaceWindowXML::widget_listbox_item( TiXmlElement* el, cXmlWidget& wid )
{
    sXmlWidgetPart xw;
    XmlUtil xu( m_filename );
    
    xw.type = "Text";
    xu.get_att( el, "Name",  xw.name );
    xu.get_att( el, "XPos",  xw.x );
    xu.get_att( el, "YPos",  xw.y );
    xu.get_att( el, "Width", xw.w );
    xu.get_att( el, "Height",    xw.h );
    xu.get_att( el, "Border",    xw.bordersize,  Optional );
    xu.get_att( el, "Multi", xw.multi,   Optional );
    xu.get_att( el, "Events",    xw.events,  Optional );
    wid.add( xw );
}

void cInterfaceWindowXML::read_checkbox_definition( TiXmlElement* el )
{
    std::string name, text;
    XmlUtil xu( m_filename );
    int id, x, y, w, h, font_size;
    
    xu.get_att( el, "Name",  name );
    xu.get_att( el, "Text",  text );
    xu.get_att( el, "XPos",  x );
    xu.get_att( el, "YPos",  y );
    xu.get_att( el, "Width", w );
    xu.get_att( el, "Height",    h );
    xu.get_att( el, "FontSize",  font_size );
    
    AddCheckbox( id, x, y, w, h, text, font_size );
    
    register_id( id, name );
}

void cInterfaceWindowXML::widget_checkbox_item( TiXmlElement* el, cXmlWidget& wid )
{
    sXmlWidgetPart xw;
    xw.type = "Checkbox";
    xw.hide = false;
    XmlUtil xu( m_filename );
    
    xu.get_att( el, "Name",  xw.name );
    xu.get_att( el, "Text",  xw.text );
    xu.get_att( el, "XPos",  xw.x );
    xu.get_att( el, "YPos",  xw.y );
    xu.get_att( el, "Width", xw.w );
    xu.get_att( el, "Height",    xw.h );
    xu.get_att( el, "FontSize",  xw.fontsize );
    wid.add( xw );
}

void cInterfaceWindowXML::widget_widget( TiXmlElement* el, cXmlWidget& wid )
{
    sXmlWidgetPart xw;
    XmlUtil xu( m_filename );
    
    xw.type = "Widget";
    xu.get_att( el, "Definition",    xw.name );
    xu.get_att( el, "XPos",  xw.x );
    xu.get_att( el, "YPos",  xw.y );
    xu.get_att( el, "Seq",   xw.seq );
    xu.get_att( el, "cache", xw.cache, true );
    wid.add( xw );
}

void cInterfaceWindowXML::read_image_definition( TiXmlElement* el )
{
    XmlUtil xu( m_filename );
    bool stat = false, hide = false;
    int id, x, y, w, h;
    int r = 0, g = 0, b = 0;
    std::string name, file = "blank.png";
    
    xu.get_att( el, "Name",  name );
    xu.get_att( el, "File",  file, Optional );
    xu.get_att( el, "XPos",  x );
    xu.get_att( el, "YPos",  y );
    xu.get_att( el, "Width", w );
    xu.get_att( el, "Height",    h );
    xu.get_att( el, "Stat",  stat, Optional );
    xu.get_att( el, "Red",   r, Optional );
    xu.get_att( el, "Green", g, Optional );
    xu.get_att( el, "Blue",  b, Optional );
    xu.get_att( el, "Hidden",    hide, Optional );
    
    DirPath dp = ImagePath( file );
    
    AddImage( id, dp, x, y, w, h, stat, r, g, b );
    HideImage( id, hide );
    register_id( id, name );
}

void cInterfaceWindowXML::read_button_definition( TiXmlElement* el )
{
    int id, x, y, w, h;
    XmlUtil xu( m_filename );
    std::string scale, alpha, name, on, off, disabled, base = "";
    /*
     *  get the button name - we'll use this to match up
     *  interface IDs
     */
    xu.get_att( el, "Name",  name );
    /*
     *  "Image" is the base string for the button.
     *  If we have "BackOn.png" "BackOff.png" and
     *  "BackDisabled.png"
     *
     *  then the base string would be "Back"
     */
    xu.get_att( el, "Image", base, Optional );
    
    /*
     *  if we have a base value, use it to construct the
     *  three image names
     */
    if( base != "" )
    {
        on      = base + "On.png";
        off     = base + "Off.png";
        disabled    = base + "Disabled.png";
    }
    
    /*
     *  On, Off and Disabled override the base derived
     *  value. You don't need base at all - just specify
     *  all three buttons directly.
     */
    xu.get_att( el, "On",    on, Optional );
    xu.get_att( el, "Off",   off, Optional );
    xu.get_att( el, "Disabled",  disabled, Optional );
    /*
     *  xywh
     */
    xu.get_att( el, "XPos",  x );
    xu.get_att( el, "YPos",  y );
    xu.get_att( el, "Width", w );
    xu.get_att( el, "Height",    h );
    /*
     *  I assume these are bools for "use alpha channel"
     *  and "scale with window"
     */
    xu.get_att( el, "Transparency",  alpha );
    xu.get_att( el, "Scale",     scale );
    /*
     *  and finally...
     */
    AddButton(
        ButtonPath( off ),
        ButtonPath( disabled ),
        ButtonPath( on ),
        id,
        x, y, w, h,
        alpha == "true",
        scale == "true"
    );
    register_id( id, name );
}

void cInterfaceWindowXML::read_slider_definition( TiXmlElement* el )
{
    XmlUtil xu( m_filename );
    bool disable = false, hide = false, liveUpdate = true;
    int id, x, y, w;
    int min = 0, max = 100, value = 0, increment = 5;
    std::string name;
    
    xu.get_att( el, "Name",  name );
    xu.get_att( el, "XPos",  x );
    xu.get_att( el, "YPos",  y );
    xu.get_att( el, "Width", w );
    xu.get_att( el, "Increment", increment, Optional );
    xu.get_att( el, "MinValue",  min, Optional );
    xu.get_att( el, "MaxValue",  max, Optional );
    xu.get_att( el, "Value", value, Optional );
    xu.get_att( el, "Hidden",    hide, Optional );
    xu.get_att( el, "Disabled",  disable, Optional );
    xu.get_att( el, "LiveUpdate", liveUpdate, Optional );
    
    // for some reason, optional ints above are unfortunately being set to 0 when not present
    if( min == 0 && max == 0 )
        max = 100;
        
    if( value < min )
        value = min;
        
    if( value > max )
        value = max;
        
    if( increment == 0 )
        increment = 5;
        
    AddSlider( id, x, y, w, min, max, increment, value, liveUpdate );
    DisableSlider( id, disable );
    HideSlider( id, hide );
    register_id( id, name );
}

void cInterfaceWindowXML::widget_slider_item( TiXmlElement* el, cXmlWidget& wid )
{
    sXmlWidgetPart xw;
    XmlUtil xu( m_filename );
    
    xw.type = "Slider";
    xw.hide = xw.stat = false;
    xw.events = true;
    xw.r = 0;
    xw.g = 100;
    xw.b = 0;
    xw.h = 5;
    
    xu.get_att( el, "Name",  xw.name );
    xu.get_att( el, "XPos",  xw.x );
    xu.get_att( el, "YPos",  xw.y );
    xu.get_att( el, "Width", xw.w );
    xu.get_att( el, "Increment", xw.h, Optional );
    xu.get_att( el, "MinValue",  xw.r, Optional );
    xu.get_att( el, "MaxValue",  xw.g, Optional );
    xu.get_att( el, "Value", xw.b, Optional );
    xu.get_att( el, "Hidden",    xw.hide, Optional );
    xu.get_att( el, "Disabled",  xw.stat, Optional );
    xu.get_att( el, "LiveUpdate", xw.events, Optional );
    
    // for some reason, optional ints above are unfortunately being set to 0 when not present; correct for that
    if( xw.r == 0 && xw.g == 0 )
        xw.g = 100;
        
    if( xw.b < xw.r )
        xw.b = xw.r;
        
    if( xw.b > xw.g )
        xw.b = xw.g;
        
    if( xw.h == 0 )
        xw.h = 5;
        
    wid.add( xw );
}

void cInterfaceWindowXML::widget_text_item( TiXmlElement* el, cXmlWidget& wid )
{
    sXmlWidgetPart xw;
    xw.type = "Text";
    xw.hide = false;
    XmlUtil xu( m_filename );
    
    xu.get_att( el, "Name",  xw.name );
    xu.get_att( el, "Text",  xw.text );
    xu.get_att( el, "XPos",  xw.x );
    xu.get_att( el, "YPos",  xw.y );
    xu.get_att( el, "Width", xw.w );
    xu.get_att( el, "Height",    xw.h );
    xu.get_att( el, "FontSize",  xw.fontsize );
    xu.get_att( el, "Hidden",    xw.hide, Optional );
    xu.get_att( el, "AutoScrollbar", xw.alpha, Optional );
    xu.get_att( el, "ForceScrollbar", xw.hide, Optional );
    
    wid.add( xw );
}

void cInterfaceWindowXML::widget_button_item( TiXmlElement* el, cXmlWidget& wid )
{
    sXmlWidgetPart xw;
    XmlUtil xu( m_filename );
    
    xw.type = "Button";
    xw.hide = false;
    xw.base = "";
    
    xu.get_att( el, "Name",  xw.name );
    xu.get_att( el, "Image", xw.base, Optional );
    
    if( xw.base != "" )
    {
        xw.on       = xw.base + "On.png";
        xw.off      = xw.base + "Off.png";
        xw.disabled = xw.base + "Disabled.png";
    }
    
    xu.get_att( el, "On",    xw.on, Optional );
    xu.get_att( el, "Off",   xw.off, Optional );
    xu.get_att( el, "Disabled",  xw.disabled, Optional );
    xu.get_att( el, "XPos",  xw.x );
    xu.get_att( el, "YPos",  xw.y );
    xu.get_att( el, "Width", xw.w );
    xu.get_att( el, "Height",    xw.h );
    xu.get_att( el, "Transparency",  xw.alpha );
    xu.get_att( el, "Scale",     xw.scale );
    xu.get_att( el, "Hidden",    xw.hide, Optional );
    xu.get_att( el, "cache", xw.cache, Optional );
    wid.add( xw );
}

void cInterfaceWindowXML::widget_image_item( TiXmlElement* el, cXmlWidget& wid )
{
    sXmlWidgetPart xw;
    XmlUtil xu( m_filename );
    
    xw.type = "Image";
    xw.hide = false;
    xw.stat = false;
    xw.r = xw.g = xw.b = 0;
    
    xu.get_att( el, "Name",  xw.name );
    xu.get_att( el, "File",  xw.file );
    xu.get_att( el, "XPos",  xw.x );
    xu.get_att( el, "YPos",  xw.y );
    xu.get_att( el, "Width", xw.w );
    xu.get_att( el, "Height",    xw.h );
    xu.get_att( el, "Red",   xw.r, Optional );
    xu.get_att( el, "Green", xw.g, Optional );
    xu.get_att( el, "Blue",  xw.b, Optional );
    xu.get_att( el, "Stat",  xw.stat, Optional );
    xu.get_att( el, "Hidden",    xw.hide, Optional );
    wid.add( xw );
}

void cInterfaceWindowXML::register_id( int id, std::string name )
{
    g_LogFile.ss()  << "registering ID "
                    << id
                    << " to name '"
                    << name
                    << "'"
                    ;
    g_LogFile.ssend();
    m_name_to_id[name] = id;
    m_id_to_name[id]     = name;
}

int cInterfaceWindowXML::get_id( std::string s, bool essential )
{
    if( m_name_to_id.find( s ) != m_name_to_id.end() )
    {
        return m_name_to_id[s];
    }
    
    if( !essential )
    {
        return -1;
    }
    
    g_LogFile.ss()
            << "Error: no ID for interface name '"
            << s
            << "'."
            ;
    g_LogFile.ssend();
    return -1;
}

cXmlWidget* cInterfaceWindowXML::new_widget( std::string name )
{
    cXmlWidget* wid = new cXmlWidget();
    
    m_widgets[name] = wid;
    return wid;
}

cXmlWidget* cInterfaceWindowXML::find_widget( std::string name )
{
    std::map<std::string, cXmlWidget*>::iterator it;
    
    it = m_widgets.find( name );
    
    if( it == m_widgets.end() )
    {
        return nullptr;
    }
    
    return it->second;
}

} // namespace WhoreMasterRenewal
