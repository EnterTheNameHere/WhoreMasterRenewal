
#include "CResource.h"
#include "CGraphics.h"

// registers the resource with the resource manager
void CResource::Register()
{
    
}

// Free all data
void CResource::Free()
{
    
}

// Frees only the loaded data, this is so the class isn't destroyed
void CResource::FreeResources()
{
    
}

CResource::CResource()
{
    m_Next = 0;
    m_Prev = 0;
    m_TimeUsed = g_Graphics.GetTicks();
}

CResource::~CResource()
{
    Free();
    m_Next = 0;
    m_Prev = 0;
}
