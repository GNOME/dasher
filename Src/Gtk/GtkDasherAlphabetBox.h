#ifndef GTK_DASHER_ALPHABET_BOX_H
#define GTK_DASHER_ALPHABET_BOX_H

#include <gtk--/dialog.h>
#include <gtk--/list.h>

#include <string>
#include <vector>

class GtkDasherAlphabetBox : public Gtk::Dialog
{
 public:
  GtkDasherAlphabetBox();

  void AddAlphabet( std::vector< std::string > alphabetlist );
 private:
  Gtk::List l;
};

#endif
