#include "GtkDasherWindow.h"
#include "GtkDasherPane.h"


#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gtk--/window.h>
#include <gtk--/main.h>
//#include <gnome--/app-helper.h> 
//#include <gnome--/app.h>

using namespace SigC;

GtkDasherWindow::GtkDasherWindow()
  : dasher_pane(), main_vbox(false, 0), toolbar(GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_BOTH ), menu(), Window() 
{  
  add(main_vbox);

  //set_contents(main_vbox);
  //  set_toolbar(toolbar);
  main_vbox.pack_start(menu, false,false,0);
  main_vbox.pack_start(toolbar,false,false,0);
  main_vbox.pack_start(dasher_pane,false,false,0);

  //Item(const Icon& icon, const Gtk::string& str, const Callback& cb, const Gtk::string& tip=Gtk::string());

  {
        using namespace Gtk::Toolbar_Helpers;
    //        using namespace Gnome::UI;

	//	Array<Info> a;	

	//    Gnome::UI::Icon new_icon("New");
	//    a.add( Gnome::UI::Item(new_icon, "New", bind<char*>( slot(this,&GtkDasherWindow::toolbar_button_cb), TB_NEW)));

     toolbar.tools().push_back(ButtonElem( "New", 
 					  bind<char*>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_NEW)));
     toolbar.tools().push_back(ButtonElem( "Open",
 					  bind<char*>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_OPEN)));

     toolbar.tools().push_back(ButtonElem( "Save",
 					  bind<char*>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_SAVE)));
     toolbar.tools().push_back(Space());
     toolbar.tools().push_back(ButtonElem( "Cut",
 					  bind<char*>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_CUT)));
     toolbar.tools().push_back(ButtonElem( "Copy",
 					  bind<char*>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_COPY)));
     toolbar.tools().push_back(ButtonElem( "Copy All",
 					  bind<char*>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_COPY_ALL)));
     toolbar.tools().push_back(ButtonElem( "Paste",
 					  bind<char*>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_PASTE)));
  }

  show_all();
  
  dasher_pane.clear();

}

GtkDasherWindow::~GtkDasherWindow()
{
}

void GtkDasherWindow::toolbar_button_cb(char *c)
{
  printf("toolbar_button_cb : %s\n",c);
}

int GtkDasherWindow::destroy_event_impl(GdkEventAny *event)
{
  Gtk::Main::quit();
  return( true );
}

int GtkDasherWindow::delete_event_impl(GdkEventAny *event)
{
  Gtk::Main::quit();
  return( false );
}
