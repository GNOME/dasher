// QtDasherScreen.h
// (c) 2003 Yann Dirson
// Derived from GtkDasherCanvas.h
// (c) 2002 Philip Cowans

#ifndef QT_DASHER_EDIT_H
#define QT_DASHER_EDIT_H

#include <qwidget.h>
#include <qtextview.h>

#include "DasherScreen.h"
#include "DashEdit.h"
#include "DasherInterface.h"

#include "QtDasherEdit.h"

class QtDasherEdit : public QTextView, public Dasher::CDashEditbox

{
  Q_OBJECT
 public:
  QtDasherEdit (Dasher::CDasherInterface *_interface, QWidget * _parent=0);
  QtDasherEdit::~QtDasherEdit();
  void QtDasherEdit::unflush();
  void QtDasherEdit::output(Dasher::symbol Symbol);
  void QtDasherEdit::deletetext();
  void QtDasherEdit::flush(Dasher::symbol Symbol);
  void QtDasherEdit::Clear();
  void QtDasherEdit::SetEncoding(Dasher::Opts::FileEncodingFormats format);
  void QtDasherEdit::SetFont(std::string fontname, long Size);
  void QtDasherEdit::write_to_file();
  void QtDasherEdit::get_new_context(std::string &context, int length);
 private:
  Dasher::CDasherInterface* interface;
};

#endif
