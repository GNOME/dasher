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
  l.set_usize( 128, 256 );
  
  get_vbox()->pack_start(l, true, true );
  get_vbox()->show_all();
  //  show();
}

void GtkDasherAlphabetBox::AddAlphabet( std::vector< std::string > alphabetlist )
{
  for( int i(0); i < alphabetlist.size(); ++i )
    cout << "Adding alphabet: " << alphabetlist[i] << endl;
}

