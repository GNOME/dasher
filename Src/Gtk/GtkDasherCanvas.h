#ifndef DASHER_CANVAS_HH
#define DASHER_CANVAS_HH

#include <gtk--/drawingarea.h>
#include <gtk--/pixmap.h>
#include <gdk--/font.h>
#include "DasherScreen.h"
#include "GtkDoubleBuffer.h"

#include "DasherInterface.h"

#include <iconv.h>

using namespace Dasher;

class GtkDasherCanvas : public Gtk::DrawingArea
{

 public:
   
  class GtkScreenWrapper;

  GtkDasherCanvas( int _width, int _height, CDasherInterface *_interface );
  ~GtkDasherCanvas();
  
  void SetFont(std::string Name);
  void TextSize(symbol Character, int* Width, int* Height, int Size) const;
  void DrawText(symbol Character, int x1, int y1, int Size) const;
  void DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const;
  void Polyline(Dasher::CDasherScreen::point* Points, int Number) const;
  void DrawPolygon(Dasher::CDasherScreen::point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const;
  void Blank() const;
  void Display();

  void clear();
  void set_encoding( int _enc );

  //  Gdk_Font *get_font( int size ) const;

  GtkScreenWrapper *get_wrapper() { return( wrapper ); }

protected:
  CDasherInterface *interface;

  GtkScreenWrapper *wrapper;

  gint expose_event_impl(GdkEventExpose *event);

  int pmwidth;
  int pmheight;

  iconv_t cdesc;  // Descriptor for iconv stuff

  bool build_fonts( int encoding );

  string fontname;

  // Double buffers for the display

  GtkDoubleBuffer *buffer;

  Gdk_Font *font_list;
  bool *font_init;

  Gdk_Font f_large;
  Gdk_Font f_medium;
  Gdk_Font f_small;

  const Gdk_Font *get_font( int size ) const;

  void swap_buffers(); 

  int enc;

  //gint button_press_event_impl(GdkEventButton *event);
  //gint button_press_event_impl(GdkEventAny *event);

 public:
  class GtkScreenWrapper : public Dasher::CDasherScreen
    {
    public:
      GtkScreenWrapper(  int _width, int _height, GtkDasherCanvas *_owner );

      void SetFont(std::string Name) { owner->SetFont( Name ); }
	
      void TextSize(symbol Character, int* Width, int* Height, int Size) const { owner->TextSize( Character, Width, Height, Size ); }
      void DrawText(symbol Character, int x1, int y1, int Size) const { owner->DrawText( Character, x1, y1, Size ); }

      void DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const
	{
	  owner->DrawRectangle( x1, y1, x2, y2, Color, ColorScheme );
	}
      void Polyline(point* Points, int Number) const { owner->Polyline( Points, Number ); }
      void DrawPolygon(point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const
	{ 
	  owner->DrawPolygon( Points, Number, Color, ColorScheme );
	}
      void Blank() const { owner->Blank(); }
      void Display() {owner->Display(); }

    private:
      GtkDasherCanvas *owner;
    };
};

#endif
