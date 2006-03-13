// Routines providing output functionality to other applications

#include "../Common/Common.h"
#include "accessibility.h"


#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#ifdef X_HAVE_UTF8_STRING
#include <X11/extensions/XTest.h>
#endif
#include <gdk/gdkx.h>

#include <iostream>


