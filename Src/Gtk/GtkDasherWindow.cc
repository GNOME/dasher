// GtkDasherWindow.h
// (c) 2002 Philip Cowans

#include "GtkDasherWindow.h"
#include "GtkDasherPane.h"

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gtk--/window.h>
#include <gtk--/main.h>
#include <gtk--/dialog.h>
#include <gtk--/radiomenuitem.h>

#include <libintl.h>

#include <iostream>

using namespace SigC;

GtkDasherWindow::GtkDasherWindow()
  : dasher_pane( this ), main_vbox(false, 0), toolbar(GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_BOTH ), menubar(), Window(), save_dialogue(), aboutbox(), dfontsel(gettext("Dasher Font")), efontsel(gettext("Editing Font")), slider_shown( true ),toolbar_shown(true), ofilesel(gettext("Open")), afilesel(gettext("Append To File")), copy_all_on_pause( false ),ifilesel(gettext("Import Training Text")), button(gettext("Close")), label(gettext("Dasher - Version 3.0.2\nWeb: http://www.inference.phy.cam.ac.uk/dasher/\nemail: dasher@mrao.cam.ac.uk")), fix_pane( false ), timestamp( false ), current_or( Alphabet ), current_enc( UserDefault )
{
  set_title("Dasher");

  add(main_vbox);

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
    
    list_file.push_back(MenuElem(gettext("_New"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_NEW)));
    list_file.push_back(MenuElem(gettext("_Open..."),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_OPEN)));
    list_file.push_back(MenuElem(gettext("_Save"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_SAVE)));
    list_file.push_back(MenuElem(gettext("Sa_ve As..."),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_SAVEAS)));

       list_file.push_back(MenuElem(gettext("_Append to File..."),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						     MENU_APPEND)));
    list_file.push_back(SeparatorElem());
    list_file.push_back(MenuElem(gettext("_Import Training Text..."),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_IMPORT)));
    list_file.push_back(SeparatorElem());
    list_file.push_back(MenuElem(gettext("E_xit"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_EXIT)));

    Menu *menu_edit = new Menu();
    MenuList& list_edit = menu_edit->items();

    list_edit.push_back(MenuElem(gettext("Cut"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_CUT)));
    list_edit.push_back(MenuElem(gettext("Copy"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_COPY)));
    list_edit.push_back(MenuElem(gettext("Paste"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_PASTE)));
    list_edit.push_back(SeparatorElem());
    list_edit.push_back(MenuElem(gettext("Copy All"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_COPYALL)));
    list_edit.push_back(SeparatorElem());
    list_edit.push_back(MenuElem(gettext("Select All"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_SELECTALL)));

    Menu *menu_or = new Menu();
    list_or = &(menu_or->items());

    RadioMenuItem_Helpers::Group ogroup;

    list_or->push_back(RadioMenuElem(ogroup,gettext("Alphabet Default"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_ODEFAULT)));
    list_or->push_back(SeparatorElem());
    list_or->push_back(RadioMenuElem(ogroup,gettext("Left to Right"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_OLR)));
    list_or->push_back(RadioMenuElem(ogroup,gettext("Right to Left"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_ORL)));
    list_or->push_back(RadioMenuElem(ogroup,gettext("Top to Bottom"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_OTB)));
    list_or->push_back(RadioMenuElem(ogroup,gettext("Bottom to Top"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_OBT)));


    Menu *menu_view = new Menu();
    list_view = &(menu_view->items());
    
    list_view->push_back(MenuElem(gettext("Orientation"), *menu_or ));
    list_view->push_back(SeparatorElem());
    //    list_view.push_back(MenuElem(gettext("Toolbar"), *menu_tool));
    list_view->push_back(CheckMenuElem(gettext("Show Toolbar"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
								MENU_TOOLBAR)));
    list_view->push_back(CheckMenuElem(gettext("Speed Slider"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_SLIDER)));
    list_view->push_back(SeparatorElem());
    list_view->push_back(CheckMenuElem(gettext("Fix Layout"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_FIX)));

    //    static_cast<CheckMenuItem *>( (*list_view)[2] )->set_active( true );
    //    static_cast<CheckMenuItem *>( (*list_view)[3] )->set_active( true );

    static_cast<CheckMenuItem *>( (*list_view)[5] )->set_sensitive( false );

    Menu *menu_enc = new Menu();
    //MenuList &list_enc = menu_enc->items();
    list_enc = &(menu_enc->items());

    //	enum FileEncodingFormats {UserDefault=-1, AlphabetDefault=-2, UTF8=65001, UTF16LE=1200, UTF16BE=1201};
 RadioMenuItem_Helpers::Group egroup;
    list_enc->push_back(RadioMenuElem(egroup,gettext("User Default"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_EUDEFAULT)));

 list_enc->push_back(RadioMenuElem(egroup,gettext("Alphabet Default"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_EADEFAULT)));
   list_enc->push_back(SeparatorElem());
 list_enc->push_back(RadioMenuElem(egroup,gettext("Unicode UTF-8"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_EUTF8)));
 list_enc->push_back(RadioMenuElem(egroup,gettext("Unicode UTF-16 (LE)"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_EUTF16LE)));
 list_enc->push_back(RadioMenuElem(egroup,gettext("Unicode UTF-16 (BE)"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_EUTF16BE)));

    Menu *menu_opts = new Menu();
    list_opts = &menu_opts->items();

    CheckMenuElem timestamp_elem(gettext("Timestamp New Files"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
								  MENU_TIMESTAMP));
    

    //    static_cast<CheckMenuItem *>( timestamp_elem.get_child() )->set_active( true );

    list_opts->push_back(timestamp_elem);
    list_opts->push_back(CheckMenuElem(gettext("Copy All on Stop"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_CAOS)));
    list_opts->push_back(SeparatorElem());
    list_opts->push_back(MenuElem(gettext("Alphabet..."), bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_ALPHABET)));
    list_opts->push_back(MenuElem(gettext("File Encoding"), *menu_enc ));
    list_opts->push_back(SeparatorElem());

    Menu *menu_fsize = new Menu();
    list_fsize = &(menu_fsize->items());
    RadioMenuItem_Helpers::Group fsgroup;

    list_fsize->push_back(RadioMenuElem(fsgroup,gettext("Default size"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_FSDEFAULT)));
    list_fsize->push_back(RadioMenuElem(fsgroup,gettext("Large fonts"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_FSBIG)));
    list_fsize->push_back(RadioMenuElem(fsgroup,gettext("Very large fonts"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_FSVBIG)));

    list_opts->push_back(MenuElem(gettext("Font size"), *menu_fsize ));

    list_opts->push_back(MenuElem(gettext("Editing Font..."),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_EFONT)));
    list_opts->push_back(MenuElem(gettext("Dasher Font..."),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_DFONT)));
    list_opts->push_back(MenuElem(gettext("Reset Fonts"), bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_RFONT)));
    list_opts->push_back(CheckMenuElem(gettext("One Dimensional"), bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_1D)));
    list_opts->push_back(CheckMenuElem(gettext("Draw Mouse"),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_DRAWMOUSE)));

    //    static_cast<MenuItem *>( (*list_opts)[0] )->set_sensitive( false );
    // static_cast<MenuItem *>( (*list_opts)[4] )->set_sensitive( false );

    //    static_cast<CheckMenuItem *>( list_opts[0] )->set_active( false );
    Menu *menu_help = new Menu();
    MenuList &list_help = menu_help->items();

    list_help.push_back(MenuElem(gettext("About Dasher..."),bind<int>( slot(this,&GtkDasherWindow::menu_button_cb),
						      MENU_ABOUT)));


    // Create the menu bar
    //   Gtk+ does not have O(1) tail lookups so you should build menus 
    //   backwards whenever you plan to make lots of access to back().
    menubar.items().push_back(MenuElem(gettext("_File"),"<control>f",*menu_file));
    menubar.items().push_back(MenuElem(gettext("_Edit"),"<control>e",*menu_edit));
    menubar.items().push_back(MenuElem(gettext("_View"),"<control>v",*menu_view));
    menubar.items().push_back(MenuElem(gettext("_Options"),"<control>o",*menu_opts));
    menubar.items().push_back(MenuElem(gettext("_Help"),"<control>h",*menu_help));
  }

  //Item(const Icon& icon, const Gtk::string& str, const Callback& cb, const Gtk::string& tip=Gtk::string());

  {
        using namespace Gtk::Toolbar_Helpers;
	//using namespace Gnome::UI;

	//	Array<Info> a;	

	//    Gnome::UI::Icon new_icon("New");
	//    a.add( Gnome::UI::Item(new_icon, "New", bind<char*>( slot(this,&GtkDasherWindow::toolbar_button_cb), TB_NEW)));

	toolbar.tools().push_back(ButtonElem( gettext("New"),
					      bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
							   TB_NEW)));
     toolbar.tools().push_back(ButtonElem( gettext("Open"),
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_OPEN)));

     toolbar.tools().push_back(ButtonElem( gettext("Save"),
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_SAVE)));
     toolbar.tools().push_back(Space());
     toolbar.tools().push_back(ButtonElem( gettext("Cut"),
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_CUT)));
     toolbar.tools().push_back(ButtonElem( gettext("Copy"),
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_COPY)));
     toolbar.tools().push_back(ButtonElem( gettext("Copy All"),
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_COPY_ALL)));
     toolbar.tools().push_back(ButtonElem( gettext("Paste"),
 					  bind<int>( slot(this,&GtkDasherWindow::toolbar_button_cb),
 						       TB_PASTE)));
  }

  show_all();

  dasher_pane.set_settings_ui( this );

  save_dialogue.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::file_ok_sel));
  save_dialogue.delete_event.connect( SigC::slot(this, &GtkDasherWindow::file_close_sel) );

  dfontsel.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::dfont_ok_sel));
  dfontsel.get_cancel_button()->clicked.connect(slot(this, &GtkDasherWindow::dfont_cancel_sel));
  dfontsel.delete_event.connect( SigC::slot(this, &GtkDasherWindow::dfont_close_sel) );

  efontsel.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::efont_ok_sel));
  efontsel.get_cancel_button()->clicked.connect(slot(this, &GtkDasherWindow::efont_cancel_sel));
  efontsel.delete_event.connect( SigC::slot(this, &GtkDasherWindow::efont_close_sel) );


  ofilesel.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::ofile_ok_sel));
  ofilesel.get_cancel_button()->clicked.connect(slot(this, &GtkDasherWindow::ofile_cancel_sel));
  ofilesel.delete_event.connect( SigC::slot(this, &GtkDasherWindow::ofile_close_sel) );

  ifilesel.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::ifile_ok_sel));
  ifilesel.get_cancel_button()->clicked.connect(slot(this, &GtkDasherWindow::ifile_cancel_sel));
  ifilesel.delete_event.connect( SigC::slot(this, &GtkDasherWindow::ifile_close_sel) );


  afilesel.get_ok_button()->clicked.connect(slot(this, &GtkDasherWindow::afile_ok_sel));
  afilesel.get_cancel_button()->clicked.connect(slot(this, &GtkDasherWindow::afile_cancel_sel));
  afilesel.delete_event.connect( SigC::slot(this, &GtkDasherWindow::afile_close_sel) );
  
  dasher_pane.clear();

  refresh_title();

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
  
  SigC::Slot1<gint, GdkEventAny *> s = SigC::slot(this, &GtkDasherWindow::about_delete_sel);

  aboutbox.delete_event.connect(s);

}



GtkDasherWindow::~GtkDasherWindow()
{
}

void GtkDasherWindow::file_ok_sel()
{
  save_dialogue.current_filename = save_dialogue.get_filename();

  save_dialogue.set_filename(save_dialogue.current_filename.c_str());
  save_dialogue.hide();
  
  dasher_pane.save_as( save_dialogue.current_filename );

  refresh_title();
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
  dasher_pane.open(ofilesel.get_filename());
    
  ofilesel.hide();

  refresh_title();
}

void GtkDasherWindow::ofile_cancel_sel()
{
  ofilesel.hide();
}

void GtkDasherWindow::about_close_sel()
{
  aboutbox.hide();
}

gint GtkDasherWindow::about_delete_sel( GdkEventAny *e )
{
  aboutbox.hide();
  return( true );
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
      std::cerr << "Undefined toolbar action called" << std::endl;
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
    case MENU_DRAWMOUSE:
      toggle_drawmouse();
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

    case MENU_EUDEFAULT:
      encoding( UserDefault );
      break;
    case MENU_EADEFAULT:
      encoding( AlphabetDefault );
      break; 
    case MENU_EUTF8:
      encoding( UTF8 );
      break;
    case MENU_EUTF16LE:
      encoding( UTF16LE );
      break;
    case MENU_EUTF16BE:
      encoding( UTF16BE );
      break;

    case MENU_FSDEFAULT:
      dasher_pane.set_dasher_font_size(Normal);
      break;
    case MENU_FSBIG:
      dasher_pane.set_dasher_font_size(Big);
      break;
    case MENU_FSVBIG:
      dasher_pane.set_dasher_font_size(VBig);
      break;
    case MENU_1D:
      toggle_1d();
      break;

    case MENU_ABOUT:
      aboutbox.show();
      break;
      
    default:
      std::cerr << "Undefined menu action called" << std::endl;
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
  // The orientation menu handler. You will never find a more wretched
  // hive of scum and villainy. We must be cautious.

  // Or... because gtk is broken, we receive *two* events from this
  // menu for each click - one for the new selection (as it has been
  // selected) and one from the old selection (because it has been
  // deselected). We must only respond to the first of these, as we
  // will otherwise end up getting into a loop.

  bool isSelection( false );

  switch(o)
    {
    case Alphabet:
      isSelection = static_cast<RadioMenuItem *>( (*list_or)[0] )->get_active();
      break;
    case LeftToRight:
      isSelection = static_cast<RadioMenuItem *>( (*list_or)[2] )->get_active();
      break;
    case RightToLeft:
      isSelection = static_cast<RadioMenuItem *>( (*list_or)[3] )->get_active();
      break;
    case TopToBottom:
      isSelection = static_cast<RadioMenuItem *>( (*list_or)[4] )->get_active();
      break;
    case BottomToTop:
      isSelection = static_cast<RadioMenuItem *>( (*list_or)[5] )->get_active();
      break;
    default:
      std::cerr << "Something strange is happening here - we were asked to use an orientation that doesn't exist" << std::endl;
      break;
    }

  if( isSelection )
    dasher_pane.orientation(o);
}

void GtkDasherWindow::encoding( Opts::FileEncodingFormats e )
{

 bool isSelection( false );

  switch(e)
    {
    case UserDefault:
      isSelection = static_cast<RadioMenuItem *>( (*list_enc)[0] )->get_active();
      break;
    case AlphabetDefault:
      isSelection = static_cast<RadioMenuItem *>( (*list_enc)[1] )->get_active();
      break;
    case UTF8:
      isSelection = static_cast<RadioMenuItem *>( (*list_enc)[3] )->get_active();
      break;
    case UTF16LE:
      isSelection = static_cast<RadioMenuItem *>( (*list_enc)[4] )->get_active();
      break;
    case UTF16BE:
      isSelection = static_cast<RadioMenuItem *>( (*list_enc)[5] )->get_active();
      break;
    default:
      std::cerr << "Something strange is happening here - we were asked to use an encoding that doesn't exist" << std::endl;
      break;
    }

  if( isSelection )
    dasher_pane.encoding( e );
}


void GtkDasherWindow::reset()
{
 dasher_pane.reset();
 
 refresh_title();
} 

void GtkDasherWindow::refresh_title()
{
  // Updates the title to match the current filename according to the dasher pane

  string cfn;

  cfn = dasher_pane.get_current_filename();

  if( cfn == std::string() )
    set_title("Dasher");
  else
    {
      char tbuffer[256];
      snprintf( tbuffer, 256, "Dasher - %s", cfn.c_str());
      set_title(tbuffer);
    }

}

void GtkDasherWindow::save()
{
  dasher_pane.save();
}

void GtkDasherWindow::save_as()
{ 
  save_dialogue.set_filename( dasher_pane.get_current_filename() );
  save_dialogue.show();
}

void GtkDasherWindow::open()
{
  ofilesel.set_filename( dasher_pane.get_current_filename() );
  ofilesel.show();
}

void GtkDasherWindow::exit()
{
  if( dasher_pane.is_dirty() )
    std::cout << "Warning - exiting without saving changes" << std::endl;

  Gtk::Main::quit();
}

void GtkDasherWindow::select_all()
{
  dasher_pane.select_all();
}

void GtkDasherWindow::cut()
{
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
	dasher_pane.fix( true );
      else
	dasher_pane.fix( false );
      
      static_cast<CheckMenuItem *>( (*list_view)[5] )->set_active( fix_pane );
    }
}

void GtkDasherWindow::ChangeMaxBitRate(double NewMaxBitRate)
{
  dasher_pane.move_slider( NewMaxBitRate );
}

void GtkDasherWindow::ShowToolbar( bool value )
{
  toolbar_shown = value;
  
  if( toolbar_shown )
    toolbar.show();
  else
    toolbar.hide();
  
  static_cast<CheckMenuItem *>( (*list_view)[2] )->set_active( value );
}

void GtkDasherWindow::ShowSpeedSlider( bool value )
{
      slider_shown = value;
      dasher_pane.show_slider( slider_shown );

      static_cast<CheckMenuItem *>( (*list_view)[3] )->set_active( slider_shown );
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
  // FIXME - the following commented out code is broken (and generates signal propagation loops)

  if( current_or != Orientation )
     {
       current_or = Orientation;
       switch( current_or )
 	{
 	case Alphabet:
 	  static_cast<CheckMenuItem *>( (*list_or)[0] )->set_active( true );
 	  break;
 	case LeftToRight:
 	  static_cast<CheckMenuItem *>( (*list_or)[2] )->set_active( true );
 	  break;
 	case RightToLeft:
 	  static_cast<CheckMenuItem *>( (*list_or)[3] )->set_active( true );
 	  break;
 	case TopToBottom:
 	  static_cast<CheckMenuItem *>( (*list_or)[4] )->set_active( true );
 	  break;
 	case BottomToTop:
 	  static_cast<CheckMenuItem *>( (*list_or)[5] )->set_active( true );
 	  break;
 	}
     }


}

void GtkDasherWindow::SetFileEncoding(Opts::FileEncodingFormats Encoding)
{
  if( current_enc != Encoding )
     {
       current_enc = Encoding;
       switch( current_enc )
 	{
 	case UserDefault:
 	  static_cast<CheckMenuItem *>( (*list_enc)[0] )->set_active( true );
 	  break;
 	case AlphabetDefault:
 	  static_cast<CheckMenuItem *>( (*list_enc)[1] )->set_active( true );
 	  break;
 	case UTF8:
 	  static_cast<CheckMenuItem *>( (*list_enc)[3] )->set_active( true );
 	  break;
 	case UTF16LE:
 	  static_cast<CheckMenuItem *>( (*list_enc)[4] )->set_active( true );
 	  break;
 	case UTF16BE:
 	  static_cast<CheckMenuItem *>( (*list_enc)[5] )->set_active( true );
 	  break;
 	}
     }
}


void GtkDasherWindow::CopyAllOnStop(bool Value)
{
  if( copy_all_on_pause != Value )
    {
      copy_all_on_pause = Value;

      static_cast<CheckMenuItem *>( (*list_opts)[1] )->set_active( copy_all_on_pause );
    }
}

void GtkDasherWindow::SetDasherDimensions(bool Value)
{
  oned = Value;

  static_cast<CheckMenuItem *>( (*list_opts)[10] ) ->set_active( oned );
}

void GtkDasherWindow::DrawMouse(bool Value)
{
  draw_mouse = Value;

  static_cast<CheckMenuItem *>( (*list_opts)[11] )->set_active( draw_mouse );
}

void GtkDasherWindow::toggle_toolbar()
{
  dasher_pane.show_toolbar(static_cast<CheckMenuItem *>( (*list_view)[2] )->get_active());
}

void GtkDasherWindow::toggle_timestamp()
{
  dasher_pane.timestamp(  static_cast<CheckMenuItem *>( (*list_opts)[0] )->get_active());
}

void GtkDasherWindow::toggle_copy_all()
{
  dasher_pane.copy_all_on_pause( static_cast<CheckMenuItem *>( (*list_opts)[1] )->get_active() );
}

void GtkDasherWindow::toggle_1d()
{
  dasher_pane.set_dasher_dimensions( static_cast<CheckMenuItem *>( (*list_opts)[10] )->get_active());
}

void GtkDasherWindow::toggle_drawmouse()
{
  dasher_pane.drawmouse( static_cast<CheckMenuItem *>( (*list_opts)[11] )->get_active());
}

void GtkDasherWindow::ChangeAlphabet(const std::string& NewAlphabetID)
{
  dasher_pane.change_alphabet( NewAlphabetID );
}

gint GtkDasherWindow::file_close_sel( GdkEventAny *e )
{
  save_dialogue.hide();
  return( true );
};

gint GtkDasherWindow::dfont_close_sel( GdkEventAny *e )
{
  dfontsel.hide();
  return( true );
};

gint GtkDasherWindow::efont_close_sel( GdkEventAny *e )
{
  efontsel.hide();
  return( true );
};

gint GtkDasherWindow::ofile_close_sel( GdkEventAny *e )
{
  ofilesel.hide();
  return( true );
};

gint GtkDasherWindow::ifile_close_sel( GdkEventAny *e )
{
  ifilesel.hide();
  return( true );
};

gint GtkDasherWindow::afile_close_sel( GdkEventAny *e )
{
  afilesel.hide();
  return( true );
};

