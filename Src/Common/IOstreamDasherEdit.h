// IOstreamDasherEdit.h
// (c) 2003 Yann Dirson
// Derived from GtkDasherEdit.h
// (c) 2002 Philip Cowans

#ifndef IOSTREAM_DASHER_EDIT_H
#define IOSTREAM_DASHER_EDIT_H

#include "DashEdit.h"
#include "DasherTypes.h"
#include "DasherInterface.h"

#include <string.h>
#include <iostream>

using namespace Dasher;

class IOstreamDasherEdit : public Dasher::CDashEditbox
{
 public:
  IOstreamDasherEdit( CDasherInterface *_interface, std::ostream* os = &std::cout );
  ~IOstreamDasherEdit();

  void write_to_file();
  void get_new_context(std::string& str, int max);
  void unflush();
  void output(symbol Symbol);
  void flush(symbol Symbol);
  void Clear();

  void SetEncoding(Opts::FileEncodingFormats Encoding);
  void SetFont(std::string Name, long Size);
  bool SaveAs( const std::string filename, bool a );
  bool Save( bool a=false);
  bool Open( const std::string filename );

  void TimeStampNewFiles(bool Value);

  void kill_flush();

  void set_display_encoding( int _enc );

 protected:
  int flush_count;
  CDasherInterface *interface;

  std::ostream *outstream;

  int enc;
  char encstr[256];
};

#endif
