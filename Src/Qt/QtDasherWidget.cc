// QtDasherScreen.cc
// (c) 2003 Yann Dirson
// Derived from GtkDasherCanvas.cc
// (c) 2002 Philip Cowans

#include "QtDasherWidget.h"
#include "QtDasherScreen.h"
#include "QtDasherEdit.h"
#include "DasherScreen.h"
#include "SettingsStore.h"
#include <qtextview.h>
#include <qvbox.h>
#include <qpushbutton.h>

QtDasherWidget::QtDasherWidget (int _width, int _height,
				CDasherInterface *_interface,
				QWidget * _parent) 
{
  showMaximized();
  QVBox *box = new QVBox(this);
  QtDasherEdit *dashereditbox = new QtDasherEdit(_interface,this);
  QtDasherScreen *dasherscreen = new QtDasherScreen(_width,_height,_interface,this,dashereditbox);

  box->resize(240,310);
  box->show();

  dashereditbox->setGeometry(0,0,240,60);
  dasherscreen->setGeometry(0,60,240,250);

  dashereditbox->show();
  dasherscreen->show();

}

QtDasherWidget::~QtDasherWidget()
{
}

