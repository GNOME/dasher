// QtDasherScreen.h
// (c) 2003 Yann Dirson
// Derived from GtkDasherCanvas.h
// (c) 2002 Philip Cowans

#ifndef QT_DASHER_WIDGET_H
#define QT_DASHER_WIDGET_H

#include <qwidget.h>

#include "DasherScreen.h"
#include "DashEdit.h"
#include "DasherInterface.h"

#include "QtDasherScreen.h"

using namespace Dasher;

class QtDasherWidget : public QWidget

{
  Q_OBJECT
 public:
  QtDasherWidget (int _width, int _height,
		  CDasherInterface *_interface,
		  QWidget * _parent=0);
  QtDasherWidget::~QtDasherWidget();

};

#endif
