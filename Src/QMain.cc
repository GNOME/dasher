#include <qapplication.h>
#include <qpushbutton.h>

#include "Qt/QtDasherWidget.h"

int main (int argc, char **argv)
{
  QApplication app (argc, argv);
  QtDasherWidget *dasher = new QtDasherWidget (240, 250, 
			       new CDasherInterface, 0);
  app.setMainWidget (dasher);

  return app.exec();
}
