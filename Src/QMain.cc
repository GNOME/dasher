#include <qapplication.h>
#include <qpushbutton.h>
#include <qtopia/qpeapplication.h>

#include "Qt/QtDasherWidget.h"

int main(int argc, char **argv) {
  QPEApplication app(argc, argv);

  QtDasherWidget *dasher = new QtDasherWidget(240, 250,
                                              new CDasherInterface, 0);
  app.setMainWidget(dasher);
  dasher->showMaximized();

  return app.exec();
}
