// QtDasherPrefs.cc
// (c) 2003 Matthew Garrett

#include <qwidgetfactory.h>

#include "QtDasherPrefs.h"
#include "DasherInterface.h"

QtDasherPrefs::QtDasherPrefs (Dasher::CDasherInterface *_interface)
{
  interface=_interface;
  prefs = (QDialog *)QWidgetFactory::create("preferences.ui");
  alphabetbox = (QListBox *) prefs->child("alphabetlist", "QListBox");  
  colourbox = (QListBox *) prefs->child("colourlist", "QListBox");
  onedbutton = (QCheckBox *) prefs->child("onedbutton", "QCheckBox");
  eyetrackerbutton = (QCheckBox *) prefs->child("eyetrackerbutton", "QCheckBox");
  buttonstart = (QCheckBox *) prefs->child("buttonstart", "QCheckBox");
  spacestart = (QCheckBox *) prefs->child("spacestart", "QCheckBox");
  mouseposstart = (QCheckBox *) prefs->child("mouseposstart", "QCheckBox");
  copystop = (QCheckBox *) prefs->child("copystop", "QCheckBox");
  speakstop = (QCheckBox *) prefs->child("speakstop", "QCheckBox");
  windowpause = (QCheckBox *) prefs->child("windowpause", "QCheckBox");
  
  speedslider = (QSlider *) prefs->child("speedslider", "QSlider");
  orient1 = (QRadioButton *) prefs->child("orient1", "QRadioButton");
  orient2 = (QRadioButton *) prefs->child("orient2", "QRadioButton");
  orient3 = (QRadioButton *) prefs->child("orient3", "QRadioButton");
  orient4 = (QRadioButton *) prefs->child("orient4", "QRadioButton");
  orient5 = (QRadioButton *) prefs->child("orient5", "QRadioButton");
  showbar = (QCheckBox *) prefs->child("showbar", "QCheckBox");
  showmousepos = (QCheckBox *) prefs->child("showmousepos", "QCheckBox");
  mouseline = (QCheckBox *) prefs->child("mouseline", "QCheckBox");
  boxoutline = (QCheckBox *) prefs->child("boxoutline", "QCheckBox");
  autocolour = (QCheckBox *) prefs->child("autocolour", "QCheckBox");
  transparancy = (QSlider *) prefs->child("transparancy", "QSlider");
}

QtDasherPrefs::~QtDasherPrefs()
{
}

void QtDasherPrefs::exec() 
{
  if (prefs->exec()) {
    update();
  }
}

void QtDasherPrefs::update()
{
  interface->ChangeMaxBitRate(speedslider->value()/10);
  interface->ShowToolbar(showbar->isChecked());
  interface->CopyAllOnStop(copystop->isChecked());
  interface->DrawMouse(showmousepos->isChecked());
  interface->DrawMouseLine(mouseline->isChecked());
  interface->StartOnSpace(spacestart->isChecked());
  interface->StartOnLeft(buttonstart->isChecked());
  interface->WindowPause(windowpause->isChecked());
  interface->MouseposStart(mouseposstart->isChecked());
  interface->Speech(speakstop->isChecked());
  interface->PaletteChange(autocolour->isChecked());
  interface->SetDasherDimensions(onedbutton->isChecked());
  interface->SetDasherEyetracker(eyetrackerbutton->isChecked());
}
