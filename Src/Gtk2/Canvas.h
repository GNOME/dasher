#ifndef __canvas_h__
#define __canvas_h__

#include "PangoCache.h"

#include "../DasherCore/DasherScreen.h"
#include "../DasherCore/DasherTypes.h"
#include "../DasherCore/CustomColours.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>


using namespace Dasher;

class CCanvas : public Dasher::CDasherScreen {

 public:

  CCanvas( GtkWidget *pCanvas, CPangoCache *pPangoCache );
  ~CCanvas();

  void ExposeEvent( GdkEventExpose *pEvent );

  // CDasherScreen methods


  void SetFont(std::string Name) {};
  void SetFontSize(Dasher::Opts::FontSize fontsize) {};
  Dasher::Opts::FontSize GetFontSize() const { return Dasher::Opts::FontSize(1); };
  void TextSize(const std::string &String, screenint* Width, screenint* Height, int Size) const;
  void DrawString(symbol Character, screenint x1, screenint y1, int Size) const;
  void DrawString(const std::string &String, screenint x1, screenint y1, int Size) const;
  void DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Color, Opts::ColorSchemes ColorScheme) const;
  void SendMarker( int iMarker );
  void Polyline(point* Points, int Number) const;
  void Polyline(point* Points, int Number, int Colour) const;
  void Polygon(point* Points, int Number, int Colour) const;
  void DrawPolygon(point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const { 
    // FIXME - not implemented 
  };
  void Blank() const;
  void Display();
  void SetColourScheme(const CCustomColours *Colours);

 private:
  
  GtkWidget *m_pCanvas;

  GdkPixmap *m_pDisplayBuffer;
  GdkPixmap *m_pDecorationBuffer;
  GdkPixmap *m_pOnscreenBuffer;

  GdkPixmap *m_pOffscreenBuffer;
  
  CPangoCache *m_pPangoCache;

  GdkColor *colours;
  
  int m_iWidth;
  int m_iHeight;

  PangoRectangle *m_pPangoInk;
  PangoRectangle *m_pPangoLogical;

  gulong lSignalHandler;

};

#endif
