// QtDasherScreen.h
// (c) 2003 Yann Dirson
// Derived from GtkDasherCanvas.h
// (c) 2002 Philip Cowans

#ifndef QT_DASHER_SCREEN_H
#define QT_DASHER_SCREEN_H

#include <string>
#include <sys/time.h>

#include <qwidget.h>
#include <qpainter.h>
#include <qfont.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qtimer.h>
#include <qcursor.h>

#include "DasherScreen.h"
#include "DashEdit.h"
#include "DasherInterface.h"

#include <iconv.h>

using namespace Dasher;

class QtDasherScreen : public QWidget, public Dasher::CDasherScreen

{
  Q_OBJECT
 public:
  QtDasherScreen (int _width, int _height,
		  CDasherInterface *_interface,
		  QWidget * _parent=0, Dasher::CDashEditbox* edit=0);
  QtDasherScreen::~QtDasherScreen();

  void SetFont(std::string Name)
    { fontname = Name; /* set_the_font(); */ }

  void SetFontSize(Dasher::Opts::FontSize fontsize)
    {
#warning QtDasherScreen::SetFontSize() not implemented
    }
  Dasher::Opts::FontSize GetFontSize()
    {
#warning QtDasherScreen::GetFontSize() not implemented
      return (Dasher::Opts::Normal);
    }
  
  void TextSize(symbol Character, int* Width, int* Height, int Size) const
    { 
      // should probably use QPainter::boundingRect()
      *Width = *Height = Fonts[Size].pixelSize();
      
    }
  void DrawText(symbol Character, int x1, int y1, int Size) const
    {
      //      QFont font = QFont (fontname.c_str(), Size);
      //      font.setPixelSize(Size);
      QPoint point = QPoint(x1, y1+Size/2);
  
      painter->setFont (Fonts[Size]);
      painter->drawText (point,
			 QString(interface->GetDisplayText(Character).c_str()));
    }

  void DrawRectangle(int x1, int y1, int x2, int y2,
		     int Color, Opts::ColorSchemes ColorScheme) const;
  void Polyline(point* Points, int Number) const;
  void DrawPolygon(point* Points, int Number, int Color,
		   Opts::ColorSchemes ColorScheme) const;

  std::vector<int> FontSizes;
  std::vector<QFont> Fonts;

  void Blank() const {
    painter->begin(pixmap);
    painter->setPen (NoPen);
    painter->fillRect(0, 0, m_iWidth, m_iHeight,
		      QColor(255,255,255));
  }
  void Display() {
    painter->end();
    repaint();
  }

  void paintEvent( QPaintEvent * )
  {
    bitBlt(this, 0, 0, pixmap);
  }

  void mousePressEvent (QMouseEvent *e);
  void mouseReleaseEvent (QMouseEvent *e);
  
 protected:
  QColor getColor(int Color, const Opts::ColorSchemes ColorScheme) const;
  
  long QtDasherScreen::get_time();

  CDasherInterface* interface;
  Dasher::CDashEditbox* edit;

  bool paused;

  QPainter* painter;

  QPixmap* pixmap;

  std::string fontname;

  protected slots:
    void timer();

};

#endif
