#include "GtkDasherWindow.h"
#include "GtkDasherPane.h"


#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gtk--/window.h>
#include <gtk--/main.h>

using namespace SigC;

GtkDasherWindow::GtkDasherWindow()
  : dasher_pane(), main_vbox(false, 0), toolbar(GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_BOTH ), menubar(), Window(), save_dialogue()
{  
  add(main_vbox);

  //set_contents(main_vbox);
  //  set_toolbar(toolbar);
  main_vbox.pack_start(menubar, false,false,0);
  main_vbox.pack_start(toolbar,false,false,0);
  main_vbox.pack_start(dasher_pane,false,false,0);

  // use helpers to st up menu
  {
    using namespace Gtk::Menu_Helpers;
    
    // Create the file menu
    Menu *menu_file = new Menu();
    MenuList& list_file = menu_file->items();
    
    list_file.push_back(MenuElem("_New"));
    list_file.push_back(MenuElem("_Open..."));
    list_file.push_back(MenuElem("_Save"));
    list_file.push_back(MenuElem("Sa_ve As..."));
    list_file.push_back(MenuElem("_Append to File..."));
    list_file.push_back(SeparatorElem());
    list_file.push_back(MenuElem("_Import Training Test..."));
    list_file.push_back(SeparatorElem());
    list_file.push_back(MenuElem("E_xit"));

    Menu *menu_edit = new Menu();
    MenuList& list_edit = menu_edit->items();

    list_edit.push_back(MenuElem("Cut"));
    list_edit.push_back(MenuElem("Copy"));
    list_edit.push_back(MenuElem("Paste"));
    list_edit.push_back(SeparatorElem());
    list_edit.push_back(MenuElem("Copy All"));
    list_edit.push_back(SeparatorElem());
    list_edit.push_back(MenuElem("Select All"));

    Menu *menu_or = new Menu();
    MenuList& list_or = menu_or->items();

    list_or.push_back(MenuElem("Alphabet Default"));
    list_or.push_back(SeparatorElem());
    list_or.push_back(MenuElem("Left to Right"));
    list_or.push_back(MenuElem("Right to Left"));
    list_or.push_back(MenuElem("Top to Bottom"));
    list_or.push_back(MenuElem("Bottom to Top"));

    Menu *menu_tool = new Menu();
    MenuList &list_tool = menu_tool->items();

    list_tool.push_back(MenuElem("Visible"));
    list_tool.push_back(SeparatorElem());
    list_tool.push_back(MenuElem("Show Text"));
    list_tool.push_back(MenuElem("Large Icons"));

    Menu *menu_view = new Menu();
    MenuList& list_view = menu_view->items();
    
    list_view.push_back(MenuElem("Orientation", *menu_or ));
    list_view.push_back(SeparatorElem());
    list_view.push_back(MenuElem("Toolbar", *menu_tool));
    list_view.push_back(MenuElem("Speed Slider"));
    list_view.push_back(SeparatorElem());
    list_view.push_back(MenuElem("Fix Layout"));

    Menu *menu_enc = new Menu();
    MenuList &list_enc = menu_enc->items();

    list_enc.push_back(MenuElem("Unicode UTF-8"));

    Menu *menu_opts = new Menu();
    MenuList & list_opts = menu_opts->items();
    
    list_opts.push_back(MenuElem("Timestamp New Files"));
    list_opts.push_back(MenuElem("Copy All on Stop"));
    list_opts.push_back(SeparatorElem());
    list_opts.push_back(MenuElem("Alphabet..."));
    list_opts.push_back(MenuElem("File Encoding", *menu_enc ));
    list_opts.push_back(SeparatorElem());
    list_opts.push_back(MenuElem("Editing Font..."));
    list_opts.push_back(MenuElem("Dasher Font..."));
    list_opts.push_back(MenuElem("Reset Fonts"));
			
    Menu *menu_help = new Menu();
    MenuList &list_help = menu_help->items();

    list_help.push_back(MenuElem("About Dasher..."));


    // Create the menu bar
    //   Gtk+ does not have O(1) tail lookups so you should build menus 
    //   backwards whenever you plan to make lots of access to back().
    menubar.items().push_back(MenuElem("_File","<control>f",*menu_file));
    menubar.items().push_back(MenuElem("_Edit","<control>e",*menu_edit));
    menubar.items().push_back(MenuElem("_View","<control>v",*menu_view));
    menubar.items().push_back(MenuElem("_Options","<control>o",*menu_opts));
    menubar.items().push_back(MenuElem("_Help","<control>h",*menu_help));
  }

  //Item(const Icon& icon, const Gtk::string& str, const Callback& cb, const Gtk::string& tip=Gtk::string());

  {
        using namespace Gtk::Toolbar_Helpers;
	//using namespace Gnome::UI;

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


  if( !strcmp( c, TB_NEW ) )
    //    cout << "Restart dasher here" << endl;
    dasher_pane.reset();
    
  else if( !strcmp( c, TB_SAVE ) )
    save_dialogue.show();
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
