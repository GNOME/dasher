#include "DasherSpi.h"

#ifdef GNOME_A11Y

#include <cspi/spi.h>
#include <string.h>

enum {
  NOT_INIT,
  INIT_SUCCESS,
  INIT_FAIL
}  status = NOT_INIT;

bool initSPI() {
  if (status == NOT_INIT) {
    status = (SPI_init()==2) ? INIT_FAIL : INIT_SUCCESS;
  }
  return (status==INIT_SUCCESS);
}

#else
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <gdk/gdkx.h>
#endif

void sendText(const char *szText) {
#ifdef GNOME_A11Y
  if(!initSPI())
    return;

  char *szNewText;
  szNewText = new char[strlen(szText) + 1];
  strcpy(szNewText, szText);
  
  SPI_generateKeyboardEvent(0, szNewText, SPI_KEY_STRING);
  
  delete[] szNewText;
#else
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
#endif
}
