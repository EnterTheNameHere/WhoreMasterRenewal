#include "Attribute.hpp"

namespace WhoreMasterRenewal
{

Attribute::Attribute( std::string name, AttributeType type ) :
    m_Name( name ), m_Type( type )
{
    //ctor
}

Attribute::~Attribute()
{
    //dtor
}

Attribute::Attribute( const Attribute& other ) :
    m_Name( other.m_Name ), m_Type( other.m_Type )
{
    //copy ctor
}

Attribute& Attribute::operator=(const Attribute& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator
    return *this;
}

}
