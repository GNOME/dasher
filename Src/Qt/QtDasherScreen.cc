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

QtDasherScreen::QtDasherScreen (int _width, int _height,
				CDasherInterface *_interface,
				QWidget * _parent):
  QWidget(_parent), interface( _interface ),
  fontname( "fixed" ), // fontsize(12),
  Dasher::CDasherScreen(_width, _height)
{
  // font = new QFont (fontname, fontsize);
  painter = new QPainter ();

  pixmap = new QPixmap (_width, _height);

  interface->ChangeScreen(this);

  paused=true;

  QTimer *tmr = new QTimer(this);
  connect (tmr, SIGNAL(timeout()), SLOT(timer()));
  tmr->start(40);
}

long QtDasherScreen::get_time()
{
  long s_now;
  long ms_now;
  
  struct timeval tv;
  struct timezone tz;
  
  gettimeofday( &tv, &tz );
  
  s_now = tv.tv_sec;
  ms_now = tv.tv_usec / 1000;
  
  return( s_now * 1000 + ms_now );
}

QtDasherScreen::~QtDasherScreen()
{
  delete painter;
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

void QtDasherScreen::DrawText(std::string String, int x1, int y1, int Size) const
{
  QFont font = QFont (fontname.c_str(), Size);
  font.setPixelSize(Size);
  QPoint point = QPoint(x1, y1+Size/2);
  
  
  
  
  painter->setFont (font);
  painter->drawText (point,
		     QString::fromUtf8(String.c_str()));
};

void QtDasherScreen::DrawText(symbol Character, int x1, int y1, int Size) const
{
  DrawText(interface->GetDisplayText(Character),x1,y1,Size);
}

void QtDasherScreen::DrawRectangle(int x1, int y1, int x2, int y2,
				   int Color, Opts::ColorSchemes ColorScheme) const
{
  painter->setPen (NoPen);
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
  Polyline(Points,Number,0);
}

void QtDasherScreen::Polyline(point* Points, int Number, int Colour) const
{
  QPointArray qpa(Number);
  Points_to_QPointArray (Points, Number, qpa);
  painter->setPen (SolidLine);
  painter->drawPolyline (qpa);

}

void QtDasherScreen::DrawPolygon(point* Points, int Number, int Color,
				 Opts::ColorSchemes ColorScheme) const
{
  painter->setPen (NoPen);
  painter->setBrush (getColor (Color, ColorScheme));
  QPointArray qpa(Number);
  Points_to_QPointArray (Points, Number, qpa);
  painter->drawPolygon (qpa);
}

void QtDasherScreen::mousePressEvent (QMouseEvent *e)
{
  if (paused==true) {
    paused=false;
    interface->Unpause(get_time());
  } else {
    paused=true;
    interface->PauseAt(0,0);
  }
}

void QtDasherScreen::mouseReleaseEvent(QMouseEvent *e)
{
  return;
}

void QtDasherScreen::resize(int x, int y)
{
  pixmap->resize(x,y);
}

void QtDasherScreen::timer()
{
  if (paused==false) {
    QPoint cursorpos;
    cursorpos=this->cursor().pos();
    cursorpos=mapFromGlobal(cursorpos);

    //FIXME - I've hard-coded this to take the height of the title bar into 
    //account

    interface->TapOn(cursorpos.x(), cursorpos.y(), get_time());
  }
}









