#ifndef GTK_DASHER_ALPHABET_BOX_H
#define GTK_DASHER_ALPHABET_BOX_H

#include <gtk--/dialog.h>
#include <gtk--/list.h>
#include <gtk--/button.h>
#include <gtk--/frame.h>

#include <string>
#include <vector>

class GtkDasherAlphabetBox : public Gtk::Dialog
{
 public:
  GtkDasherAlphabetBox();

  void AddAlphabet( std::vector< std::string > alphabetlist );

  Gtk::Button *get_ok_button();
  Gtk::Button *get_cancel_button();

  std::string get_selection();

 private:
  Gtk::List l;
  Gtk::Button b_ok;
  Gtk::Button b_cancel;
  Gtk::Frame f;
};

#endif
