#include <qapplication.h>
#include <qpushbutton.h>
#include <qtopia/qpeapplication.h>

#include "Qt/QtDasherScreen.h"

int main (int argc, char **argv)
{
  QPEApplication app (argc, argv);

  QtDasherScreen *dasher = new QtDasherScreen (240, 310, 
					       new CDasherInterface, 0);
  app.setMainWidget (dasher);
  dasher->showMaximized();

  return app.exec();
}
