#include "GtkDasherEdit.h"


#include <string>
#include <iostream.h>
#include <fstream.h>

GtkDasherEdit::GtkDasherEdit( CDasherInterface *_interface )
  : Gtk::Text(), Dasher::CDashEditbox(), flush_count(0), interface( _interface ), filename_set( false ), efont("-*-fixed-*-*-*-*-*-140-*-*-*-*-*-*")
{
  set_editable( true );
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
  //  Gdk_Font fixed_font("-misc-fixed-medium-r-*-*-*-140-*-*-*-*-*-*");
  Gdk_Color white("white");

  insert ( efont, black, white, label, 1);
}

void GtkDasherEdit::flush(symbol Symbol)
{
  ++flush_count;

  string label;

  label = interface->GetEditText( Symbol );

  Gdk_Color black("black");
  // Gdk_Font fixed_font("-misc-fixed-medium-r-*-*-*-140-*-*-*-*-*-*");
  Gdk_Color white("white");

  insert ( efont, black, white, label, 1);
}

void GtkDasherEdit::Cut()
{
  cut_clipboard();
}

void GtkDasherEdit::Copy()
{
  copy_clipboard();
}

void GtkDasherEdit::CopyAll()
{
  select_region(0, -1 );
  copy_clipboard();
}

void GtkDasherEdit::Paste()
{
  paste_clipboard();
}

void GtkDasherEdit::SelectAll()
{
  select_region(0, -1 );
}

void GtkDasherEdit::Clear()
{
  delete_text(0, -1 );
}

void GtkDasherEdit::SetEncoding(Opts::FileEncodingFormats Encoding)
{
}

void GtkDasherEdit::SetFont(std::string Name, long Size)
{
  char xfnbuffer[256];

  snprintf( xfnbuffer, 256, "-*-%s-*-*-*-*-%d-*-*-*-*-*-*-*", Name.c_str(), Size );

  efont.create(xfnbuffer);
}

bool GtkDasherEdit::SaveAs(std::string filename)
{
  current_filename = filename;
  filename_set = true;

  return( Save() );
}

bool GtkDasherEdit::Save()
{
  if( !filename_set )
    return( false );

  ofstream ofile( current_filename.c_str() );

  if( ofile.bad() )
    return( false );

  ofile << get_chars(0, -1) << endl;
  ofile.close();

  return( true );
}

bool GtkDasherEdit::Open( std::string filename )
{
  return( false );
}
