
#include "cXmlWidget.h"

namespace WhoreMasterRenewal
{

cXmlWidget::cXmlWidget() {}

int cXmlWidget::size()
{
    return int( list.size() );
}

sXmlWidgetPart& cXmlWidget::operator[]( int i )
{
    return list[i];
}

void cXmlWidget::add( sXmlWidgetPart& part )
{
    list.push_back( part );
}

} // namespace WhoreMasterRenewal
