
#include "cXmlWidget.h"

namespace WhoreMasterRenewal
{

cXmlWidget::cXmlWidget()
    : m_List()
{}

int cXmlWidget::size()
{
    return int( m_List.size() );
}

sXmlWidgetPart& cXmlWidget::operator[]( int i )
{
    return m_List[i];
}

void cXmlWidget::add( sXmlWidgetPart& part )
{
    m_List.push_back( part );
}

} // namespace WhoreMasterRenewal
