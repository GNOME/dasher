// QtDasherPrefereces.h
// (c) 2004 Matthew Garrett

#ifndef QT_DASHER_PREFS_H
#define QT_DASHER_PREFS_H

#include <qwidget.h>
#include <qtextview.h>
#include <fcntl.h>
#include <qdialog.h>
#include <qslider.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include "DasherScreen.h"
#include "DashEdit.h"
#include "DasherInterface.h"

#include "QtDasherEdit.h"
#include "QtDasherScreen.h"

class QtDasherPrefs : public QObject

{
  Q_OBJECT
 public:
  QtDasherPrefs (Dasher::CDasherInterface *_interface);
  QtDasherPrefs::~QtDasherPrefs();
  void QtDasherPrefs::exec();
  void QtDasherPrefs::update();
 private:
  Dasher::CDasherInterface* interface;
  QDialog *prefs;
  QListBox *alphabetbox ;
  QListBox *colourbox ;
  QCheckBox *onedbutton ;
  QCheckBox *eyetrackerbutton ;
  QCheckBox *buttonstart ;
  QCheckBox *spacestart ;
  QCheckBox *mouseposstart ;
  QCheckBox *copystop ;
  QCheckBox *speakstop ;
  QCheckBox *windowpause ;
  QSlider *speedslider ;
  QRadioButton *orient1 ;
  QRadioButton *orient2 ;
  QRadioButton *orient3 ;
  QRadioButton *orient4 ;
  QRadioButton *orient5 ;
  QCheckBox *showbar ;
  QCheckBox *showmousepos ;
  QCheckBox *mouseline ;
  QCheckBox *boxoutline ;
  QCheckBox *autocolour ;
  QSlider *transparancy ;
};

#endif


