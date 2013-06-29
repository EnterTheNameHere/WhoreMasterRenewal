
#include "cInterfaceEvent.h"

namespace WhoreMasterRenewal
{

cInterfaceEvent::cInterfaceEvent()
{
    ;
}

cInterfaceEvent::~cInterfaceEvent()
{
    if( m_Next )
        delete m_Next;
    m_Next = nullptr;
}



cInterfaceEventManager::cInterfaceEventManager()
{
    ;
}

cInterfaceEventManager::~cInterfaceEventManager()
{
    if( m_Events )
        delete m_Events;
    m_Events = nullptr;
}

bool cInterfaceEventManager::CheckButton( int ObjectID )
{
    return CheckEvent( EVENT_BUTTONCLICKED, ObjectID );
}

bool cInterfaceEventManager::CheckListbox( int ObjectID )
{
    return CheckEvent( EVENT_SELECTIONCHANGE, ObjectID );
}

bool cInterfaceEventManager::CheckCheckbox( int ObjectID )
{
    return CheckEvent( EVENT_CHECKBOXCLICKED, ObjectID );
}

bool cInterfaceEventManager::CheckSlider( int ObjectID )
{
    return CheckEvent( EVENT_SLIDERCHANGE, ObjectID );
}

bool cInterfaceEventManager::CheckEvent( int EventID, int ObjectID )
{
    if( !m_Events )
        return false;

    cInterfaceEvent* current = m_Events;
    cInterfaceEvent* last = nullptr;
    while( current )
    {
        if( current->m_EventID == EventID && current->m_ObjectID == ObjectID )
        {
            if( last )
            {
                last->m_Next = current->m_Next;
                current->m_Next = nullptr;
                delete current;
                m_NumEvents--;
                return true;
            }
            else
            {
                if( current->m_Next )
                    m_Events = current->m_Next;
                else
                    m_Events = nullptr;

                current->m_Next = nullptr;
                delete current;
                m_NumEvents--;
                return true;
            }
        }
        last = current;
        current = current->m_Next;
    }
    return false;
}

int cInterfaceEventManager::GetNumEvents()
{
    return m_NumEvents;
}

void cInterfaceEventManager::AddEvent( int ID, int Object )
{
    cInterfaceEvent* newEvent = new cInterfaceEvent();
    newEvent->m_EventID = ID;
    newEvent->m_ObjectID = Object;
    newEvent->m_Next = nullptr;

    if( !m_Events )
        m_Events = newEvent;
    else
    {
        cInterfaceEvent* current = m_Events;
        while( current->m_Next )
            current = current->m_Next;
        current->m_Next = newEvent;
    }
    m_NumEvents++;
}

void cInterfaceEventManager::ClearEvents()
{
    if( m_Events )
        delete m_Events;
    m_Events = nullptr;
}

} // namespace WhoreMasterRenewal
