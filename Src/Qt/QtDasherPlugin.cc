#include "QtDasherPlugin.h"

#include <qpe/global.h>
 
#include <qpainter.h>
#include <qlist.h>
#include <qbitmap.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qdialog.h>
#include <qscrollview.h>
#include <qpopupmenu.h>
#include <qhbuttongroup.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qwindowsystem_qws.h>

QtDasherPlugin::QtDasherPlugin(QWidget* parent, const char* name, WFlags f) : QFrame(parent,name,f)
{
  (new QHBoxLayout(this))->setAutoAdd(TRUE);
  interface = new CDasherInterface;
  interface->SetSystemLocation("/opt/QtPalmtop/share/dasher/");
  interface->Unpause(0);
  interface->Start();  
  d = new QtDasherScreen(240,100,interface,this,this);
  interface->ChangeMaxBitRate(2.5);
  d->show();
  utf8_codec = new QUtf8Codec;
}

QSize QtDasherPlugin::sizeHint() const
{
  return QSize(240,100);
}

QtDasherPlugin::~QtDasherPlugin()
{
  delete d;
}

void QtDasherPlugin::resetState()
{
  flushcount=0;
  interface->Start();
  interface->Redraw();
}

void QtDasherPlugin::unflush()
{
  if (flushcount==0)
    return;
  for (flushcount; flushcount>0; flushcount--) {
    deletetext();
  }
}

void QtDasherPlugin::output(int Symbol)
{
  std::string label = interface->GetEditText(Symbol);
  QString unicodestring = utf8_codec->toUnicode(label.c_str());
  for (int i=0; i<int(unicodestring.length()); i++) {
    emit key( unicodestring[i].unicode(), 0, 0, true, false );
    emit key( unicodestring[i].unicode(), 0, 0, false, false );
  }
}

void QtDasherPlugin::deletetext()
{
  emit key( 0, Qt::Key_Backspace, 0, true, false);
  emit key( 0, Qt::Key_Backspace, 0, false, false);
}

void QtDasherPlugin::flush(int Symbol)
{
  if (Symbol==0)
    return;
  output(Symbol);
  flushcount++;
}
