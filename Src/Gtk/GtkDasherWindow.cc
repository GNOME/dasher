#include "GtkDasherWindow.h"
#include "GtkDasherPane.h"

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gtk--/window.h>
#include <gtk--/main.h>
#include <gtk--/dialog.h>
#include <gtk--/radiomenuitem.h>

using namespace SigC;

GtkDasherWindow::GtkDasherWindow()
  : dasher_pane( this ), main_vbox(false, 0), toolbar(GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_BOTH ), menubar(), Window(), save_dialogue(), aboutbox(), dfontsel("Dasher Font"), efontsel("Editing Font"), slider_shown( true ),toolbar_shown(true), ofilesel("Open"), afilesel("Append To File"), copy_all_on_pause( false ),ifilesel("Import Training Text"), button("Close"), label("Dasher - Version 3.0.0 preview 2\nWeb: http://www.inference.phy.cam.ac.uk/dasher/\nemail: dasher@mrao.cam.ac.uk"), fix_pane( false ), timestamp( false ), current_or( Alphabet )
{
  //  dasher_pane.set_settings_ui( this );
  
  set_title( "Dasher" );

  add(main_vbox);

  //set_contents(main_vbox);
  //  set_toolbar(toolbar);
  main_vbox.pack_start(menubar, false,false,0);
  main_vbox.pack_start(toolbar,false,false,0);
  main_vbox.pack_start(dasher_pane,true,true,0);

  // use helpers to st up menu
  {
    using namespace Gtk::Menu_Helpers;


    // ** WARNING ** If you change the order of the menus, you need to
    // update the array indicies used to access the checkboxes etc...
    
    // Create the file menu
    Menu *menu_file = new Menu();
    MenuList& list_file = menu_file->items();
    
    list_file.push_back(MenuElem("_New",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_NEW)));
    list_file.push_back(MenuElem("_Open...",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_OPEN)));
    list_file.push_back(MenuElem("_Save",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_SAVE)));
    list_file.push_back(MenuElem("Sa_ve As...",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_SAVEAS)));

       list_file.push_back(MenuElem( "_Append to File...",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						     MENU_APPEND)));
    list_file.push_back(SeparatorElem());
    list_file.push_back(MenuElem("_Import Training Text...",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_IMPORT)));
    list_file.push_back(SeparatorElem());
    list_file.push_back(MenuElem("E_xit",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_EXIT)));

    Menu *menu_edit = new Menu();
    MenuList& list_edit = menu_edit->items();

    list_edit.push_back(MenuElem("Cut",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_CUT)));
    list_edit.push_back(MenuElem("Copy",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_COPY)));
    list_edit.push_back(MenuElem("Paste",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_PASTE)));
    list_edit.push_back(SeparatorElem());
    list_edit.push_back(MenuElem("Copy All",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_COPYALL)));
    list_edit.push_back(SeparatorElem());
    list_edit.push_back(MenuElem("Select All",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_SELECTALL)));

    Menu *menu_or = new Menu();
    list_or = &(menu_or->items());

    RadioMenuItem_Helpers::Group ogroup;

    list_or->push_back(RadioMenuElem(ogroup,"Alphabet Default",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_ODEFAULT)));
    list_or->push_back(SeparatorElem());
    list_or->push_back(RadioMenuElem(ogroup,"Left to Right",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_OLR)));
    list_or->push_back(RadioMenuElem(ogroup,"Right to Left",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_ORL)));
    list_or->push_back(RadioMenuElem(ogroup,"Top to Bottom",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_OTB)));
    list_or->push_back(RadioMenuElem(ogroup,"Bottom to Top",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_OBT)));


    Menu *menu_view = new Menu();
    list_view = &(menu_view->items());
    
    list_view->push_back(MenuElem("Orientation", *menu_or ));
    list_view->push_back(SeparatorElem());
    //    list_view.push_back(MenuElem("Toolbar", *menu_tool));
    list_view->push_back(CheckMenuElem("Show Toolbar",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
								MENU_TOOLBAR)));
    list_view->push_back(CheckMenuElem("Speed Slider",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_SLIDER)));
    list_view->push_back(SeparatorElem());
    list_view->push_back(CheckMenuElem("Fix Layout",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_FIX)));

    static_cast<CheckMenuItem *>( (*list_view)[2] )->set_active( true );
    static_cast<CheckMenuItem *>( (*list_view)[3] )->set_active( true );

    static_cast<CheckMenuItem *>( (*list_view)[5] )->set_sensitive( false );

    Menu *menu_enc = new Menu();
    MenuList &list_enc = menu_enc->items();

    list_enc.push_back(MenuElem("Unicode UTF-8"));

    Menu *menu_opts = new Menu();
    list_opts = &menu_opts->items();

    CheckMenuElem timestamp_elem("Timestamp New Files",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
								  MENU_TIMESTAMP));
    

    //    static_cast<CheckMenuItem *>( timestamp_elem.get_child() )->set_active( true );

    list_opts->push_back(timestamp_elem);
    list_opts->push_back(CheckMenuElem("Copy All on Stop",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_CAOS)));
    list_opts->push_back(SeparatorElem());
    list_opts->push_back(MenuElem("Alphabet...", bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_ALPHABET)));
    list_opts->push_back(MenuElem("File Encoding", *menu_enc ));
    list_opts->push_back(SeparatorElem());
    list_opts->push_back(MenuElem("Editing Font...",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_EFONT)));
    list_opts->push_back(MenuElem("Dasher Font...",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_DFONT)));
    list_opts->push_back(MenuElem("Reset Fonts", bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_RFONT)));

    static_cast<MenuItem *>( (*list_opts)[0] )->set_sensitive( false );
    static_cast<MenuItem *>( (*list_opts)[4] )->set_sensitive( false );

    //    static_cast<CheckMenuItem *>( list_opts[0] )->set_active( false );
    Menu *menu_help = new Menu();
    MenuList &list_help = menu_help->items();

    list_help.push_back(MenuElem("About Dasher...",bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_ABOUT)));


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
					      bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
							   TB_NEW)));
     toolbar.tools().push_back(ButtonElem( "Open",
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_OPEN)));

     toolbar.tools().push_back(ButtonElem( "Save",
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_SAVE)));
     toolbar.tools().push_back(Space());
     toolbar.tools().push_back(ButtonElem( "Cut",
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_CUT)));
     toolbar.tools().push_back(ButtonElem( "Copy",
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_COPY)));
     toolbar.tools().push_back(ButtonElem( "Copy All",
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_COPY_ALL)));
     toolbar.tools().push_back(ButtonElem( "Paste",
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_PASTE)));
  }

  dasher_pane.set_settings_ui( this );

  save_dialogue.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::file_ok_sel));
  dfontsel.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::dfont_ok_sel));
  dfontsel.get_cancel_button()->clicked.connect(slot(this, &GtkDasherWindow::dfont_cancel_sel));
  efontsel.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::efont_ok_sel));
  efontsel.get_cancel_button()->clicked.connect(slot(this, &GtkDasherWindow::efont_cancel_sel));

  ofilesel.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::ofile_ok_sel));
  ofilesel.get_cancel_button()->clicked.connect(slot(this, &GtkDasherWindow::ofile_cancel_sel));

  ifilesel.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::ifile_ok_sel));
  ifilesel.get_cancel_button()->clicked.connect(slot(this, &GtkDasherWindow::ifile_cancel_sel)); 

  afilesel.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::afile_ok_sel));
  afilesel.get_cancel_button()->clicked.connect(slot(this, &GtkDasherWindow::afile_cancel_sel));
  show_all();
  
  dasher_pane.clear();

  aboutbox.set_title("About Dasher");


  
  Gtk::HBox *dialogHBox = aboutbox.get_action_area();
  dialogHBox->pack_start (button, false, false, 0);


  abframe.set_border_width(8);
  label.set_padding(8,8);
  abframe.add( label );

  aboutbox.get_vbox()->pack_start (abframe, true, true, 0);


  aboutbox.get_action_area()->show_all();
  aboutbox.get_vbox()->show_all();
    

  button.clicked.connect(slot(this, &GtkDasherWindow::about_close_sel));

}



GtkDasherWindow::~GtkDasherWindow()
{
}

void GtkDasherWindow::file_ok_sel()
{
  save_dialogue.current_filename = save_dialogue.get_filename();

  save_dialogue.set_filename(save_dialogue.current_filename.c_str());
  save_dialogue.hide();
  //  cout << "foo" << endl;
  
  if( dasher_pane.save_as( save_dialogue.current_filename ))
    {
      char tbuffer[256];
      
      snprintf( tbuffer, 256, "Dasher - %s", ofilesel.get_filename().c_str());
      
      set_title(tbuffer);
    }
}

void GtkDasherWindow::ifile_ok_sel()
{
  ifilesel.hide();
  dasher_pane.import( ifilesel.get_filename() );
}

void GtkDasherWindow::ifile_cancel_sel()
{
  ifilesel.hide();
}

void GtkDasherWindow::afile_ok_sel()
{
  afilesel.hide();
  dasher_pane.append( afilesel.get_filename() );
}

void GtkDasherWindow::afile_cancel_sel()
{
  afilesel.hide();
}

void GtkDasherWindow::dfont_ok_sel()
{
  dfontsel.hide();

  nstring n( dfontsel.get_font_name() );

  const char *fname( n.gc_str() );
  
  char fnbuffer[256];

  int field(0);
  int spos(0);

  for( int i(0); i < strlen( fname ); ++i )
    {
      if( fname[i] == '-' )
	++field;
      else if(( field == 2 ) && ( spos < 255 ))
	{
	  fnbuffer[ spos ] = fname[i];
	  ++spos;
	}
    }

  fnbuffer[spos] = 0;

  dasher_pane.set_dasher_font( string( fnbuffer ));
}

void GtkDasherWindow::dfont_cancel_sel()
{
  dfontsel.hide();
}

void GtkDasherWindow::efont_ok_sel()
{
  efontsel.hide();

  nstring n( efontsel.get_font_name() );

  const char *fname( n.gc_str() );
  
  char fnbuffer[256];
  char fsbuffer[256];

  int field(0);
  int spos(0);
  int spos2(0);

  for( int i(0); i < strlen( fname ); ++i )
    {
      if( fname[i] == '-' )
	++field;
      else if(( field == 2 ) && ( spos < 255 ))
	{
	  fnbuffer[ spos ] = fname[i];
	  ++spos;
	}
      else if(( field == 7 ) && ( spos2 < 255 ))
	{
	  fsbuffer[ spos2 ] = fname[i];
	  ++spos2;
	}
    }

  fnbuffer[spos] = 0;
  fsbuffer[spos2] = 0;
  
  long size;

  size = atol( fsbuffer );

  dasher_pane.set_edit_font( string( fnbuffer ), size);
}

void GtkDasherWindow::efont_cancel_sel()
{
  efontsel.hide();
}

void GtkDasherWindow::ofile_ok_sel()
{
  if( dasher_pane.open(ofilesel.get_filename()))
    {
      char tbuffer[256];

      snprintf( tbuffer, 256, "Dasher - %s", ofilesel.get_filename().c_str());

      set_title(tbuffer);
    }
  ofilesel.hide();
}

void GtkDasherWindow::ofile_cancel_sel()
{
  ofilesel.hide();
}

void GtkDasherWindow::about_close_sel()
{
  aboutbox.hide();
}

void GtkDasherWindow::toolbar_button_cb(int c)
{
  switch( c )
    {
    case TB_NEW:
      reset();
      break;
    case TB_OPEN:
      open();
      break;
    case TB_SAVE:
      save_dialogue.show();
      break;
    case TB_CUT:
      cut();
      break;
    case TB_COPY:
      copy();
      break;
    case TB_COPY_ALL:
      copy_all();
      break;
    case TB_PASTE:
      paste();
      break;
    default:
      cout << "Undefined toolbar action called" << endl;
    }
}

void GtkDasherWindow::menu_button_cb(int c)
{
  switch( c )
    {

      // File menu

    case MENU_NEW:
      reset();
      break;
    case MENU_OPEN:
      open();
      break;
    case MENU_SAVE:
      save();
      break;
    case MENU_SAVEAS:
      save_as();
      break;
    case MENU_APPEND:
      afilesel.show();
      break;
    case MENU_IMPORT:
      ifilesel.show();
      break;
    case MENU_EXIT:
      exit();
      break;

      // Edit menu

    case MENU_CUT:
      cut();
      break;
    case MENU_COPY:
      copy();
      break;
    case MENU_PASTE:
      paste();
      break;
    case MENU_COPYALL:
      copy_all();
      break;
    case MENU_SELECTALL:
      select_all();
      break;

    case MENU_TOOLBAR:
      toggle_toolbar();
      break;
    case MENU_SLIDER:
      toggle_slider();
      break;
    case MENU_FIX:
      toggle_fix();
      break;

    case MENU_ODEFAULT:
      orientation( Alphabet );
      break;
    case MENU_OLR:
      orientation( LeftToRight );
      break;
    case MENU_ORL:
      orientation( RightToLeft );
      break;
    case MENU_OTB:
      orientation( TopToBottom );
      break;
    case MENU_OBT:
      orientation( BottomToTop );
      break;

    case MENU_TIMESTAMP:
      toggle_timestamp();
      break;
    case MENU_CAOS:
      toggle_copy_all();
      break;
    case MENU_ALPHABET:
      dasher_pane.show_alphabet_box();
      break;
    case MENU_EFONT:
      efontsel.show();
      break;
    case MENU_DFONT:
      dfontsel.show();
      break;
    case MENU_RFONT:
      reset_fonts();
      break;

    case MENU_ABOUT:
      aboutbox.show();
      break;
      
    default:
      cout << "Undefined menu action called" << endl;
      break;
    }
}

void GtkDasherWindow::reset_fonts()
{
  dasher_pane.set_edit_font( "", 0 );
  dasher_pane.set_dasher_font( "" );
}

void GtkDasherWindow::orientation( Opts::ScreenOrientations o )
{
  dasher_pane.orientation(o);
}

void GtkDasherWindow::reset()
{
  set_title("Dasher");
  dasher_pane.reset();
}

void GtkDasherWindow::save()
{
  dasher_pane.save();
}

void GtkDasherWindow::save_as()
{
  save_dialogue.show();
}

void GtkDasherWindow::open()
{
  ofilesel.show();
}

void GtkDasherWindow::exit()
{
  Gtk::Main::quit();
}

void GtkDasherWindow::select_all()
{
  dasher_pane.select_all();
}

void GtkDasherWindow::cut()
{
  cout << "In cut routine" << endl;

  dasher_pane.cut();
}

void GtkDasherWindow::copy()
{
  dasher_pane.copy();
}

void GtkDasherWindow::paste()
{
  dasher_pane.paste();
}

void GtkDasherWindow::copy_all()
{
  dasher_pane.copy_all();
}

int GtkDasherWindow::destroy_event_impl(GdkEventAny *event)
{
  exit();
  return( true );
}

int GtkDasherWindow::delete_event_impl(GdkEventAny *event)
{
  exit();
  return( false );
}

void GtkDasherWindow::toggle_slider()
{
  dasher_pane.show_speed_slider(  static_cast<CheckMenuItem *>( (*list_view)[3] )->get_active());
}

void GtkDasherWindow::toggle_fix()
{
  dasher_pane.fix_pane( static_cast<CheckMenuItem *>( (*list_view)[5] )->get_active());
}

void GtkDasherWindow::FixLayout(bool Value)
{
  if( Value != fix_pane )
    {
      fix_pane = Value;

      if( fix_pane )
	{
	  cout << "Fix layout" << endl;
	  // Need to actually fix the thing here

	  //	  vp.set_sensitive( false );

	  dasher_pane.fix( true );
	}
      else
	{
	  cout << "Unfix layout" << endl;
	  // Unfix the pane here

	  //	  vp.set_sensitive( true );
	  dasher_pane.fix( false );
	}

      static_cast<CheckMenuItem *>( (*list_view)[5] )->set_active( fix_pane );
    }
}

void GtkDasherWindow::ChangeMaxBitRate(double NewMaxBitRate)
{
  dasher_pane.move_slider( NewMaxBitRate );
}

void GtkDasherWindow::ShowToolbar( bool value )
{
  if( value != toolbar_shown )
    {
      toolbar_shown = value;

      if( toolbar_shown )
	toolbar.show();
      else
	toolbar.hide();

      static_cast<CheckMenuItem *>( (*list_view)[2] )->set_active( toolbar_shown );
    }
}

void GtkDasherWindow::ShowSpeedSlider( bool value )
{
  if( value != slider_shown )
    {
      slider_shown = value;
      dasher_pane.show_slider( slider_shown );

      static_cast<CheckMenuItem *>( (*list_view)[3] )->set_active( slider_shown );
    }
}

void GtkDasherWindow::TimeStampNewFiles(bool Value)
{
  if( timestamp != Value )
    {
      timestamp = Value;
      static_cast<CheckMenuItem *>( (*list_opts)[0] )->set_active( timestamp );
    }
}

void GtkDasherWindow::ChangeOrientation(Opts::ScreenOrientations Orientation)
{
  cout << "Change orientation " << Orientation  << endl;

  // FIXME - the following commented out code is broken (and generates signal propagation loops)

//    if( current_or != Orientation )
//      {
//        current_or = Orientation;
//        switch( current_or )
//  	{
//  	case Alphabet:
//  	  static_cast<CheckMenuItem *>( (*list_or)[0] )->set_active( true );
//  	  break;
//  	case LeftToRight:
//  	  static_cast<CheckMenuItem *>( (*list_or)[2] )->set_active( true );
//  	  break;
//  	case RightToLeft:
//  	  static_cast<CheckMenuItem *>( (*list_or)[3] )->set_active( true );
//  	  break;
//  	case TopToBottom:
//  	  static_cast<CheckMenuItem *>( (*list_or)[4] )->set_active( true );
//  	  break;
//  	case BottomToTop:
//  	  static_cast<CheckMenuItem *>( (*list_or)[5] )->set_active( true );
//  	  break;
//  	}
//      }


}


void GtkDasherWindow::CopyAllOnStop(bool Value)
{
  if( copy_all_on_pause != Value )
    {
      copy_all_on_pause = Value;

      static_cast<CheckMenuItem *>( (*list_opts)[1] )->set_active( copy_all_on_pause );
    }
}

void GtkDasherWindow::toggle_toolbar()
{
  cout << "toggle_toolbar ";

  cout << static_cast<CheckMenuItem *>( (*list_view)[2] )->get_active() << endl;

  dasher_pane.show_toolbar(static_cast<CheckMenuItem *>( (*list_view)[2] )->get_active());
}

void GtkDasherWindow::toggle_timestamp()
{
  dasher_pane.timestamp(  static_cast<CheckMenuItem *>( (*list_opts)[0] )->get_active());
}

void GtkDasherWindow::toggle_copy_all()
{
  //  copy_all_on_pause = !copy_all_on_pause;
  //  dasher_pane.copy_all_on_pause( copy_all_on_pause );

  dasher_pane.copy_all_on_pause( static_cast<CheckMenuItem *>( (*list_opts)[1] )->get_active() );
}
