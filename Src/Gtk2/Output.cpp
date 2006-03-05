// Routines providing output functionality to other applications

#include "../Common/Common.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#include <gdk/gdkx.h>

#include <iostream>

void SendText(const gchar *szText) { 
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
    gunichar *wideoutput = g_utf8_to_ucs4(szText, -1, NULL, &numoutput, NULL);


    for(int i = 0; i < numoutput; i++) {
      int modifiedkey = (modifiedkey + 1) % 10;
      // This gives us the magic X keysym
      wideoutput[i] = wideoutput[i] | 0x01000000;
      
      XDisplayKeycodes(dpy, &min, &max);
      keysym = XGetKeyboardMapping(dpy, min, max - min + 1, &numcodes);
      keysym[(max - min - modifiedkey - 1) * numcodes] = wideoutput[i];
      XChangeKeyboardMapping(dpy, min, numcodes, keysym, (max - min));
      XSync(dpy, true);
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
    XSync(dpy, true);
    g_free(wideoutput);
  }
}

void DeleteText(int iLength) {
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
