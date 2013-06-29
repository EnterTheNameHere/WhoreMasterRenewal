#ifndef INTERFACEWINDOWXML_HPP_INCLUDED_0317
#define INTERFACEWINDOWXML_HPP_INCLUDED_0317

#include "cInterfaceWindow.h" // required inheritance

#include <string>
#include <map>

class TiXmlElement;

namespace WhoreMasterRenewal
{

class cXmlWidget;

class cInterfaceWindowXML : public cInterfaceWindow
{
protected:
    std::string m_filename = "";
    std::map<std::string, int> m_name_to_id = {};
    std::map<int, std::string> m_id_to_name = {};
    
    std::map<std::string, cXmlWidget*> m_widgets = {};
    
public:
    enum AttributeNecessity
    {
        Mandatory = 0,
        Optional = 1
    };
    
    cInterfaceWindowXML();
    virtual ~cInterfaceWindowXML() {};
    
    void load();
    /*
     *  this has static linkage so we can pass it
     *  to the window manager's Push method
     */
    static void handler_func( cInterfaceWindowXML* wpt );
    /*
     *  the handler func just calls the virtual process
     *  method, which can process calls as it likes
     */
    virtual void process() = 0;
    /*
     *  populates the maps so we can get the IDs from strings
     */
    void register_id( int id, std::string name );
    /*
     *  XML reading stuff
     */
    void read_text_item( TiXmlElement* );
    void read_window_definition( TiXmlElement* );
    void read_button_definition( TiXmlElement* );
    void read_image_definition( TiXmlElement* );
    void read_listbox_definition( TiXmlElement* );
    void read_checkbox_definition( TiXmlElement* );
    void read_slider_definition( TiXmlElement* );
    void define_widget( TiXmlElement* );
    void place_widget( TiXmlElement*, std::string suffix = "" );
    void widget_text_item( TiXmlElement*, cXmlWidget& );
    void widget_button_item( TiXmlElement*, cXmlWidget& );
    void widget_listbox_item( TiXmlElement*, cXmlWidget& );
    void widget_checkbox_item( TiXmlElement*, cXmlWidget& );
    void widget_widget( TiXmlElement*, cXmlWidget& );
    void widget_image_item( TiXmlElement*, cXmlWidget& );
    void widget_slider_item( TiXmlElement*, cXmlWidget& );
    int get_id( std::string name, bool essential = false );
    
    cXmlWidget* new_widget( std::string name );
    cXmlWidget* find_widget( std::string name );
    void add_widget( std::string widget_name, int x, int y, std::string seq );
};

} // namespace WhoreMasterRenewal

#endif // INTERFACEWINDOWXML_HPP_INCLUDED_0317
