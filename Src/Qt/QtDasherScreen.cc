// QtDasherScreen.cc
// (c) 2003 Yann Dirson
// Derived from GtkDasherCanvas.cc
// (c) 2002 Philip Cowans

#include <iostream>
#include <string>

#include <qpointarray.h>
#include <qpoint.h>

#include "QtDasherScreen.h"
#include "DasherScreen.h"
#include "SettingsStore.h"

#define MAXFONTSIZE 25
#define MINFONTSIZE 8

#include <iconv.h>
#include <iostream>

QtDasherScreen::QtDasherScreen (int _width, int _height,
				CDasherInterface *_interface,
				QWidget * _parent, Dasher::CDashEditbox *edit):
  QWidget(_parent), interface( _interface ),
  fontname( "fixed" ), // fontsize(12),
  Dasher::CDasherScreen(_width, _height)
{
  // font = new QFont (fontname, fontsize);
  painter = new QPainter ();

  pixmap = new QPixmap (_width, _height);
  pixmap->setOptimization(QPixmap::BestOptim);
  interface->SetSettingsStore(new CSettingsStore);

  interface->ChangeLanguageModel(0);
  interface->ChangeView(0);
  interface->ChangeEdit(edit);

  interface->GetFontSizes(&FontSizes);

  for (int i=0; i<FontSizes.size(); i++) {
    if (FontSizes[i]>Fonts.size())
      Fonts.resize((FontSizes[i])+1);    
    Fonts[FontSizes[i]]= QFont (fontname.c_str(), FontSizes[i]);
    Fonts[FontSizes[i]].setPixelSize(FontSizes[i]);
  }

  interface->ChangeScreen(this);

  paused=true;
		     
  QTimer *tmr = new QTimer(this);
  connect (tmr, SIGNAL(timeout()), SLOT(timer()));
  tmr->start(200);

}

long QtDasherScreen::get_time()
{
  long s_now;
  long ms_now;
  
  struct timeval tv;
  struct timezone tz;
  
  gettimeofday( &tv, &tz );
  
  s_now = tv.tv_sec-1054487600;

  ms_now = tv.tv_usec / 1000;

  return( long(s_now*1000 + ms_now) );

}

QtDasherScreen::~QtDasherScreen()
{
  delete painter;
  delete interface;
  delete edit;
}

QColor QtDasherScreen::getColor(int Color, const Opts::ColorSchemes ColorScheme) const
{
  switch (ColorScheme) {
  case Dasher::Opts::Nodes1:
    switch (Color) {
    case 0: return QColor (180, 245, 180);
    case 1: return QColor (160, 200, 160);
    case 2: return QColor (0, 255, 255);
    default: abort ();
    }
  case Dasher::Opts::Nodes2:
    switch (Color) {
    case 0: return QColor (255, 185, 255);
    case 1: return QColor (140, 200, 255);
    case 2: return QColor (255, 175, 175);
    default: abort ();
    }
  case Dasher::Opts::Special1: return QColor (240, 240, 240);
  case Dasher::Opts::Special2: return QColor (255, 255, 255);
  case Dasher::Opts::Groups:
    switch (Color) {
    case 0: return QColor (255, 255, 0);
    case 1: return QColor (255, 100, 100);
    case 2: return QColor (0, 255, 0);
    default: abort ();
    }
  case Dasher::Opts::Objects: return QColor (0, 0, 0);
  default: abort();
  }   
}
  
void QtDasherScreen::DrawRectangle(int x1, int y1, int x2, int y2,
				   int Color, Opts::ColorSchemes ColorScheme) const
{
  painter->setBrush (getColor (Color, ColorScheme));
  painter->drawRect (x1, y1, x2-x1, y2-y1);
}

static void Points_to_QPointArray(const Dasher::CDasherScreen::point* const points,
				  int number,
				  QPointArray &qpa)
{
  for (int i = 0; i < number; i++) {
    qpa.setPoint (i, points[i].x, points[i].y);  
  }
}

void QtDasherScreen::Polyline(point* Points, int Number) const
{
  QPointArray qpa(Number);
  Points_to_QPointArray (Points, Number, qpa);
  painter->setPen (SolidLine);
  painter->drawPolyline (qpa);
  painter->setPen (NoPen);
}

void QtDasherScreen::DrawPolygon(point* Points, int Number, int Color,
				 Opts::ColorSchemes ColorScheme) const
{
  painter->setBrush (getColor (Color, ColorScheme));
  QPointArray qpa(Number);
  Points_to_QPointArray (Points, Number, qpa);
  painter->drawPolygon (qpa);
}

void QtDasherScreen::mousePressEvent (QMouseEvent *e)
{
    paused=false;
    interface->Unpause(get_time());
}

void QtDasherScreen::mouseReleaseEvent(QMouseEvent *e)
{
    QPoint p = e->pos();
    interface->PauseAt(p.x(), p.y());
    paused=true;
}

void QtDasherScreen::timer()
{
  if (paused==false) {
    QPoint cursorpos;
    cursorpos=this->cursor().pos();
    cursorpos=mapFromGlobal(cursorpos);

    interface->TapOn(cursorpos.x(), cursorpos.y(), get_time());
  }
}









