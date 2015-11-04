#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <gdk/gdkx.h>

#include <X11/keysym.h>
#include <algorithm>

#include "dasher_editor_external.h"
#include "dasher_editor_private.h"
#include "dasher_lock_dialogue.h"
#include "dasher_main.h"
#include "../DasherCore/ControlManager.h"

void
dasher_editor_external_finalize(GObject *pSelf) {
}

void
dasher_editor_external_create_buffer(DasherEditor *pSelf) {
}

void
dasher_editor_external_toggle_direct_mode(DasherEditor *pSelf, bool direct) {
}

void
dasher_editor_external_output(DasherEditor *pSelf, const gchar *szText, int iOffset /* unused */) {
  glong numoutput;
  int numcodes;
  Display *dpy = gdk_x11_get_default_xdisplay();
  int min, max;
  KeySym *keysym;
  KeyCode code;  
  
  if(szText[0] == '\n') {
    // If it's a newline, we want to mimic an enter press rather than a raw newline
    code = XKeysymToKeycode(dpy, XK_Return);
    if(code != 0) {
      XTestFakeKeyEvent(dpy, code, True, CurrentTime);
      XSync(dpy, true);
      XTestFakeKeyEvent(dpy, code, False, CurrentTime);
      XSync(dpy, true);
    }
  }
  else {
    // gunichar is a 32 bit data type for UTF32 (aka UCS4) encoded unicodeX
    gunichar *wideoutput = g_utf8_to_ucs4(szText, -1, NULL, &numoutput, NULL);

    // Get the X server's keycode value range.
    XDisplayKeycodes(dpy, &min, &max);

    for(int i = 0; i < numoutput; i++) {

      // In the string of text that we're outputting, we need first to
      // convert each character to a X keysym, then fake a keycode
      // event that the X server will interpret as the desired keysym.
      //
      // For the first part, we take advantage of the fact that X
      // understands a regular mapping of Unicode code points <
      // 0x1000000 to keysym values: the keysym value is the Unicode
      // code point + 0x1000000.
      // (Ref. http://www.cl.cam.ac.uk/~mgk25/ucs/keysym2ucs.c)
      //
      // For the second part, we reprogram the X server's keycode to
      // keysym mapping so that a known keycode will generate each
      // keysym that we want.  This is actually a pretty horrible
      // hack, because who knows what other X programs might be
      // relying on the keycodes that we're reprogramming, or making
      // its own hacky changes to the keyboard mapping at the same
      // time?  But we try to reduce the hackiness by using the top
      // ten keycode values in the X server's range - in the hope that
      // those might not be being used by other applications.

      // Choose which keycode we're going to appropriate for this
      // iteration.
      code = (max - 1 - (i % 10));

      // If we've used the whole set of keycodes and are looping round
      // to use the first of them again, sleep a bit first to ensure
      // that the intended client has had time to see and process the
      // X events, before we go changing the keyboard mapping again.
      if (i && !(i % 10)) {
	usleep(200000);
      }

      if(wideoutput[i] < 0x01000000) {

	// Convert Unicode code point to keysym value.
	wideoutput[i] = wideoutput[i] | 0x01000000;

	// TODO: Please see
	// http://tronche.com/gui/x/xlib/input/keyboard-encoding.html
	// for an explanation as to why you sometimes get problems
	// with upper/lower case on some X displays I'm tempted to say
	// that the XTest stuff is just broken, and require some GNOME
	// a11y support for direct entry...

	// Returns the keyboard mapping for the current display - numcodes is the 
	keysym = XGetKeyboardMapping(dpy, min, max - min + 1, &numcodes);

	// Reprogramme the keyboard map to use the new keysym
	keysym[(code - min) * numcodes] = wideoutput[i];
	XChangeKeyboardMapping(dpy, min, numcodes, keysym, (max - min));
	XSync(dpy, true);

	// Delete the old keymap
	XFree(keysym);

	// There's no way whatsoever that this could ever possibly
	// be guaranteed to work (ever), but it does.
	if(code != 0) {
	  XTestFakeKeyEvent(dpy, code, True, CurrentTime);
	  XSync(dpy, true);
	  XTestFakeKeyEvent(dpy, code, False, CurrentTime);
	  XSync(dpy, true);
	}
      }
    }
    XSync(dpy, true);
    g_free(wideoutput);
  }
}

void
dasher_editor_external_delete(DasherEditor *pSelf, int iLength, int iOffset) {
  Display *dpy;
  dpy = gdk_x11_get_default_xdisplay();
  KeyCode code;
  code = XKeysymToKeycode(dpy, XK_BackSpace);
  for(int i = 0; i < iLength; i++) {
    XTestFakeKeyEvent(dpy, code, True, 0);
    XTestFakeKeyEvent(dpy, code, False, 0);
  }
  XFlush(dpy);
}

std::string dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength) {
  return "";
}

gint
dasher_editor_external_get_offset(DasherEditor *pSelf) {
  return 0;
}

void dasher_editor_external_move(DasherEditor *pSelf, bool bForwards, Dasher::CControlManager::EditDistance iDist) {
  // TODO: implement.
}

std::string dasher_editor_external_get_text_around_cursor(
    DasherEditor *pSelf, Dasher::CControlManager::EditDistance distance) {
  // TODO: implement.
  return "";
}
