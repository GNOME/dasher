#include "Gtk2DasherPane.h"
#include "Gtk2DasherCanvas.h"
#include "Gtk2DasherEdit.h"

Gtk2DasherPane::Gtk2DasherPane(Gtk2DasherCanvas *dasher_canvas, Gtk2DasherEdit *dasher_edit)
{
  edit = dasher_edit;
  canvas = dasher_canvas;
}
  
