// GtkDasherEdit.cc
// (c) 2002 Philip Cowans

#include "GtkDasherEdit.h"


#include <string>
#include <iostream>
#include <fstream>

#include <time.h>
#include <iconv.h>

GtkDasherEdit::GtkDasherEdit( CDasherInterface *_interface )
  : Gtk::HBox(), Dasher::CDashEditbox(), text(), vsb(), flush_count(0), interface( _interface ), filename_set( false ), efont("-*-fixed-*-*-*-*-*-140-*-*-*-*-*-*"), timestamp( true ), dirty( false )
{
  enc = 1;
  snprintf( encstr, 255, "ISO-8859-%d", enc);

  pack_start( text, true, true );
  pack_start( vsb, false, false );

  vsb.set_adjustment(text.get_vadjustment()); 

  text.set_editable( true );
  show_all();

  // FIXME - need to call handle_cursor_move when the cursor position changes.

  //  text.button_release_event.connect_after((SigC::Slot1<gint, GdkEventButton *>(this, &GtkDasherEdit::handle_cursor_move)));
  
  SigC::Slot1<gint, GdkEventButton *> s = SigC::slot(this, &GtkDasherEdit::handle_cursor_move);
  SigC::Slot1<gint, GdkEventKey *> s2 = SigC::slot(this, &GtkDasherEdit::handle_key_press);

  text.button_release_event.connect( s );
  text.key_press_event.connect_after( s2 );

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

gint GtkDasherEdit::handle_key_press( GdkEventKey *e )
{
  text.set_point( text.get_position() );

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

  dirty = true;

  // FIXME - we could do something more sophisticated with dirtying -
  // ie if you flush then unflush with the net result being that there
  // is no change then it might be a good idea to unset dirty.
}

void GtkDasherEdit::output(symbol Symbol)
{
  // FIXME - again, label is utf-8 encoded, and insert is probably not
  // expecting this to be the case

  std::string label;
  label = interface->GetEditText( Symbol );

  label = interface->GetEditText( Symbol );
  
  iconv_t cdesc=iconv_open(encstr,"UTF-8");
      
  char *convbuffer = new char[256];
  char *inbuffer = new char[256];
  
  char *cb( convbuffer );
  char *ib( inbuffer );
  
  strncpy( inbuffer, label.c_str(), 255 );
  
  size_t inb = label.length();
  
  size_t outb = 256;
  iconv( cdesc, &inbuffer, &inb, &convbuffer, &outb );
  
  std::string csymbol( cb, 256-outb );
  
  Gdk_Color black("black");
  //  Gdk_Font fixed_font("-misc-fixed-medium-r-*-*-*-140-*-*-*-*-*-*");
  Gdk_Color white("white");


  text.delete_selection();
  text.insert ( efont, black, white, csymbol, 1);

  dirty = true;
}

void GtkDasherEdit::flush(symbol Symbol)
{
  // We seem to be passed Symbol 0 (root node) sometimes, so ignore
  // this

  if( Symbol != 0 )
    {
      ++flush_count;

      std::string label;

      label = interface->GetEditText( Symbol );
  
      iconv_t cdesc=iconv_open(encstr,"UTF-8");
      
      char *convbuffer = new char[256];
      char *inbuffer = new char[256];

      char *cb( convbuffer );
      char *ib( inbuffer );

      strncpy( inbuffer, label.c_str(), 255 );

      size_t inb = label.length();

      size_t outb = 256;
      iconv( cdesc, &inbuffer, &inb, &convbuffer, &outb );
      
      std::string csymbol( cb, 256-outb );

      delete cb;
      delete ib;
      
      Gdk_Color black("black");
      Gdk_Color white("white");
      
      text.delete_selection();
      text.insert ( efont, black, white, csymbol, 1);

      dirty = true;
    }
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
  if( dirty )
    std::cout << "Warning - losing unsaved changes - probably should prompt here" << std::endl;

  if( timestamp )
    {
      tm *t_struct;
      
      time_t ctime;
      
      ctime = time( NULL );
      
      t_struct= localtime( &ctime );
      
      char tbuffer[256];
      
      snprintf( tbuffer, 256, "dasher-%d%d%d-%d%d.txt", (t_struct->tm_year+1900), (t_struct->tm_mon+1), t_struct->tm_mday, t_struct->tm_hour, t_struct->tm_min);
      
      current_filename = std::string(tbuffer);
    }
  else
    current_filename = std::string();

  text.delete_text(0, -1 );

  dirty = false;
}

std::string GtkDasherEdit::get_current_filename()
{
  return( current_filename );
}

void GtkDasherEdit::TimeStampNewFiles(bool Value)
{
  timestamp = Value;
}

void GtkDasherEdit::SetEncoding(Opts::FileEncodingFormats Encoding)
{
}

void GtkDasherEdit::SetFont(std::string Name, long Size)
{
  char xfnbuffer[256];
  
  fontname=Name;
  fontsize=Size;

  snprintf( xfnbuffer, 256, "-*-%s-*-*-*-*-%d-*-*-*-*-*-iso8859-%d", Name.c_str(), Size, enc );

  efont.create(xfnbuffer);
}

void GtkDasherEdit::set_display_encoding( int _enc )
{
  if( _enc != enc )
    {
      enc = _enc;

      snprintf( encstr, 255, "ISO-8859-%d", enc);

      /* Call SetFont with the same name and size as currently used - we just
	 want to force it to change the encoding */
      SetFont(fontname,fontsize); 

    }
}

bool GtkDasherEdit::SaveAs(std::string filename, bool a)
{
  std::string old_filename;

  old_filename = current_filename;

  current_filename = filename;
  filename_set = true;

  if( Save(a) )
    return( true );
  else
    {
      current_filename = old_filename;
      return( false );
    }
}

bool GtkDasherEdit::Save(bool a)
{
  if( !filename_set )
    return( false );

  std::ofstream ofile;

  if( a )
    ofile.open( current_filename.c_str(), std::ios::app );
  else
    ofile.open( current_filename.c_str() );

  if( ofile.bad() )
    return( false );

  std::string contents;

  contents = text.get_chars(0, -1);

  ofile << contents << std::endl;
  ofile.close();

  dirty = false;

  return( true );
}

bool GtkDasherEdit::Open( std::string filename )
{

  if( dirty )
    std::cout << "Warning - losing unsaved changes - probably should prompt here" << std::endl;


  //  current_filename = filename;

  std::ifstream ifile( filename.c_str(), std::ios::binary );
  
  if( ifile.bad() )
    return( false );

  current_filename = filename;

  text.freeze();
  text.delete_text(0, -1 );

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

  dirty = false;

  return( true );
}

bool GtkDasherEdit::is_dirty()
{
  return( dirty );
}
