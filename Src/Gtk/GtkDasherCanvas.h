#ifndef DASHER_CANVAS_HH
#define DASHER_CANVAS_HH

#include <gtk--/drawingarea.h>
#include <gtk--/pixmap.h>
#include <gdk--/font.h>
#include "DasherScreen.h"
#include "GtkDoubleBuffer.h"

using namespace Dasher;

class GtkDasherCanvas : public Gtk::DrawingArea,  public Dasher::CDasherScreen
{
public:
  GtkDasherCanvas( int _width, int _height );
  ~GtkDasherCanvas();
  
  void SetFont(std::string Name);
  void TextSize(symbol Character, int* Width, int* Height, int Size) const;
  void DrawText(symbol Character, int x1, int y1, int Size) const;
  void DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const;
  void Polyline(point* Points, int Number) const;
  void DrawPolygon(point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const;
  void Blank() const;
  void Display();

  void clear();

protected:
  gint expose_event_impl(GdkEventExpose *event);

  int width;
  int height;

  // Double buffers for the display

  GtkDoubleBuffer *buffer;

  Gdk_Font f_large;
  Gdk_Font f_medium;
  Gdk_Font f_small;

  void swap_buffers(); 

  //gint button_press_event_impl(GdkEventButton *event);
  //gint button_press_event_impl(GdkEventAny *event);
};

#endif
