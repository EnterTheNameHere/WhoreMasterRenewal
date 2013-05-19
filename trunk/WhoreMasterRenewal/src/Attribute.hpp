#ifndef ATTRIBUTE_HPP_INCLUDED_1455
#define ATTRIBUTE_HPP_INCLUDED_1455

#include <string>
#include <map>
#include <pugixml.hpp>
#include "CLog.h"

namespace WhoreMasterRenewal
{

enum AttributeType : int
{
    BOOLEAN,
    INT,
    DOUBLE,
    STRING
};

class Attribute
{
    public:
        Attribute( std::string name, AttributeType type );
        virtual ~Attribute();
        Attribute( const Attribute& other );
        Attribute& operator=( const Attribute& other );
        std::string GetName() { return m_Name; }
        void SetName( std::string val ) { m_Name = val; }
        AttributeType GetType() { return m_Type; }
        void SetType( AttributeType val ) { m_Type = val; }
        
    private:
        std::string m_Name;
        AttributeType m_Type;
};

class AttributeXMLLoader
{
    static std::map<std::string, Attribute> LoadAttributesFromFile( std::string filePath )
    {
        pugi::xml_document doc;
        if( doc.load_file( filePath.c_str() ).status != pugi::xml_parse_status::status_ok )
            Logger() << "Error: cannot load attributes from \"" << filePath << "\".\n";
        // TODO: Proper error handling
    }
};

}

#endif // ATTRIBUTE_HPP_INCLUDED_1455
