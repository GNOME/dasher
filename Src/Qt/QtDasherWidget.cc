// QtDasherScreen.cc
// (c) 2003 Yann Dirson
// Derived from GtkDasherCanvas.cc
// (c) 2002 Philip Cowans

#include "QtDasherWidget.h"
#include "QtDasherScreen.h"
#include "QtDasherEdit.h"
#include "QtDasherPrefs.h"
#include "DasherScreen.h"
#include "SettingsStore.h"
#include <qtextview.h>
#include <qvbox.h>
#include <qpushbutton.h>

QtDasherWidget::QtDasherWidget (int _width, int _height,
				CDasherInterface *interface,
				QWidget * _parent) : interface(interface) , dashereditbox(0), dasherscreen(0)
{
  resize(_width,_height);
  setGeometry(0,0,_width,_height);
  QVBox *box = new QVBox(this);
  QSize widgetsize = size();
  dashereditbox = new QtDasherEdit(interface,NULL);
  dasherscreen = new QtDasherScreen(widgetsize.width(),widgetsize.height(),interface,this);
  prefs = new QtDasherPrefs(interface);

  box->resize(widgetsize.width(),widgetsize.height());
  box->show();

  interface->ColourMode(false);
  interface->ChangeLanguageModel(0);
  interface->ChangeView(0);
  interface->ChangeEdit(dashereditbox);
  interface->SetSettingsStore(new CSettingsStore);

  dashereditbox->setGeometry(0,0,240,60);
  dasherscreen->setGeometry(0,0,widgetsize.width(),widgetsize.height());

  dashereditbox->show();
  dasherscreen->show();
  prefs->exec();
  show();
}

QtDasherWidget::~QtDasherWidget()
{
}

void QtDasherWidget::resizeEvent(QResizeEvent *event)
{
  if (dasherscreen!=NULL) {    
    delete dasherscreen;
    dasherscreen = new QtDasherScreen(size().width(),size().height(),interface,this);
    dasherscreen->setGeometry(0,0,size().width(),size().height());
    dasherscreen->show();
  }
}
