// IOstreamDasherEdit.cc
// (c) 2003 Yann Dirson
// Derived from GtkDasherEdit.cc
// (c) 2002 Philip Cowans

#include "IOstreamDasherEdit.h"

#include <string>
#include <iostream>

#include <iconv.h>

IOstreamDasherEdit::IOstreamDasherEdit( CDasherInterface *_interface, std::ostream* os )
  : Dasher::CDashEditbox(), outstream(os), flush_count(0), interface( _interface )
{
  enc = 1;
  snprintf( encstr, 255, "ISO-8859-%d", enc);

  *outstream << "Dasher starting\n";
}

IOstreamDasherEdit::~IOstreamDasherEdit( )
{
}

void IOstreamDasherEdit::write_to_file()
{
}

void IOstreamDasherEdit::kill_flush()
{
  flush_count = 0;
}

void IOstreamDasherEdit::get_new_context(std::string& str, int max)
{
  str = std::string();
}

void IOstreamDasherEdit::unflush()
{
  if (flush_count == 0)
    return;

  for (;flush_count>0;flush_count--)
    *outstream << "\b";
}

void IOstreamDasherEdit::output(symbol Symbol)
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
  iconv( cdesc, FUDGE &inbuffer, &inb, &convbuffer, &outb );
  
  std::string csymbol( cb, 256-outb );
  
  *outstream << csymbol;
}

void IOstreamDasherEdit::flush(symbol Symbol)
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
      iconv( cdesc, FUDGE &inbuffer, &inb, &convbuffer, &outb );
      
      std::string csymbol( cb, 256-outb );

      delete cb;
      delete ib;

      *outstream << csymbol;
      outstream->flush();
    }
}

void IOstreamDasherEdit::Clear()
{
}

void IOstreamDasherEdit::TimeStampNewFiles(bool Value)
{
}

void IOstreamDasherEdit::SetEncoding(Opts::FileEncodingFormats Encoding)
{
}

void IOstreamDasherEdit::SetFont(std::string Name, long Size)
{
}

void IOstreamDasherEdit::set_display_encoding( int _enc )
{
  if( _enc != enc )
    {
      enc = _enc;

      snprintf( encstr, 255, "ISO-8859-%d", enc);
    }
}

bool IOstreamDasherEdit::SaveAs(std::string filename, bool a)
{
  return( true );
}

bool IOstreamDasherEdit::Save(bool a)
{
  return( true );
}

bool IOstreamDasherEdit::Open( std::string filename )
{
  return( true );
}
