#ifndef __DASHER_CANVAS_HH
#define __DASHER_CANVAS_HH

#include <iostream>
#include <string>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "DasherScreen.h"
#include "DasherInterface.h"

#include "Gtk2DoubleBuffer.h"

using namespace Dasher;
using namespace Opts;

class Gtk2DasherCanvas
{ 
 public: 
  class Gtk2ScreenWrapper;

  Gtk2DasherCanvas(guint width, guint height, CDasherInterface *interface);
  ~Gtk2DasherCanvas();
  
  void SetFont(std::string Name);
  void TextSize(symbol Character, int* Width, int* Height, int Size) const;
  void DrawText(symbol Character, int x1, int y1, int Size) const;
  void DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const;
  void Polyline(Dasher::CDasherScreen::point* Points, int Number) const;
  void DrawPolygon(Dasher::CDasherScreen::point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const;
  void Blank() const;
  void Display();
  void clear();
  void CreateNewBuffer();

  GdkColor get_color(int Color, Opts::ColorSchemes ColorScheme) const;
  GdkFont *GetFont(int size) const;

  Gtk2DoubleBuffer *buffer;
  GtkWidget *canvas;
  CDasherInterface *interface;
  Gtk2ScreenWrapper *wrapper;

  Gtk2ScreenWrapper *get_wrapper() { return ( wrapper ); }

 protected:
  int pmwidth;
  int pmheight;
};

class Gtk2DasherCanvas::Gtk2ScreenWrapper : public Dasher::CDasherScreen 
{
 public:
  Gtk2ScreenWrapper(int width, int height, Gtk2DasherCanvas *owner); 

  void SetFont(std::string Name) { owner->SetFont( Name ); }

  GdkFont *GetFont(int size) const { owner->GetFont(size); }

  void TextSize(symbol Character, int* Width, int* Height, int Size) const { owner->TextSize( Character, Width, Height, Size ); }

  void DrawText(symbol Character, int x1, int y1, int Size) const { owner->DrawText( Character, x1, y1, Size ); }
  
  void DrawRectangle(int x1, int y1, int x2, int y2, int Color, Opts::ColorSchemes ColorScheme) const { owner->DrawRectangle( x1, y1, x2, y2, Color, ColorScheme ); }

  void Polyline(point* Points, int Number) const { owner->Polyline( Points, Number ); }

  void DrawPolygon(point* Points, int Number, int Color, Opts::ColorSchemes ColorScheme) const { owner->DrawPolygon( Points, Number, Color, ColorScheme ); }

  void Blank() const { owner->Blank(); }

  void Display() { owner->Display(); }

  void SetFontSize(Dasher::Opts::FontSize) {};
  Dasher::Opts::FontSize GetFontSize() {};

 private:
  Gtk2DasherCanvas *owner;
};

#endif

