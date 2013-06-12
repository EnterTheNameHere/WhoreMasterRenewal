
#include "CResource.h"
#include "CGraphics.h"

namespace WhoreMasterRenewal
{

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
    m_Next = nullptr;
    m_Prev = nullptr;
    m_TimeUsed = g_Graphics.GetTicks();
}

CResource::~CResource()
{
    Free();
    m_Next = nullptr;
    m_Prev = nullptr;
}

} // namespace WhoreMasterRenewal
