#include "GtkDasherEdit.h"


#include <string>
#include <iostream.h>
#include <fstream.h>

GtkDasherEdit::GtkDasherEdit( CDasherInterface *_interface )
  : Gtk::HBox(), Dasher::CDashEditbox(), text(), vsb(), flush_count(0), interface( _interface ), filename_set( false ), efont("-*-fixed-*-*-*-*-*-140-*-*-*-*-*-*")
{
  pack_start( text, true, true );
  pack_start( vsb, false, false );

  vsb.set_adjustment(text.get_vadjustment()); 

  text.set_editable( true );
  show_all();
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
  text.backward_delete( flush_count );
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

  text.insert ( efont, black, white, label, 1);
}

void GtkDasherEdit::flush(symbol Symbol)
{
  ++flush_count;

  string label;

  label = interface->GetEditText( Symbol );

  Gdk_Color black("black");
  // Gdk_Font fixed_font("-misc-fixed-medium-r-*-*-*-140-*-*-*-*-*-*");
  Gdk_Color white("white");

  text.insert ( efont, black, white, label, 1);
}

void GtkDasherEdit::Cut()
{
  text.cut_clipboard();
}

void GtkDasherEdit::Copy()
{
  text.copy_clipboard();
}

void GtkDasherEdit::CopyAll()
{
  text.select_region(0, -1 );
  text.copy_clipboard();
}

void GtkDasherEdit::Paste()
{
  text.paste_clipboard();
}

void GtkDasherEdit::SelectAll()
{
  text.select_region(0, -1 );
}

void GtkDasherEdit::Clear()
{
  text.delete_text(0, -1 );
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

  string contents;

  contents = text.get_chars(0, -1);

  ofile << contents << endl;
  ofile.close();

  return( true );
}

bool GtkDasherEdit::Open( std::string filename )
{
  cout << "Filename is " << filename << endl;

  current_filename = filename;

  ifstream ifile( filename.c_str(), ios::binary );
  
  if( ifile.bad() )
    return( false );

  text.freeze();

  Clear();

  char fbuffer[ 1024 ];

  Gdk_Color black("black");
  Gdk_Color white("white");

  while( !ifile.eof() )
    {
      

      ifile.read( fbuffer, 1023 );
      fbuffer[1023] = 0;

      string rtext( fbuffer );

      //      ifile >> rtext;

      //cout << rtext << endl;

      text.insert( efont, black, white, rtext.c_str(), 1023);
    }

  ifile.close();

  text.thaw();

  return( true );
}
