#include "GtkDasherEdit.h"

GtkDasherEdit::GtkDasherEdit()
  : Gtk::Text(), Dasher::CDashEditbox()
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
}

void GtkDasherEdit::output(symbol Symbol)
{
  cout << "Output " << Symbol << endl;

  char foo;

  foo = Symbol + 96;

  Gdk_Color black("black");
  Gdk_Font fixed_font("-misc-fixed-medium-r-*-*-*-140-*-*-*-*-*-*");
  Gdk_Color white("white");

   insert ( Gdk_Font(), black, white, &foo, 1);
}

void GtkDasherEdit::flush(symbol Symbol)
{
  cout << "Flush " << Symbol << endl;
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
