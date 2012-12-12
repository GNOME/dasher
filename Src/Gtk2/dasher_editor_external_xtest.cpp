#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
dasher_editor_external_create_buffer(DasherEditor *pSelf) {
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
    // If it's a nreline, we want to mimic an enter press rather than a raw newline
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

    for(int i = 0; i < numoutput; i++) {

      // Erm - this makes no sense
      int modifiedkey = (i + 1) % 10;

      if(wideoutput[i] < 0x01000000) {

	// See http://wiki.x.org/wiki/KeySyms for the logic behind this
	// tranlation
	wideoutput[i] = wideoutput[i] | 0x01000000;

	// TODO: Please see
	// http://tronche.com/gui/x/xlib/input/keyboard-encoding.html
	// for an explanation as to why you sometimes get problems
	// with upper/lower case on some X displays I'm tempted to say
	// that the XTest stuff is just broken, and require some GNOME
	// a11y support for direct entry...
	
	XDisplayKeycodes(dpy, &min, &max);
	
	// Returns the keyboard mapping for the current display - numcodes is the 
	keysym = XGetKeyboardMapping(dpy, min, max - min + 1, &numcodes);
	
	// Reprogramme the keyboard map to use the new keysym
	keysym[(max - min - modifiedkey - 1) * numcodes] = wideoutput[i];
	XChangeKeyboardMapping(dpy, min, numcodes, keysym, (max - min));
	XSync(dpy, true);
	
	// Delete the old keymap
	XFree(keysym);
	// There's no way whatsoever that this could ever possibly
	// be guaranteed to work (ever), but it does.
	code = (max - modifiedkey - 1);
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

const gchar *
dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength) {
  return "";
}

gint
dasher_editor_external_get_offset(DasherEditor *pSelf) {
  return 0;
}
