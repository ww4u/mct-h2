#include "xconfig.h"
#include "../include/mystd.h"
XConfig::XConfig(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy( Qt::StrongFocus );
}

void XConfig::setName( const QString &name )
{ mName = name; }
QString XConfig::name()
{ return mName; }

void XConfig::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent( event );

    emit signal_focus_in( mName );

}