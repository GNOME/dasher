#include "GtkDasherAlphabetBox.h"
#include <gtk--/dialog.h>
#include <gtk--/listitem.h>
#include <gtk--/list.h>
#include <gtk--/label.h>

#include <string>
#include <vector>
#include <list>

GtkDasherAlphabetBox::GtkDasherAlphabetBox()
  : Dialog(), b_ok( "Ok" ), b_cancel( "Cancel" )
{
  //get_vbox()->set_border_width( 10 );
  set_title( "Alphabet" );

  l.set_usize( 374, 256 );
  l.set_selection_mode( GTK_SELECTION_SINGLE );

  f.set_border_width(4);
  f.set_shadow_type( GTK_SHADOW_IN );

  f.add( l );

  get_vbox()->pack_start(f, true, true );
  get_vbox()->show_all();

  get_action_area()->pack_start( b_ok, false, false );
  get_action_area()->pack_start( b_cancel, false, false );
  get_action_area()->show_all();
}

Gtk::Button *GtkDasherAlphabetBox::get_ok_button()
{
  return( &b_ok );
}

Gtk::Button *GtkDasherAlphabetBox::get_cancel_button()
{
  return( &b_cancel );
}

std::string GtkDasherAlphabetBox::get_selection()
{
    Gtk::List::SelectionList &selection = l.selection();

    if( selection.empty() )
      {
	return( string("") );
      }
    else
      {
	Gtk::List::SelectionList::iterator i=selection.begin();
	Gtk::ListItem *item = (*i);
	Gtk::Label *label = dynamic_cast<Gtk::Label*>(item->get_child());
	Gtk::string name=label->get();
	return( string( name ) );
      }
}

void GtkDasherAlphabetBox::set_selection( std::string _selection )
{
  Gtk::List::ItemList::iterator i( l.items().begin() );
  
  while( i != l.items().end() )
    {
      if( dynamic_cast<Gtk::Label*>((*i)->get_child())->get() == _selection )
	(*i)->select();
      i++;
    }
}

void GtkDasherAlphabetBox::AddAlphabet( std::vector< std::string > alphabetlist )
{
  for( int i(0); i < alphabetlist.size(); ++i )
    {
      cout << "Adding alphabet: " << alphabetlist[i] << endl;
      
      Gtk::ListItem *li;

      li = new Gtk::ListItem( alphabetlist[i] );

      l.add( *li );
      li->show();
    }
}

