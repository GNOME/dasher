#include "Timer.h"
#include "DasherControl.h"
#include "DasherControlPrivate.h"
#include "canvas.h"

#include <sys/time.h>

gint
timer_callback(gpointer data)
{

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
    int x;
    int y;

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
    gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);
    if (onedmode==true) {
      // In one dimensional mode, we want to scale the vertical component so that it's possible
      // for the amount of input to cover the entire canvas
      float scalefactor;
      float newy=y;
      gdk_drawable_get_size(the_canvas->window, &dasherwidth, &dasherheight);
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

    pMouseInput->SetCoordinates( x, y );

    dasher_tap_on( 0, 0, get_time() );
  }
  
  else {
    // If we're paused, then we still need to work out where the mouse is for two
    // reasons - start on mouse position, and to update the on-screen representation
    int x,y;
    gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);

     pMouseInput->SetCoordinates( x, y );
    
    if (onedmode==true) {
      float scalefactor;
      float newy=y;
      gdk_drawable_get_size(the_canvas->window, &dasherwidth, &dasherheight);
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
      dasher_draw_mouse_position(x,y,0);

    oldx = x;
    oldy = y;
    
    if (mouseposstart==true) {
      // The user must hold the mouse pointer inside the red box, then the yellow box
      // If the user fails to move to the yellow box, display the red box again and
      // start over
      dasherheight=the_canvas->allocation.height;
      gdk_window_get_pointer(the_canvas->window, &x, &y, NULL);

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
	    dasher_redraw();
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
	    dasher_control_toggle_pause(); // Yes, confusingly named
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
	  dasher_redraw();
	} else if (firstbox==true) {
	  // Start counting again if the mouse is outside the red box and the yellow
	  // box isn't being displayed
	  starttime=0;
	}
      }
    }
  }
  // need non-zero return value so timer repeats
  return 1;
}


long get_time() {
  // We need to provide a monotonic time source that ticks every millisecond
  long s_now;
  long ms_now;
  
  struct timeval tv;
  struct timezone tz;
  
  gettimeofday (&tv, &tz);
  
  s_now = tv.tv_sec;
  ms_now = tv.tv_usec / 1000;
  
  return (s_now * 1000 + ms_now);
}
