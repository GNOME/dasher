// QtDasherScreen.cc
// (c) 2003 Yann Dirson
// Derived from GtkDasherCanvas.cc
// (c) 2002 Philip Cowans

#include <qevent.h>
#include <stdio.h>
#include <qfile.h>
#include "QtDasherEdit.h"

QtDasherEdit::QtDasherEdit (Dasher::CDasherInterface *_interface, QWidget * _parent) 
  : QTextEdit(_parent), Dasher::CDashEditbox()
{
  interface=_interface;
  connect (this, SIGNAL(currentAlignmentChanged ( int )), this, SLOT(CursorMoved(int)));  
  connect (this, SIGNAL(selectionChanged ()), this, SLOT(CursorMoved()));
}

QtDasherEdit::~QtDasherEdit()
{
  outputstring="";
}

void QtDasherEdit::unflush()
{
}

void QtDasherEdit::output(Dasher::symbol Symbol)
{
  const QString label=QString::fromUtf8(interface->GetEditText(Symbol).c_str(),-1);
  insert(label,(unsigned int)CheckNewLines|RemoveSelected);
  outputstring+=label;
}

void QtDasherEdit::deletetext(Dasher::symbol Symbol)
{
  QString displaystring = QString::fromUtf8(interface->GetEditText(Symbol).c_str(),-1);

  for (int i=0; i<displaystring.length(); i++) {
    doKeyboardAction(ActionBackspace);
  }

  if (outputstring.length()>=displaystring.length()) {
    outputstring.truncate(outputstring.length()-displaystring.length());
  }
}

void QtDasherEdit::flush(Dasher::symbol Symbol)
{
}

void QtDasherEdit::Clear()
{
  setText("");
}

void QtDasherEdit::SetEncoding(Dasher::Opts::FileEncodingFormats format)
{
}

void QtDasherEdit::SetFont(std::string fontname, long Size)
{
}

void QtDasherEdit::write_to_file()
{
  std::string filename=interface->GetTrainFile();
  int fd=open(filename.c_str(),O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
  QFile file;
  file.open(IO_WriteOnly|IO_Append,fd);
  file.writeBlock(outputstring,outputstring.length());
  file.close();
  outputstring="";
}

void QtDasherEdit::get_new_context(std::string &context, int contextlength) 
{
  int position,para;
  QString edittext;
  QString contextstring;
  getCursorPosition(&para,&position);
  if (position<contextlength) {
    contextlength=position;
  }
  edittext=text();
  for (int i=0; i<contextlength; i++) {
    contextstring+=edittext.at(position-contextlength+i);
  }
  context=contextstring.utf8();
}

void QtDasherEdit::CursorMoved(int a)
{
  // FIXME - check if running
  //  interface->Start();
}


