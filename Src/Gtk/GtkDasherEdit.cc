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

  // FIXME - need to call handle_cursor_move when the cursor position changes.

  //  text.button_release_event.connect_after((SigC::Slot1<gint, GdkEventButton *>(this, &GtkDasherEdit::handle_cursor_move)));
  
  SigC::Slot1<gint, GdkEventButton *> s = SigC::slot(this, &GtkDasherEdit::handle_cursor_move);

  text.button_release_event.connect( s );
}

GtkDasherEdit::~GtkDasherEdit( )
{
}

void GtkDasherEdit::write_to_file()
{
}

gint GtkDasherEdit::handle_cursor_move( GdkEventButton *e )
{
  if( text.get_selection_start_pos() < text.get_selection_end_pos() )
    text.set_point( text.get_selection_start_pos() );
  else
    text.set_point( text.get_selection_end_pos() );

  //  text.delete_selection();

  kill_flush();

  interface->Start();
  interface->Redraw();

  return( true );
}

void GtkDasherEdit::kill_flush()
{
  flush_count = 0;
}

void GtkDasherEdit::get_new_context(std::string& str, int max)
{
  int start;
  int end;

  end = text.get_point();
  start = end - max;

  if( start < 0 )
    start = 0;

  str = text.get_chars( start, end );
}

void GtkDasherEdit::unflush()
{
  text.backward_delete( flush_count );
  flush_count = 0;

  //  cout << "Point is " << text.get_point() << endl;
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


  text.delete_selection();
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

  text.delete_selection();
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

void GtkDasherEdit::TimeStampNewFiles(bool Value)
{
  cout << "Timestamp set to " << Value << endl;
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

bool GtkDasherEdit::SaveAs(std::string filename, bool a)
{
  current_filename = filename;
  filename_set = true;

  return( Save(a) );
}

bool GtkDasherEdit::Save(bool a)
{
  if( !filename_set )
    return( false );

  ofstream ofile;

  if( a )
    ofile.open( current_filename.c_str(), ios::app );
  else
    ofile.open( current_filename.c_str() );

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
    
      int fpos(0);

      while( !ifile.eof() && (fpos < 1024) )
	{
	  fbuffer[fpos] = ifile.get();
	  
	  if( !ifile.eof() )
	    ++fpos;
	}

      text.insert( efont, black, white, fbuffer, fpos);
    }

  ifile.close();
  //  text.set_point( text.get_length() -1);

  text.thaw();

  // Restart the interface with the new context

  kill_flush();

  interface->Start();
  interface->Redraw();

  return( true );
}

