#include <qapplication.h>
#include <qpushbutton.h>

#include "Qt/QtDasherScreen.h"

int main (int argc, char **argv)
{
  QApplication app (argc, argv);

  QtDasherScreen *dasher = new QtDasherScreen (240, 320, 
					       new CDasherInterface, 0);
  app.setMainWidget (dasher);
  dasher->show();

  return app.exec();
}
