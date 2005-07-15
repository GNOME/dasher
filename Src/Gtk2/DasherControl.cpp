#include "DasherControl.h"
#include "Timer.h"

#include "../DasherCore/DasherInterface.h"
#include "../DasherCore/Event.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include <sys/stat.h>

// 'Private' methods

void handle_parameter( int iParameter );
void control_handle_event( CEvent *pEvent );

extern "C" gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data);
extern "C" void realize_canvas(GtkWidget *widget, gpointer user_data);
extern "C" void speed_changed(GtkHScale *hscale, gpointer user_data);
extern "C" gint canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer data);
extern "C" gint key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data);

// 'Private' member variable

// Callbacks for parameter notification and event handling

void (*fpParameterCallback)(int) = NULL;
void (*fpEventCallback)(CEvent *) = NULL;

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

CDasherControl::CDasherControl( GtkVBox *pVBox ) {

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
  g_signal_connect( m_pCanvas, "key_press_event", G_CALLBACK(key_press_event), this );    
  
  GTK_WIDGET_SET_FLAGS( m_pCanvas, GTK_CAN_FOCUS );

  paused=true; // FIXME - read directly from interface

  // Set up directory locations and so on.

  char *home_dir;

  home_dir = getenv( "HOME" );
  user_data_dir = new char[ strlen( home_dir ) + 10 ];
  sprintf( user_data_dir, "%s/.dasher/", home_dir );

  // Ooh, I love Unix
  mkdir(user_data_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );

  // PROGDATA is provided by the makefile
  system_data_dir = PROGDATA"/";

  // FIXME - Reimplement properly
  
//   dasher_set_parameter_string( STRING_SYSTEMDIR, system_data_dir );
//   dasher_set_parameter_string( STRING_USERDIR, user_data_dir );

//   // Add all available alphabets and colour schemes to the core
//   scan_alphabet_files();
//   scan_colour_files();

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

  // FIXME - reimplement

  //  initialise_canvas(360,360);
  
  // FIXME - I think we need a way of resizing the canvas without
  // having to rebuild the screen object (although I guess we rebuild
  // the buffers anyay, so maybe it's okay)

  // In any case, we shouldn't create with one size then immediately resize

  // dasher_resize_canvas( the_canvas->allocation.width, the_canvas->allocation.height );

  //  rebuild_buffer();


 if( m_pScreen != NULL )
    delete m_pScreen;

  m_pScreen = new CCanvas( m_pCanvas, m_pPangoCache );
  m_pInterface->ChangeScreen( m_pScreen );

  m_pInterface->Redraw();

  // Now start the timer loops as everything is set up

  // Aim for 20 frames per second
  g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE,50, timer_callback, this,NULL); 

}

void CDasherControl::CanvasConfigureEvent() {

  std::cout << "Canvas configure event" << std::endl;

  if( m_pScreen != NULL )
    delete m_pScreen;

  m_pScreen = new CCanvas( m_pCanvas, m_pPangoCache );
  m_pInterface->ChangeScreen( m_pScreen );

  
  
  m_pInterface->Redraw();
}

void CDasherControl::HandleParameterNotification( int iParameter ) {

  if( iParameter == SP_DASHER_FONT ) {

    // FIXME - reimplement

    //    dasherfont=dasher_control_get_parameter_string( SP_DASHER_FONT );
    //    set_canvas_font( dasher_control_get_parameter_string( SP_DASHER_FONT ) );
    //  if (dasherfont=="")
    //  dasherfont="DASHERFONT";
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

  // Check whether anything outside of the control wants to be notified.

//   if( fpParameterCallback != NULL )
//     fpParameterCallback( iParameter );
}


int CDasherControl::TimerEvent() {

  int x,y;

  gdk_window_get_pointer(m_pCanvas->window, &x, &y, NULL);
  m_pMouseInput->SetCoordinates( x, y );

  // FIXME - it would be much nicer if we just called a single
  // interface function here, rather than haing all of the stuff below
  // in this file

  if( m_pInterface->GetBoolParameter( BP_DASHER_PAUSED ) ) {
    m_pInterface->DrawMousePos(x,y,0); 
  }
  else {
    m_pInterface->TapOn( 0, 0, get_time() );
  }
  
  return 1;

  // FIXME - REIMPLEMENT

//   if (exiting==TRUE)
//     {
//       // Exit if we're called when Dasher is exiting
//       return 0;
//     }

  if (training==TRUE)
    {
      // Check if we're still training, and if so just return non-0 in order to get
      // called again
      if (g_async_queue_try_pop(trainqueue)==NULL) {
	return 1;
      } else {
	// Otherwise, we've just finished training - make everything work again
	training=FALSE;
	// Get rid of the training dialog and thread
	g_async_queue_unref(trainqueue);
	gtk_widget_hide(train_dialog);

	// FIXME - REIMPLEMENT

// 	// We need to do this again to get the configuration saved, as we
// 	// can't do gconf stuff from the other thread
// 	dasher_set_parameter_string( STRING_ALPHABET, alphabet.c_str() );
	
// 	// And call update_colours again now that we can do something useful
// 	update_colours();
// 	//	deletemenutree();
// 	// And making bonobo calls from another thread is likely to lead to
// 	// pain as well. It'd be nice to do this while training, but.
// 	add_control_tree(controltree);

// 	dasher_redraw();
      }
    }
  
    if (!paused) {
    
     if (eyetrackermode) {

       // FIXME - REIMPLEMENT

//         if (direction==TRUE) {
//             dasher_set_parameter_int(INT_ONEBUTTON, 150);
//         }
//         if (direction==FALSE) {
//             dasher_set_parameter_int(INT_ONEBUTTON, -150); 
//         }
    }

    // FIXME - REIMPLEMENT (shouldn't happen inside DasherControl??)
    
//     if (leavewindowpause==true) {
//       gtk_window_get_size(GTK_WINDOW(window), &dasherwidth, &dasherheight);
      
//       gdk_window_get_pointer(GTK_WIDGET(window)->window, &x, &y, NULL);
      
//       if (x>dasherwidth || x<0 || y>dasherheight || y<0) {
// 	// Don't do anything with the mouse position if we're outside the window. There's a
// 	// minor issue with this - if the user moves the cursor back in, Dasher will think
// 	// that lots of time has passed and jerk forwards until it recalculates the framerate
// 	return 1;
//       }
//     }
    gdk_window_get_pointer(m_pCanvas->window, &x, &y, NULL);
    if (onedmode==true) {
      // In one dimensional mode, we want to scale the vertical component so that it's possible
      // for the amount of input to cover the entire canvas
      float scalefactor;
      float newy=y;
      gdk_drawable_get_size(m_pCanvas->window, &dasherwidth, &dasherheight);
      if (yscale==0) {
	// For the magic value 0, we want the canvas size to reflect a full Y deflection
	// otherwise the user can't access the entire range. 2 is actually a slight
	// overestimate, but doing it properly would require thought and the benefit
	// is probably insufficient.
	scalefactor=2;
      } else {
	scalefactor=float(dasherheight)/float(yscale);
      }
      // Transform the real Y coordinate into a fudged Y coordinate
      newy-=dasherheight/2;
      newy=newy*scalefactor;
      newy+=dasherheight/2;
      y=int(newy);
    } 
    // And then provide the mouse position to the core. Of course, the core may then
    // do its own fudging.

    // FIXME - Reimmplement properly
    //    pMouseInput->SetCoordinates( x, y );

    //    dasher_tap_on( 0, 0, get_time() );
  }
  
  else {
    // If we're paused, then we still need to work out where the mouse is for two
    // reasons - start on mouse position, and to update the on-screen representation

    gdk_window_get_pointer(m_pCanvas->window, &x, &y, NULL);

    // FIXME - Reimplement properly
    //     pMouseInput->SetCoordinates( x, y );
    
    if (onedmode==true) {
      float scalefactor;
      float newy=y;
      gdk_drawable_get_size(m_pCanvas->window, &dasherwidth, &dasherheight);
      if (yscale==0) {
	scalefactor=2;
      } else {
	scalefactor=float(dasherheight)/float(yscale);
      }
      newy-=dasherheight/2;
      newy=newy*scalefactor;
      newy+=dasherheight/2;
      y=int(newy);
    } 

    if(( x != oldx ) || ( y != oldy )) // Only redraw if the mouse has actually moved
      m_pInterface->DrawMousePos(x,y,0);

    oldx = x;
    oldy = y;
    
    if (mouseposstart==true) {
      // The user must hold the mouse pointer inside the red box, then the yellow box
      // If the user fails to move to the yellow box, display the red box again and
      // start over
      dasherheight=m_pCanvas->allocation.height;
      gdk_window_get_pointer(m_pCanvas->window, &x, &y, NULL);

      // FIXME - REIMPLEMENT
      
//       if (firsttime==firstbox==secondbox==false) { // special case for Dasher 
// 	firstbox=true;                             // startup
// 	dasher_redraw();
//       }
      
      if (y>(dasherheight/2-mouseposstartdist-100) && y<(dasherheight/2-mouseposstartdist) && firstbox==true) {
	// The pointer is inside the red box
	if (starttime==0) {
	  // for the first time
	  starttime=time(NULL);
	} else {
	  // for some period of time
	  if ((time(NULL)-starttime)>2) {
	    // for long enough to trigger the yellow box
	    starttime=time(NULL);
	    secondbox=true;
	    firstbox=false;
	    // FIXME
	    //	    dasher_redraw();
	  }
	}
      } else if (y<(dasherheight/2+mouseposstartdist+100) && y>(dasherheight/2+mouseposstartdist) && secondbox==true) {      
	// inside the yellow box, and the yellow box has been displayed
	if (starttime2==0) {
	  // for the first time
	  starttime2=time(NULL);
	  starttime=0;
	} else {
	  // for some period of time
	  if ((time(NULL)-starttime2)>2) {
	    // for long enough to trigger starting Dasher
	    secondbox=false;

	    // FIXME - REIMPLEMENT

	    //	    dasher_control_toggle_pause(); // Yes, confusingly named
	  }
	}
      } else {
	if (secondbox==true && (starttime2>0 || (time(NULL)-starttime)>3)) {
	  // remove the yellow box if the user moves the pointer outside it
	  // or fails to select it sufficiently quickly
	  secondbox=false;
	  firstbox=true;
	  starttime2=0;
	  starttime=0;
	  // FIXME
	  //	  dasher_redraw();
	} else if (firstbox==true) {
	  // Start counting again if the mouse is outside the red box and the yellow
	  // box isn't being displayed
	  starttime=0;
	}
      }
    }
  }

    return 1;

}



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
  // Hurrah for glib making this a nice easy thing to do
  // rather than the WORLD OF PAIN it would otherwise be
  GDir* directory;
  G_CONST_RETURN gchar* filename;
  GPatternSpec *alphabetglob;
  alphabetglob=g_pattern_spec_new("alphabet*xml");
  directory = g_dir_open(system_data_dir,0,NULL);

  while((filename=g_dir_read_name(directory))) {
    if (alphabet_filter(filename, alphabetglob)) {

      // FIXME - REIMPLEMENT
      //    add_alphabet_filename(filename);
    }
 }

  directory = g_dir_open(user_data_dir,0,NULL);

  while((filename=g_dir_read_name(directory))) {
    if (alphabet_filter(filename, alphabetglob)) {
       // FIXME - REIMPLEMENT
      // add_alphabet_filename(filename);
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
      // FIXME - REIMPLEMENT
      // add_colour_filename(filename);
    }
  }

  directory = g_dir_open(user_data_dir,0,NULL);

  while((filename=g_dir_read_name(directory))) {
    if (colour_filter(filename, colourglob)) {
      // FIXME - REIMPLEMENT
      //  add_colour_filename(filename);
    }
  }

  // FIXME - need to delete glob?
}

void CDasherControl::SliderEvent() {
 m_pInterface->SetLongParameter( LP_MAX_BITRATE, GTK_RANGE( m_pSpeedHScale )->adjustment->value * 100 );
}


int CDasherControl::alphabet_filter(const gchar* filename, GPatternSpec *alphabetglob )
{
  return int(g_pattern_match_string(alphabetglob,filename));
}

int CDasherControl::colour_filter(const gchar* filename, GPatternSpec *colourglob )
{
  return int(g_pattern_match_string(colourglob,filename));
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

extern "C" gboolean
button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  
  // FIXME - REIMPLEMENT
  //dasher_pause( (gint) event->x,(gint) event->y ); 
  paused = TRUE;

  return FALSE;
}


extern "C" void speed_changed(GtkHScale *hscale, gpointer user_data) {
  static_cast< CDasherControl* >(user_data)->SliderEvent();
}

void dasher_control_set_parameter_callback( void (*_fpParameterCallback)(int) ) {
  fpParameterCallback = _fpParameterCallback;
}

void dasher_control_set_event_callback( void (*_fpEventCallback)( CEvent * ) ) {
  fpEventCallback = _fpEventCallback;
}


void control_handle_event( CEvent *pEvent ) {
  if( fpEventCallback != NULL )
    fpEventCallback( pEvent );
}

extern "C" gboolean
button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  // FIXME - probably want to pass some data about which button etc.

  std::cout << "In button press event - data is " << data << std::endl;

 return  static_cast< CDasherControl* >(data)->ButtonPressEvent( event );

}


extern "C" gint
key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
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
