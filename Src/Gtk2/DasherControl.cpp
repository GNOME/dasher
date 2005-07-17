#include "DasherControl.h"
#include "Timer.h"

#include "../DasherCore/DasherInterface.h"
#include "../DasherCore/Event.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include <sys/stat.h>

// 'Private' methods

extern "C" gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data);
extern "C" void realize_canvas(GtkWidget *widget, gpointer user_data);
extern "C" void speed_changed(GtkHScale *hscale, gpointer user_data);
extern "C" gint canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data);
extern "C" gint key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data);

// 'Private' member variable

// Callbacks for parameter notification and event handling

// FIXME - I'm sure a lot of these can be made local

char *system_data_dir;
char *user_data_dir;


// 'Public' member variables

gboolean paused=FALSE;
gboolean exiting=FALSE;
gboolean training=FALSE;
GAsyncQueue* trainqueue;
GtkWidget *train_dialog;
std::string alphabet;
ControlTree *controltree;
bool eyetrackermode=false;
gboolean direction=TRUE;
bool onedmode=false;
gint dasherwidth, dasherheight;
long yscale, mouseposstartdist=0;
gboolean mouseposstart;
gboolean firstbox=FALSE;
gboolean secondbox=FALSE;
time_t starttime=0;
time_t starttime2=0;
time_t dasherstarttime;


int oldx;
int oldy;




// CDasherControl class definitions

CDasherControl::CDasherControl( GtkVBox *pVBox, GtkDasherControl *pDasherControl ) {

  m_pDasherControl = pDasherControl;

  m_pInterface = new CDasherInterface; // FIXME - doing this will probably trigger drawing events, but we haven't created the canvas yet (delay setting screen until it's initialised?)


  // Set up the GTK widgets

  m_pVBox = GTK_WIDGET(pVBox);
  m_pCanvas = gtk_drawing_area_new();
  m_pSpeedFrame = gtk_frame_new( "Speed:" );
  m_pSpeedHScale = gtk_hscale_new_with_range( 0.1, 8.0, 0.1 );

  gtk_container_add( GTK_CONTAINER( m_pSpeedFrame ), m_pSpeedHScale );
  
  gtk_box_pack_start( GTK_BOX( m_pVBox ), m_pCanvas, TRUE, TRUE, 0 );
  gtk_box_pack_start( GTK_BOX( m_pVBox ), m_pSpeedFrame, FALSE, FALSE, 0 );
  
  gtk_widget_show_all( GTK_WIDGET( m_pVBox ) );

  // Connect callbacks - note that we need to implement the callbacks
  // as "C" style functions and pass this as user data so they can
  // call the object
  
  // Callback for the button presses to the canvas

  g_signal_connect( m_pCanvas, "button_press_event", G_CALLBACK(button_press_event), this );
  g_signal_connect( m_pSpeedHScale, "value-changed", G_CALLBACK(speed_changed), this );

  // Callback for the canvas being realised

  g_signal_connect_after( m_pCanvas, "realize", G_CALLBACK(realize_canvas), this );
  g_signal_connect( m_pCanvas, "configure_event", G_CALLBACK(canvas_configure_event), this );

  // We'll use the same call back for keyboard events from the canvas
  // and slider - maybe this isn't the right thing to do long term

  g_signal_connect( m_pCanvas, "key_press_event", G_CALLBACK(key_press_event), this );    
  g_signal_connect( m_pSpeedHScale, "key_press_event", G_CALLBACK(key_press_event), this );    
  
  GTK_WIDGET_SET_FLAGS( m_pCanvas, GTK_CAN_FOCUS );

  // Set up directory locations and so on.

  char *home_dir;

  home_dir = getenv( "HOME" );
  user_data_dir = new char[ strlen( home_dir ) + 10 ];
  sprintf( user_data_dir, "%s/.dasher/", home_dir );

  // Ooh, I love Unix
  mkdir(user_data_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );

  // PROGDATA is provided by the makefile
  system_data_dir = PROGDATA"/";

  m_pInterface->SetStringParameter( SP_SYSTEM_LOC, system_data_dir );
  m_pInterface->SetStringParameter( SP_USER_LOC, user_data_dir );

  // Add all available alphabets and colour schemes to the core
  scan_alphabet_files();
  scan_colour_files();

  // Start the dasher model

  m_pInterface->PauseAt(0,0); // FIXME - pause should be implicit (ie in core)?
  m_pInterface->Start(); // FIXME - should we hold off on this until later?

  // Tell the core that we handle edit events etc.

  m_pInterface->ChangeEdit( this );
  m_pInterface->SetSettingsUI( this );

  // Create an input device object - FIXME - should make this more flexible

  m_pMouseInput = new CDasherMouseInput;
  m_pInterface->SetInput( m_pMouseInput );

  // Create a pango cache

  m_pPangoCache = new CPangoCache;

  m_pScreen = NULL;

}

CDasherControl::~CDasherControl() {

  // Delete the interface

  if( m_pInterface != NULL ) {
    delete m_pInterface;
    m_pInterface = NULL;
  }

  // Delete the input device

  if( m_pMouseInput != NULL ) {
    delete m_pMouseInput;
    m_pMouseInput = NULL;
  }

}

void CDasherControl::RealizeCanvas() {
  // Start the timer loops as everything is set up
  // Aim for 20 frames per second

  g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE,50, timer_callback, this,NULL); 
}

void CDasherControl::CanvasConfigureEvent() {

  if( m_pScreen != NULL )
    delete m_pScreen;

  m_pScreen = new CCanvas( m_pCanvas, m_pPangoCache );
  m_pInterface->ChangeScreen( m_pScreen );
  
  m_pInterface->Redraw();
}

void CDasherControl::HandleParameterNotification( int iParameter ) {

  if( iParameter == SP_DASHER_FONT ) {
    m_pPangoCache->ChangeFont( m_pInterface->GetStringParameter( SP_DASHER_FONT ));
    m_pInterface->Redraw();
  }
  else if( iParameter == LP_MAX_BITRATE ) {
     gtk_range_set_value(GTK_RANGE( m_pSpeedHScale ), m_pInterface->GetLongParameter( LP_MAX_BITRATE )/100.0 );
  }
  else if( iParameter == BP_SHOW_SLIDER ) {
    if (m_pSpeedFrame != NULL) {
      if ( m_pInterface->GetBoolParameter( BP_SHOW_SLIDER )) {
	gtk_widget_show( GTK_WIDGET( m_pSpeedFrame ) );
	gtk_range_set_value( GTK_RANGE( m_pSpeedHScale ),  m_pInterface->GetLongParameter( LP_MAX_BITRATE )/100.0);
      } else {
	gtk_widget_hide( GTK_WIDGET( m_pSpeedFrame ) );
      }
    }
  }

  // Emit a dasher_changed signal to notify the application about changes.
  
  g_signal_emit_by_name( GTK_OBJECT( m_pDasherControl ), "dasher_changed", iParameter );
}


int CDasherControl::TimerEvent() {

  int x,y;

  gdk_window_get_pointer(m_pCanvas->window, &x, &y, NULL);
  m_pMouseInput->SetCoordinates( x, y );

  m_pInterface->NewFrame( get_time() );
 
  return 1;

  // See CVS for code which used to be here

}

void CDasherControl::HandleEvent( CEvent *pEvent ) {
  if( pEvent->m_iEventType == 2 ) {
    CEditEvent *pEditEvent( static_cast< CEditEvent* >( pEvent ));

    if( pEditEvent->m_iEditType == 1 ) {
      // Insert event
      g_signal_emit_by_name( GTK_OBJECT( m_pDasherControl ), "dasher_edit_insert",  pEditEvent->m_sText.c_str() );
    }
    else if( pEditEvent->m_iEditType == 2 ) {
      // Delete event
      g_signal_emit_by_name( GTK_OBJECT( m_pDasherControl ), "dasher_edit_delete", pEditEvent->m_sText.c_str() );
    }
  }
  else if( pEvent->m_iEventType == 4 ) {
    g_signal_emit_by_name( GTK_OBJECT( m_pDasherControl ), "dasher_start" );
  }
  else if( pEvent->m_iEventType == 5 ) {
    g_signal_emit_by_name( GTK_OBJECT( m_pDasherControl ), "dasher_stop" );
  }
};


gboolean CDasherControl::ButtonPressEvent(GdkEventButton *event) {

  // Take the focus if we click on the canvas

  GdkEventFocus *focusEvent = (GdkEventFocus *) g_malloc(sizeof(GdkEventFocus));
  gboolean *returnType;

  focusEvent->type = GDK_FOCUS_CHANGE;
  focusEvent->window = (GdkWindow *) m_pCanvas;
  focusEvent->send_event = FALSE;
  focusEvent->in = TRUE;

  gtk_widget_grab_focus(GTK_WIDGET(m_pCanvas));
  g_signal_emit_by_name(GTK_OBJECT(m_pCanvas), "focus_in_event", GTK_WIDGET(m_pCanvas), focusEvent, NULL, &returnType);


#ifdef WITH_GPE
  // GPE version requires the button to be held down rather than clicked
  if ((event->type != GDK_BUTTON_PRESS) && (event->type != GDK_BUTTON_RELEASE))
    return FALSE;
#else
  if ((event->type != GDK_BUTTON_PRESS) && (event->type != GDK_2BUTTON_PRESS))
    return FALSE;
#endif


  // FIXME - This shouldn't be in the control

  // CJB,  2003-08.  If we have a selection, replace it with the new input.
  // This code is duplicated in key_press_event.
  // if (gtk_text_buffer_get_selection_bounds (the_text_buffer, NULL, NULL))
  //  gtk_text_buffer_cut_clipboard(the_text_buffer, the_text_clipboard, TRUE);

  // FIXME - This should be moved into CDasherInterface

  // CJB.  2004-07.
  // One-button mode; change direction on mouse click.
  //  direction=!direction;

  if( m_pInterface->GetBoolParameter( BP_START_MOUSE ) ) {
    if( m_pInterface->GetBoolParameter( BP_DASHER_PAUSED ) ) 
      m_pInterface->Unpause( get_time() );
    else
      m_pInterface->PauseAt(0,0);
  }

  return false;

}


gint CDasherControl::KeyPressEvent( GdkEventKey *event ) {

  switch( event->keyval ) {
  case GDK_space:
    // FIXME - wrap this in a 'start/stop' method (and use for buttons as well as keys)

    if( m_pInterface->GetBoolParameter( BP_START_SPACE ) ) {
      if( m_pInterface->GetBoolParameter( BP_DASHER_PAUSED ) ) 
	m_pInterface->Unpause( get_time() );
      else
	m_pInterface->PauseAt(0,0);
    }
    break;
  }

}

// Method definitions

extern "C"
void realize_canvas(GtkWidget *widget, gpointer user_data) {
  // Just call the apropriate method in the object we are given
  static_cast< CDasherControl* >(user_data)->RealizeCanvas();
}


void CDasherControl::scan_alphabet_files()
{


  // FIXME - this shouldn't be here - the interface should do this itself upon construction

  // Hurrah for glib making this a nice easy thing to do
  // rather than the WORLD OF PAIN it would otherwise be
  GDir* directory;
  G_CONST_RETURN gchar* filename;
  GPatternSpec *alphabetglob;
  alphabetglob=g_pattern_spec_new("alphabet*xml");
  directory = g_dir_open(system_data_dir,0,NULL);

  std::cout << "looking for alphabet files in: " << system_data_dir << std::endl;

  while((filename=g_dir_read_name(directory))) {
    std::cout << "Filename is: " << filename << std::endl;

    if (alphabet_filter(filename, alphabetglob)) {

      std::cout << "adding" << std::endl;

      m_pInterface->AddAlphabetFilename(filename);
    }
 }

  directory = g_dir_open(user_data_dir,0,NULL);

  while((filename=g_dir_read_name(directory))) {
    if (alphabet_filter(filename, alphabetglob)) {
      m_pInterface->AddAlphabetFilename(filename);
    }
  }

  // FIXME - need to delete glob?

}

void CDasherControl::scan_colour_files()
{
  GDir* directory;
  G_CONST_RETURN gchar* filename;
  
  GPatternSpec *colourglob;
  colourglob=g_pattern_spec_new("colour*xml");

  directory = g_dir_open(system_data_dir,0,NULL);

  while((filename=g_dir_read_name(directory))) {
    if (colour_filter(filename, colourglob)) {
      m_pInterface->AddColourFilename(filename);
    }
  }

  directory = g_dir_open(user_data_dir,0,NULL);

  while((filename=g_dir_read_name(directory))) {
    if (colour_filter(filename, colourglob)) {
      m_pInterface->AddColourFilename(filename);
    }
  }

  // FIXME - need to delete glob?
}

void CDasherControl::SliderEvent() {
 m_pInterface->SetLongParameter( LP_MAX_BITRATE, GTK_RANGE( m_pSpeedHScale )->adjustment->value * 100 );
}


// FIXME - these two methods seem a bit pointless!

int CDasherControl::alphabet_filter(const gchar* filename, GPatternSpec *alphabetglob )
{
  return int(g_pattern_match_string(alphabetglob,filename));
}

int CDasherControl::colour_filter(const gchar* filename, GPatternSpec *colourglob )
{
  return int(g_pattern_match_string(colourglob,filename));
}


GArray *CDasherControl::GetAllowedValues( int iParameter ) {
 GArray *pRetVal( g_array_new( false, false, sizeof( gchar* )));

  std::vector< std::string > vList;

  switch( iParameter ) {
  case SP_ALPHABET_ID:
    m_pInterface->GetAlphabets( &vList );
    break;
  case SP_COLOUR_ID:
    m_pInterface->GetColours( &vList );
   break;
  }

  for( std::vector<std::string>::iterator it( vList.begin() ); it != vList.end(); ++it ) {
    // For internal glib reasons we need to make a variable and then
    // pass - we can use the iterator directly
    const char *pTemp(  it->c_str() );

    std::cout << "pTemp is " << pTemp << std::endl;

    g_array_append_val( pRetVal, pTemp );
  }

  return pRetVal;

}

// void dasher_control_toggle_pause() {
//   // Actually starts Dasher if we're already stopped. I'd rename it,
//   // but I derive perverse satisfaction from this.
//   if (paused == TRUE) {
//     dasher_unpause( get_time() );
//     paused = FALSE;
//     starttime=starttime2=0;
//     dasherstarttime=time(NULL);
//   } else {
//     // should really be the current position, but that's not necessarily anywhere near the canvas
//     // and it doesn't seem to actually matter in any case
//     dasher_pause(0,0);    
//     if (onedmode==true) {
//       // Don't immediately jump back to full speed if started in one-dimensional mode
//       // (I wonder how many of our heuristics violate the principle of least surprise?)
//       dasher_halt();
//     }
//     paused = TRUE;

//     // FIXME - REIMPLEMENT outside of control

// // #ifdef GNOME_SPEECH
// //     if (speakonstop==true)
// //       speak();
// // #endif
// //     if (stdoutpipe==true) {
// //       outputpipe();
// //     }

//     // FIXME - SUPERCEDED by Keith's logging stuff?

// //     if (timedata==TRUE) {
// //       // Just a debugging thing, output to the console
// //       printf(_("%d characters output in %ld seconds\n"),outputcharacters,
// // 	     time(NULL)-dasherstarttime);
// //       outputcharacters=0;
// //     }
//     if (mouseposstart==true) {
//       firstbox=true;
//       dasher_redraw();
//     }
//   }
// }

gpointer
change_alphabet(gpointer alph)
{
  // This is launched as a separate thread in order to let the main thread
  // carry on updating the training window
  // FIXME - REIMPLEMENT
  //dasher_set_parameter_string( STRING_ALPHABET, (gchar*)alph );
  //  g_free(alph);
  g_async_queue_push(trainqueue,(void *)1);
  g_thread_exit(NULL);
  return NULL;
}

// FIXME - this is part of the edit box, so deal with it outside of the control

// extern "C" gboolean
// button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
// {
  
//   // FIXME - REIMPLEMENT
//   //dasher_pause( (gint) event->x,(gint) event->y ); 
//   paused = TRUE;

//   return FALSE;
//}


// "C" style callbacks - these are here just because it's not possible
// (or at least not easy) to connect a callback directly to a C++
// method, so we pass a pointer to th object in the user_data field
// and use a wrapper function. Please do not put any functional code
// here.

extern "C" void 
speed_changed(GtkHScale *hscale, gpointer user_data) {
  static_cast< CDasherControl* >(user_data)->SliderEvent();
}

extern "C" gboolean
button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data) {
 return static_cast< CDasherControl* >(data)->ButtonPressEvent( event );
}

extern "C" gint
key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data) {
  return static_cast< CDasherControl* >(data)->KeyPressEvent( event );
  // FIXME - REIMPLEMENT all of this (where not obsolete)

//   int width,height;
//   if (event->type != GDK_KEY_PRESS)
//     return FALSE;

//   if (keyboardcontrol == false) {
//     // CJB,  2003-08.  If we have a selection, replace it with the new input.
//     // This code is duplicated in button_press_event. 
//     if (gtk_text_buffer_get_selection_bounds (the_text_buffer, NULL, NULL))
//       gtk_text_buffer_cut_clipboard(the_text_buffer,the_text_clipboard,TRUE);
//   }

//   // Eww. This stuff all needs to be rewritten at some point, anyway
//   switch (event->keyval) {    
//   case GDK_Up:
//     if (keyboardcontrol == true) {
//       if (cyclickeyboardmodeon==true) {
// 	int cycles=0;
// 	buttonnum++;
// 	buttonnum=buttonnum%9;
// 	while(buttons[buttonnum+1].x==0 && buttons[buttonnum+1].y==0 && cycles<10) {
// 	  buttonnum++;
// 	  buttonnum=buttonnum%9;
// 	  cycles++;
// 	}

// 	// FIXME - REIMPLEMENT

// // 	paused=false;
// // 	dasher_draw_go_to(int(buttons[buttonnum+1].x),int(buttons[buttonnum+1].y));
// // 	paused=true;
// 	return TRUE;
//       }
//       if (buttons[1].x==0 && buttons[1].y==0) {
// 	// FIXME - REIMPLEMENT

// // 	width = the_canvas->allocation.width;
// // 	height = the_canvas->allocation.height;
// // 	paused=false;
// // 	dasher_go_to((int)(0.70*width), (int)(0.20*height));
// // 	dasher_draw_go_to((int)(0.70*width), (int)(0.20*height));
// // 	paused=true;
// 	return TRUE;
//       } else {

// 	// FIXME - REIMPLEMENT

// // 	paused=false;
// // 	dasher_go_to(int(buttons[1].x),int(buttons[1].y));
// // 	dasher_draw_go_to(int(buttons[1].x),int(buttons[1].y));
// // 	paused=true;
// 	return TRUE;
//       }
//     }
//     break;
//   case GDK_Down:
//     if (keyboardcontrol == true) {
//       if (cyclickeyboardmodeon==true) {
// 	int cycles=0;
// 	buttonnum--;
// 	if (buttonnum<0) {
// 	  buttonnum=8;
// 	}
// 	buttonnum=buttonnum%9;
// 	while(buttons[buttonnum+1].x==0 && buttons[buttonnum+1].y==0 && cycles<10) {
// 	  buttonnum--;
// 	  if (buttonnum<0) {
// 	    buttonnum=8;
// 	  }
// 	  cycles++;
// 	}

// 	// FIXME - REIMPLEMENT

// // 	paused=false;
// // 	dasher_draw_go_to(int(buttons[buttonnum+1].x),int(buttons[buttonnum+1].y));
// // 	paused=true;
// 	return TRUE;
//       }
//       if (buttons[3].x==0 && buttons[3].y==0) {

// 	// FIXME - REIMPLEMENT

// // 	width = the_canvas->allocation.width;
// // 	height = the_canvas->allocation.height;
// // 	paused=false;
// // 	dasher_go_to((int)(0.70*width), (int)(0.80*height));
// // 	dasher_draw_go_to((int)(0.70*width), (int)(0.80*height));
// //	paused=true;
// 	return TRUE;
//       } else {

// 	// FIXME - REIMPLEMENT

// // 	paused=false;
// // 	dasher_go_to(int(buttons[3].x),int(buttons[3].y));
// // 	dasher_draw_go_to(int(buttons[3].x),int(buttons[3].y));
// // 	paused=true;
// 	return TRUE;
//       }
//     }
//     break;
//   case GDK_Left:
//     if (keyboardcontrol == true) {
//       if (cyclickeyboardmodeon==true) {
// 	return TRUE;
//       }
//       if (buttons[2].x==0 && buttons[2].y==0) {

// 	// FIXME - REIMPLEMENT
	
// // 	width = the_canvas->allocation.width;
// // 	height = the_canvas->allocation.height;
// // 	paused=false;
// // 	dasher_go_to((int)(0.25*width), (int)(0.50*height));
// // 	dasher_draw_go_to((int)(0.25*width), (int)(0.50*height));
// // 	paused=true;
// 	return TRUE;
//       } else {

// 	// FIXME - REIMPLEMENT

// // 	paused=false;
// // 	dasher_go_to(int(buttons[2].x),int(buttons[2].y));
// // 	dasher_draw_go_to(int(buttons[2].x),int(buttons[2].y));
// // 	paused=true;
// 	return TRUE;
//       }
//     }
//     break;
//   case GDK_Right:
//     if (keyboardcontrol==true) {
//       if (cyclickeyboardmodeon==true) {

	
// 	// FIXME - REIMPLEMENT

// // 	paused=false;
// //       	dasher_go_to(int(buttons[buttonnum+1].x),int(buttons[buttonnum+1].y));
// // 	dasher_draw_go_to(int(buttons[buttonnum+1].x),int(buttons[buttonnum+1].y));
// // 	paused=true;
// 	return TRUE;
//       }
//       if (buttons[4].x==0 && buttons[4].y==0) {
// 	return TRUE;
//       } else {

// 	// FIXME - REIMPLEMENT

// // 	paused=false;
// // 	dasher_go_to(int(buttons[4].x),int(buttons[4].y));
// // 	dasher_draw_go_to(int(buttons[4].x),int(buttons[4].y));
// // 	paused=true;
// 	return TRUE;
//       }
//     }
//     break;
//   case GDK_space:
//     if (startspace == TRUE) {
//       dasher_control_toggle_pause();      
//     }
//     return TRUE;
//     break;
//   case GDK_F12:
//     // If the user presses F12, recentre the cursor. Useful for one-dimensional use - 
//     // probably should be documented somewhere, really.

//     // FIXME - REIMPLEMENT

// //     int x, y;
// //     gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);
// //     XWarpPointer(gdk_x11_get_default_xdisplay(), 0, GDK_WINDOW_XID(the_canvas->window), 0, 0, 0, 0, the_canvas->allocation.width/2, the_canvas->allocation.height/2);
//     return TRUE;
//     break;
//   default:
//     return FALSE;
//   }  
//  return FALSE;
}

extern "C" gint
slider_key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data)
{

  // FIXME - REIMPLEMENT all of this (where not obsolete)

//   if (event->type != GDK_KEY_PRESS)
//     return FALSE;

//   switch (event->keyval) {
//   case GDK_space:
//     if (startspace == TRUE) {
//       dasher_control_toggle_pause();
//     }
//     return TRUE;
//     break;
//   case GDK_F12:

//     // FIXME - REIMPLEMENT and remove code duplication

// //     int x, y;
// //     gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);
// //     XWarpPointer(gdk_x11_get_default_xdisplay(), 0, GDK_WINDOW_XID(the_canvas->window), 0, 0, 0, 0, the_canvas->allocation.width/2, the_canvas->allocation.height/2);
//     return TRUE;
//     break;
//   default:
//     return FALSE;
//   }  
  return FALSE;
}

extern "C" gint
canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{

  std::cout << "canvas_configure_event: " << data << std::endl;

  ((CDasherControl *)data)->CanvasConfigureEvent();

  // Fixme - reimplement sanely

//   // If the canvas is resized, we need to regenerate all of the buffers
//   rebuild_buffer();

//   dasher_resize_canvas( the_canvas->allocation.width, the_canvas->allocation.height );

//   dasher_redraw();

//   if (setup==TRUE) {
//     // If we're set up and resized, then save those settings

//     // FIXME - Reimplement this

// //     dasher_set_parameter_int(INT_EDITHEIGHT,gtk_paned_get_position(GTK_PANED(glade_xml_get_widget(widgets,"vpaned1"))));
// //     gtk_window_get_size(GTK_WINDOW(window), &dasherwidth, &dasherheight);
// //     dasher_set_parameter_int(INT_SCREENHEIGHT, dasherheight);
// //     dasher_set_parameter_int(INT_SCREENWIDTH, dasherwidth);
//   }

  return FALSE;
}

