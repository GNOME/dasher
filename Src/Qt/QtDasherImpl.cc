#include <iostream>
#include <qapplication.h>
#include <qobject.h>
#include <qpixmap.h>
#include "DasherInterface.h"
#include "QtDasherScreen.h"
#include "QtDasherImpl.h"

/* XPM */
static const char * qtdasher_xpm[]={
"28 7 2 1",
"# c #303030",
"  c None",
" ########################## ",
"                            ",
"    #     #                 ",
"   # #   # #                ",
"  #   # #   #               ",
" #     #     #              ",
" ########################## "};


QtDasherImpl::QtDasherImpl()
  : qtdasherwidget(0), icn(0), ref(0), qtdasherinterface(0)
{
}

QtDasherImpl::~QtDasherImpl()
{
    delete qtdasherwidget;
    delete icn;
}

QWidget *QtDasherImpl::inputMethod( QWidget *parent, Qt::WFlags f )
{
  if ( !qtdasherwidget ) {
	qtdasherwidget = new QtDasherPlugin( parent, "Dasher", f );
  }
  return qtdasherwidget;
}

void QtDasherImpl::resetState()
{
    if ( qtdasherwidget )
      qtdasherwidget->resetState();
}

QPixmap *QtDasherImpl::icon()
{
    if ( !icn )
	icn = new QPixmap( (const char **)qtdasher_xpm );
    return icn;
}

QString QtDasherImpl::name()
{
    return qApp->translate( "InputMethods", "Dasher" );
}

void QtDasherImpl::onKeyPress( QObject *receiver, const char *slot )
{
    if ( qtdasherwidget )
      QObject::connect( qtdasherwidget, SIGNAL(key(ushort,ushort,ushort,bool,bool)), receiver, slot );
}

#ifndef QT_NO_COMPONENT
QRESULT QtDasherImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_InputMethod )
	*iface = this;

    if ( *iface )
	(*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( QtDasherImpl )
}
#endif

