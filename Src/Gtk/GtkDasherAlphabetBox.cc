#include "GtkDasherAlphabetBox.h"
#include <gtk--/dialog.h>
#include <gtk--/listitem.h>
#include <gtk--/list.h>

#include <string>
#include <vector>
#include <list>

GtkDasherAlphabetBox::GtkDasherAlphabetBox()
  : Dialog()
{
  get_vbox()->pack_start(l, true, true );
  show_all();
  show();
}

void GtkDasherAlphabetBox::AddAlphabet( std::vector< std::string > alphabetlist )
{
  Gtk::List_Helpers::ItemList *al = &(l.items());

  for( int i(0); i < alphabetlist.size(); ++i )
    {
      cout << "Adding alphabet: " << alphabetlist[i] << endl;

      Gtk::ListItem li( alphabetlist[i] );
      al->push_back( li );

    }



}

