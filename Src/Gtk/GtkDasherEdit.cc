#include "GtkDasherEdit.h"

#include <string>

GtkDasherEdit::GtkDasherEdit( CDasherInterface *_interface )
  : Gtk::Text(), Dasher::CDashEditbox(), flush_count(0), interface( _interface )
{
}

GtkDasherEdit::~GtkDasherEdit()
{
}

void GtkDasherEdit::write_to_file()
{
}

void GtkDasherEdit::get_new_context(std::string& str, int max)
{
}

void GtkDasherEdit::unflush()
{
  backward_delete( flush_count );
  flush_count = 0;
}

void GtkDasherEdit::output(symbol Symbol)
{
  // FIXME - again, label is utf-8 encoded, and insert is probably not
  // expecting this to be the case

  string label;
  label = interface->GetEditText( Symbol );

  Gdk_Color black("black");
  Gdk_Font fixed_font("-misc-fixed-medium-r-*-*-*-140-*-*-*-*-*-*");
  Gdk_Color white("white");

  insert ( Gdk_Font(), black, white, label, 1);
}

void GtkDasherEdit::flush(symbol Symbol)
{
  ++flush_count;

  string label;

  label = interface->GetEditText( Symbol );

  Gdk_Color black("black");
  Gdk_Font fixed_font("-misc-fixed-medium-r-*-*-*-140-*-*-*-*-*-*");
  Gdk_Color white("white");

  insert ( Gdk_Font(), black, white, label, 1);
}

void GtkDasherEdit::Clear()
{
}

void GtkDasherEdit::SetEncoding(Opts::FileEncodingFormats Encoding)
{
}

void GtkDasherEdit::SetFont(std::string Name, long Size)
{
}
