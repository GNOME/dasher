
#include "DasherControl.h"
#include "DasherControlPrivate.h"

#include "Timer.h"
#include "canvas.h"

#include "../DasherCore/libdasher.h"
#include "../DasherCore/Event.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <sys/stat.h>

// 'Private' methods

void scan_alphabet_files();
void scan_colour_files();
int alphabet_filter(const gchar* filename);
int colour_filter(const gchar* filename);
void handle_parameter( int iParameter );
void control_handle_event( CEvent *pEvent );

extern "C" gboolean button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data);

// 'Private' member variable

// Callbacks for parameter notification and event handling

void (*fpParameterCallback)(int) = NULL;
void (*fpEventCallback)(CEvent *) = NULL;

// FIXME - I'm sure a lot of these can be made local

char *system_data_dir;
char *user_data_dir;
GPatternSpec *alphabetglob, *colourglob;

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
CDasherMouseInput *pMouseInput;


int oldx;
int oldy;

GtkScale *m_pSpeedHScale;
GtkFrame *m_pSpeedFrame;

// Method definitions

extern "C"
void realize_canvas(GtkWidget *widget, gpointer user_data) {



  initialise_canvas(360,360);

  dasher_resize_canvas( the_canvas->allocation.width, the_canvas->allocation.height );
  rebuild_buffer();

  dasher_redraw();

  // Now start the timer loops as everything is set up

  // Aim for 20 frames per second
  g_timeout_add_full(G_PRIORITY_DEFAULT_IDLE,50, timer_callback, NULL,NULL); 

}

GtkWidget *dasher_control_new() {
  // FIXME - The Dasher control should really create its own widgets, but for now they get created externally and passed here

  GtkVBox *pVBox;

  pVBox = GTK_VBOX( gtk_vbox_new( false, 0 ) );
  
  the_canvas = gtk_drawing_area_new();


  m_pSpeedFrame = GTK_FRAME( gtk_frame_new( "Speed:" ) );
  m_pSpeedHScale = GTK_SCALE( gtk_hscale_new_with_range( 0.1, 8.0, 0.1 ) );

  gtk_container_add( GTK_CONTAINER( m_pSpeedFrame ), GTK_WIDGET(m_pSpeedHScale) );
  
  gtk_box_pack_start( GTK_BOX( pVBox ), GTK_WIDGET( the_canvas ), TRUE, TRUE, 0 );
  gtk_box_pack_start( GTK_BOX( pVBox ), GTK_WIDGET( m_pSpeedFrame ), FALSE, FALSE, 0 );

  


//   gtk_widget_show( GTK_WIDGET( the_canvas ) );
//   gtk_widget_show( GTK_WIDGET( the_canvas ) );
//   gtk_widget_show( GTK_WIDGET( m_pSpeedFrame ) );
//   gtk_widget_show( GTK_WIDGET( m_pSpeedHScale ) );
  gtk_widget_show_all( GTK_WIDGET( pVBox ) );

  g_signal_connect( the_canvas, "button_press_event", G_CALLBACK(button_press_event), NULL );

  //  gtk_widget_add_events( the_canvas, GDK_REALIZE );
  g_signal_connect_after( the_canvas, "realize", G_CALLBACK(realize_canvas), NULL );


  paused=true;

//   the_canvas = pCanvas;

//   m_pSpeedHScale = pSpeedHScale;
//   m_pSpeedFrame = pSpeedFrame;

  dasher_set_blank_callback( blank_callback );
  dasher_set_display_callback( display_callback );
  dasher_set_colour_scheme_callback( receive_colour_scheme_callback );
  dasher_set_draw_rectangle_callback( draw_rectangle_callback );
  dasher_set_draw_polyline_callback( draw_polyline_callback );
  dasher_set_draw_colour_polyline_callback( draw_colour_polyline_callback );
  dasher_set_draw_colour_polygon_callback( draw_colour_polygon_callback );
  dasher_set_draw_text_callback( draw_text_callback );
  dasher_set_draw_text_string_callback( draw_text_string_callback );
  dasher_set_text_size_callback( text_size_callback );
  dasher_set_send_marker_callback( send_marker_callback );

  dasher_early_initialise();

  char *home_dir;

  home_dir = getenv( "HOME" );
  user_data_dir = new char[ strlen( home_dir ) + 10 ];
  sprintf( user_data_dir, "%s/.dasher/", home_dir );

  // Ooh, I love Unix
  mkdir(user_data_dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );

  // PROGDATA is provided by the makefile
  system_data_dir = PROGDATA"/";
  
  dasher_set_parameter_string( STRING_SYSTEMDIR, system_data_dir );
  dasher_set_parameter_string( STRING_USERDIR, user_data_dir );

  // Add all available alphabets and colour schemes to the core
  scan_alphabet_files();
  scan_colour_files();

  dasher_set_parameter_int( INT_LANGUAGEMODEL, 0 );
  dasher_set_parameter_int( INT_VIEW, 0 );
  
  dasher_start();
  dasher_redraw();

  dasher_late_initialise( 360, 360 ); // FIXME - shouldn't have fixed constants

  pMouseInput = new CDasherMouseInput;

  dasher_set_input( pMouseInput );





  // Realize the canvas now so that it can set up the buffers correctly
  //  gtk_widget_realize(the_canvas);



  // We support advanced colour mode
  dasher_set_parameter_bool( BOOL_COLOURMODE, true);

  dasher_set_parameter_callback( handle_parameter );
  dasher_set_event_callback( control_handle_event );

  // Initialise the model, and force a screen redraw.

  dasher_start();
  dasher_pause(0,0); // we start paused
  // dasher_redraw(); // Redrawing here will fail due to widget not being realized

  return GTK_WIDGET( pVBox );

}

void dasher_control_delete() {
  dasher_finalise();
}


void scan_alphabet_files()
{
  // Hurrah for glib making this a nice easy thing to do
  // rather than the WORLD OF PAIN it would otherwise be
  GDir* directory;
  G_CONST_RETURN gchar* filename;
  alphabetglob=g_pattern_spec_new("alphabet*xml");
  directory = g_dir_open(system_data_dir,0,NULL);

  while((filename=g_dir_read_name(directory))) {
    if (alphabet_filter(filename)) {
      add_alphabet_filename(filename);
    }
  }

  directory = g_dir_open(user_data_dir,0,NULL);

  while((filename=g_dir_read_name(directory))) {
    if (alphabet_filter(filename)) {
      add_alphabet_filename(filename);
    }
  }
}

void scan_colour_files()
{
  GDir* directory;
  G_CONST_RETURN gchar* filename;
  colourglob=g_pattern_spec_new("colour*xml");
  directory = g_dir_open(system_data_dir,0,NULL);

  while((filename=g_dir_read_name(directory))) {
    if (colour_filter(filename)) {
      add_colour_filename(filename);
    }
  }

  directory = g_dir_open(user_data_dir,0,NULL);

  while((filename=g_dir_read_name(directory))) {
    if (colour_filter(filename)) {
      add_colour_filename(filename);
    }
  }
}

int alphabet_filter(const gchar* filename)
{
  return int(g_pattern_match_string(alphabetglob,filename));
}

int colour_filter(const gchar* filename)
{
  return int(g_pattern_match_string(colourglob,filename));
}

void dasher_control_toggle_pause() {
  // Actually starts Dasher if we're already stopped. I'd rename it,
  // but I derive perverse satisfaction from this.
  if (paused == TRUE) {
    dasher_unpause( get_time() );
    paused = FALSE;
    starttime=starttime2=0;
    dasherstarttime=time(NULL);
  } else {
    // should really be the current position, but that's not necessarily anywhere near the canvas
    // and it doesn't seem to actually matter in any case
    dasher_pause(0,0);    
    if (onedmode==true) {
      // Don't immediately jump back to full speed if started in one-dimensional mode
      // (I wonder how many of our heuristics violate the principle of least surprise?)
      dasher_halt();
    }
    paused = TRUE;

    // FIXME - REIMPLEMENT outside of control

// #ifdef GNOME_SPEECH
//     if (speakonstop==true)
//       speak();
// #endif
//     if (stdoutpipe==true) {
//       outputpipe();
//     }

    // FIXME - SUPERCEDED by Keith's logging stuff?

//     if (timedata==TRUE) {
//       // Just a debugging thing, output to the console
//       printf(_("%d characters output in %ld seconds\n"),outputcharacters,
// 	     time(NULL)-dasherstarttime);
//       outputcharacters=0;
//     }
    if (mouseposstart==true) {
      firstbox=true;
      dasher_redraw();
    }
  }
}

void dasher_control_set_parameter_bool( int iParameter, bool bValue ) {
  dasher_get_interface()->SetBoolParameter( iParameter, bValue );
}

void dasher_control_set_parameter_long( int iParameter, long lValue ) {
 dasher_get_interface()->SetLongParameter( iParameter, lValue );
}

void dasher_control_set_parameter_string( int iParameter, const std::string &strValue ) { 
  dasher_get_interface()->SetStringParameter( iParameter, strValue );
}

bool dasher_control_get_parameter_bool( int iParameter ) {
  return dasher_get_interface()->GetBoolParameter( iParameter );
}

long dasher_control_get_parameter_long( int iParameter ) {
  return dasher_get_interface()->GetLongParameter( iParameter );
}

std::string dasher_control_get_parameter_string( int iParameter ) { 
  return dasher_get_interface()->GetStringParameter( iParameter );
}

/// Get the allowed values for a string parameter

int dasher_control_get_allowed_values( int iParameter, const char **list, int s ) {

  switch( iParameter ) {
  case SP_ALPHABET_ID:
    return dasher_get_alphabets( list, s );
    break;
  case SP_COLOUR_ID:
    return dasher_get_colours( list, s );
    break;
  default:
    return -1;
    break;
  }
}

void dasher_control_train_file( const char *cstrFilename ) {
  dasher_train_file( cstrFilename );
}

gpointer
change_alphabet(gpointer alph)
{
  // This is launched as a separate thread in order to let the main thread
  // carry on updating the training window
  dasher_set_parameter_string( STRING_ALPHABET, (gchar*)alph );
  //  g_free(alph);
  g_async_queue_push(trainqueue,(void *)1);
  g_thread_exit(NULL);
  return NULL;
}

extern "C" gboolean
button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{

  dasher_pause( (gint) event->x,(gint) event->y );
  paused = TRUE;

  return FALSE;
}


extern "C" void speed_changed(GtkHScale *hscale, gpointer user_data) {
  dasher_set_parameter_double( DOUBLE_MAXBITRATE, GTK_RANGE(hscale)->adjustment->value );
}

void dasher_control_set_parameter_callback( void (*_fpParameterCallback)(int) ) {
  fpParameterCallback = _fpParameterCallback;
}

void dasher_control_set_event_callback( void (*_fpEventCallback)( CEvent * ) ) {
  fpEventCallback = _fpEventCallback;
}

void handle_parameter( int iParameter ) {

  if( iParameter == SP_DASHER_FONT ) {
    //    dasherfont=dasher_control_get_parameter_string( SP_DASHER_FONT );
    set_canvas_font( dasher_control_get_parameter_string( SP_DASHER_FONT ) );
    //  if (dasherfont=="")
    //  dasherfont="DASHERFONT";
  }
  else if( iParameter == LP_MAX_BITRATE ) {
     gtk_range_set_value(GTK_RANGE( m_pSpeedHScale ), dasher_control_get_parameter_long( LP_MAX_BITRATE )/100.0 );
  }
  else if( iParameter == BP_SHOW_SLIDER ) {
    if (m_pSpeedFrame != NULL) {
      if ( dasher_control_get_parameter_bool( BP_SHOW_SLIDER ) ) {
	gtk_widget_show( GTK_WIDGET( m_pSpeedFrame ) );
	gtk_range_set_value( GTK_RANGE( m_pSpeedHScale ),  dasher_control_get_parameter_long( LP_MAX_BITRATE )/100.0);
      } else {
	gtk_widget_hide( GTK_WIDGET( m_pSpeedFrame ) );
      }
    }
  }

  // Check whether anything outside of the control wants to be notified.

  if( fpParameterCallback != NULL )
    fpParameterCallback( iParameter );
}

void control_handle_event( CEvent *pEvent ) {
  if( fpEventCallback != NULL )
    fpEventCallback( pEvent );
}

extern "C" gboolean
button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GdkEventFocus *focusEvent = (GdkEventFocus *) g_malloc(sizeof(GdkEventFocus));
  gboolean *returnType;

#ifdef WITH_GPE
  // GPE version requires the button to be held down rather than clicked
  if ((event->type != GDK_BUTTON_PRESS) && (event->type != GDK_BUTTON_RELEASE))
    return FALSE;
#else
  if ((event->type != GDK_BUTTON_PRESS) && (event->type != GDK_2BUTTON_PRESS))
    return FALSE;
#endif

  focusEvent->type = GDK_FOCUS_CHANGE;

  focusEvent->window = (GdkWindow *) the_canvas;
  focusEvent->send_event = FALSE;
  focusEvent->in = TRUE;

  gtk_widget_grab_focus(GTK_WIDGET(the_canvas));
  g_signal_emit_by_name(GTK_OBJECT(the_canvas), "focus_in_event", GTK_WIDGET(the_canvas), focusEvent, NULL, &returnType);

  // FIXME - This shouldn't be in the control

  // CJB,  2003-08.  If we have a selection, replace it with the new input.
  // This code is duplicated in key_press_event.
  // if (gtk_text_buffer_get_selection_bounds (the_text_buffer, NULL, NULL))
  //  gtk_text_buffer_cut_clipboard(the_text_buffer, the_text_clipboard, TRUE);

  // FIXME - This should be moved into CDasherInterface

  // CJB.  2004-07.
  // One-button mode; change direction on mouse click.
  //  direction=!direction;

  //  if (startleft == TRUE) {

  if( dasher_control_get_parameter_bool( BP_START_MOUSE ) )
    dasher_control_toggle_pause();

  return FALSE;
}


extern "C" gint
key_press_event (GtkWidget *widget, GdkEventKey *event, gpointer data)
{

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
  return FALSE;
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
