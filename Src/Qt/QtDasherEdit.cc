// QtDasherScreen.cc
// (c) 2003 Yann Dirson
// Derived from GtkDasherCanvas.cc
// (c) 2002 Philip Cowans

#include <qevent.h>
#include "QtDasherEdit.h"

QtDasherEdit::QtDasherEdit (Dasher::CDasherInterface *_interface, QWidget * _parent) 
  : Dasher::CDashEditbox()
{
  interface=_interface;
}

QtDasherEdit::~QtDasherEdit()
{
}

void QtDasherEdit::unflush()
{
}

void QtDasherEdit::output(Dasher::symbol Symbol)
{
  std::string label;
  label=interface->GetEditText(Symbol);
  append(label.c_str());
}

void QtDasherEdit::deletetext()
{
}

void QtDasherEdit::flush(Dasher::symbol Symbol)
{
}

void QtDasherEdit::Clear()
{
}

void QtDasherEdit::SetEncoding(Dasher::Opts::FileEncodingFormats format)
{
}

void QtDasherEdit::SetFont(std::string fontname, long Size)
{
}

void QtDasherEdit::write_to_file()
{
}

void QtDasherEdit::get_new_context(std::string &context, int length) 
{
}
