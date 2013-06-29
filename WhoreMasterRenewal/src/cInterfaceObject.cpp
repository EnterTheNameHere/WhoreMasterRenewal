
#include "cInterfaceObject.h"

namespace WhoreMasterRenewal
{

cInterfaceObject::cInterfaceObject()
{
    ;
}

cInterfaceObject::~cInterfaceObject()
{
    ;
}

void cInterfaceObject::SetPosition( int x, int y, int width, int height )
{
    m_XPos = x;
    m_YPos = y;
    m_Width = width;
    m_Height = height;
}

void cInterfaceObject::Draw()
{
    
}

int cInterfaceObject::GetXPos()
{
    return m_XPos;
}

int cInterfaceObject::GetYPos()
{
    return m_YPos;
}

int cInterfaceObject::GetWidth()
{
    return m_Width;
}

int cInterfaceObject::GetHeight()
{
    return m_Height;
}

} // namespace WhoreMasterRenewal
