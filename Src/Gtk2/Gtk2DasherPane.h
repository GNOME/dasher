#ifndef GPEDASHERPANE_HH
#define GPEDASHERPANE_HH

#include "Gtk2DasherCanvas.h"
#include "Gtk2DasherEdit.h"
#include "DasherSettingsInterface.h"

class Gtk2DasherPane
{
 public:
  Gtk2DasherPane(Gtk2DasherCanvas *dasher_canvas, Gtk2DasherEdit *dasher_edit);
  
  Gtk2DasherCanvas *canvas;
  Gtk2DasherEdit *edit;
};

#endif  
