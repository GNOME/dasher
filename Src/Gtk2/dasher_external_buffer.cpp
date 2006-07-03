// TODO: Make build system conditional
#include "config.h"
#ifdef GNOME_A11Y

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#ifdef X_HAVE_UTF8_STRING
#include <X11/extensions/XTest.h>
#endif
#include <gdk/gdkx.h>
#include <string.h>
#include <cspi/spi.h>

#include "dasher_buffer_set.h"
#include "dasher_external_buffer.h"

// TODO: Figure out if we need this stuff and re-implement

// Before...

//   int min, max;
//   Display *dpy = gdk_x11_get_default_xdisplay();

// #ifdef X_HAVE_UTF8_STRING
//   XDisplayKeycodes(dpy, &min, &max);
//   origkeymap = XGetKeyboardMapping(dpy, min, max - min + 1, &numcodes);
// #endif

// And after...

// #ifdef X_HAVE_UTF8_STRING
//   // We want to set the keymap back to whatever it was before,
//   // if that's possible
//   int min, max;
//   Display *dpy = gdk_x11_get_default_xdisplay();
//   XDisplayKeycodes(dpy, &min, &max);
//   XChangeKeyboardMapping(dpy, min, numcodes, origkeymap, (max - min));
// #endif

// ---

static void dasher_external_buffer_class_init(DasherExternalBufferClass *pClass);
static void dasher_external_buffer_init(DasherExternalBuffer *pAction);
static void dasher_external_buffer_destroy(GObject *pObject);
static void idasher_buffer_set_interface_init (gpointer g_iface, gpointer iface_data);
void dasher_external_buffer_insert(DasherExternalBuffer *pSelf, const gchar *szText);
void dasher_external_buffer_delete(DasherExternalBuffer *pSelf, int iLength);
void dasher_external_buffer_edit_convert(DasherExternalBuffer *pSelf);
void dasher_external_buffer_edit_protect(DasherExternalBuffer *pSelf);
void dasher_external_buffer_conversion_mode(DasherExternalBuffer *pSelf, gboolean bMode);
gchar *dasher_external_buffer_get_context(DasherExternalBuffer *pSelf, gint iMaxLength);
void dasher_external_buffer_edit_move(DasherExternalBuffer *pSelf, int iDirection, int iDist);
void dasher_external_buffer_edit_delete(DasherExternalBuffer *pSelf, int iDirection, int iDist);
void dasher_external_buffer_handle_focus(DasherExternalBuffer *pSelf, const AccessibleEvent *pEvent);
void dasher_external_buffer_handle_caret(DasherExternalBuffer *pSelf, const AccessibleEvent *pEvent);

void focus_listener(const AccessibleEvent *pEvent, void *pUserData);
void caret_listener(const AccessibleEvent *pEvent, void *pUserData);

typedef struct _DasherExternalBufferPrivate DasherExternalBufferPrivate;

struct _DasherExternalBufferPrivate {
  AccessibleEventListener *pFocusListener;
  AccessibleEventListener *pCaretListener;
  AccessibleText *pAccessibleText;
};

GType dasher_external_buffer_get_type() {

  static GType dasher_external_buffer_type = 0;

  if(!dasher_external_buffer_type) {
    static const GTypeInfo dasher_external_buffer_info = {
      sizeof(DasherExternalBufferClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_external_buffer_class_init,
      NULL,
      NULL,
      sizeof(DasherExternalBuffer),
      0,
      (GInstanceInitFunc) dasher_external_buffer_init,
      NULL
    };

    static const GInterfaceInfo idasher_buffer_set_info = {
      (GInterfaceInitFunc) idasher_buffer_set_interface_init,
      NULL,
      NULL 
    };
    
    dasher_external_buffer_type = g_type_register_static(G_TYPE_OBJECT, "DasherExternalBuffer", &dasher_external_buffer_info, static_cast < GTypeFlags > (0));

    g_type_add_interface_static (dasher_external_buffer_type,
                                 IDASHER_BUFFER_SET_TYPE,
                                 &idasher_buffer_set_info);
  }

  return dasher_external_buffer_type;
}

static void dasher_external_buffer_class_init(DasherExternalBufferClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_external_buffer_destroy;
}

static void dasher_external_buffer_init(DasherExternalBuffer *pDasherControl) {
  pDasherControl->private_data = new DasherExternalBufferPrivate;
}

static void idasher_buffer_set_interface_init (gpointer g_iface, gpointer iface_data) {
  IDasherBufferSetInterface *iface = (IDasherBufferSetInterface *)g_iface;
  iface->insert_text = (void (*)(IDasherBufferSet *pSelf, const gchar *szText))dasher_external_buffer_insert;
  iface->delete_text = (void (*)(IDasherBufferSet *pSelf, gint iLength))dasher_external_buffer_delete;
  iface->get_context = (gchar *(*)(IDasherBufferSet *pSelf, gint iMaxLength))dasher_external_buffer_get_context;
  iface->edit_move = (void (*)(IDasherBufferSet *pSelf, gint iDirection, gint iDist))dasher_external_buffer_edit_move;
  iface->edit_delete = (void (*)(IDasherBufferSet *pSelf, gint iDirection, gint iDist))dasher_external_buffer_edit_delete;
  iface->edit_convert = (void (*)(IDasherBufferSet *pSelf))dasher_external_buffer_edit_convert;
  iface->edit_protect = (void (*)(IDasherBufferSet *pSelf))dasher_external_buffer_edit_protect;
  iface->conversion_mode = (void (*)(IDasherBufferSet *pSelf, gboolean bMode))dasher_external_buffer_conversion_mode;
}

static void dasher_external_buffer_destroy(GObject *pObject) {
  g_message("In destructor");
  DasherExternalBufferPrivate *pPrivate = (DasherExternalBufferPrivate *)(((DasherExternalBuffer *)pObject)->private_data);

  SPI_deregisterGlobalEventListener(pPrivate->pFocusListener, "focus:");
  SPI_deregisterGlobalEventListener(pPrivate->pCaretListener, "object:text-caret-moved");

  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

DasherExternalBuffer *dasher_external_buffer_new() {
  g_message("creating buffer");
  DasherExternalBuffer *pDasherControl;
  pDasherControl = (DasherExternalBuffer *)(g_object_new(dasher_external_buffer_get_type(), NULL));

  DasherExternalBufferPrivate *pPrivate = (DasherExternalBufferPrivate *)(pDasherControl->private_data);
  pPrivate->pFocusListener = SPI_createAccessibleEventListener(focus_listener, pDasherControl);
  pPrivate->pCaretListener = SPI_createAccessibleEventListener(caret_listener, pDasherControl);

  // TODO: Need to deregister these on destruction

  if(pPrivate->pFocusListener && pPrivate->pCaretListener) {
    SPI_registerGlobalEventListener(pPrivate->pFocusListener, "focus:");
    SPI_registerGlobalEventListener(pPrivate->pCaretListener, "object:text-caret-moved");
  }
  else {
    g_message("Could not obtain an SPI listener");
  }

  pPrivate->pAccessibleText = 0;

  return pDasherControl;
}

void dasher_external_buffer_insert(DasherExternalBuffer *pSelf, const gchar *szText) { 
  char *szNewText;
  szNewText = new char[strlen(szText) + 1];
  strcpy(szNewText, szText);
  
  SPI_generateKeyboardEvent(0, szNewText, SPI_KEY_STRING);
  
  delete[] szNewText;
  return;

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

void dasher_external_buffer_delete(DasherExternalBuffer *pSelf, int iLength) {
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

void dasher_external_buffer_edit_move(DasherExternalBuffer *pSelf, int iDirection, int iDist) {
}

void dasher_external_buffer_edit_delete(DasherExternalBuffer *pSelf, int iDirection, int iDist) {
}

gchar *dasher_external_buffer_get_context(DasherExternalBuffer *pSelf, gint iMaxLength) {
  DasherExternalBufferPrivate *pPrivate = (DasherExternalBufferPrivate *)(pSelf->private_data);

  if(pPrivate->pAccessibleText) {
    int iOffset(AccessibleText_getCaretOffset(pPrivate->pAccessibleText));
    int iStart(iOffset - iMaxLength);
      
    if(iStart < 0 )
      iStart = 0;

    return AccessibleText_getText(pPrivate->pAccessibleText, iOffset-iMaxLength, iOffset);
  }
  else {
    return 0;
  }
}

void dasher_external_buffer_handle_focus(DasherExternalBuffer *pSelf, const AccessibleEvent *pEvent) {
  DasherExternalBufferPrivate *pPrivate = (DasherExternalBufferPrivate *)(pSelf->private_data);
  
  if(pPrivate->pAccessibleText) {
    AccessibleText_unref(pPrivate->pAccessibleText);
    pPrivate->pAccessibleText = 0;
  }
  
  Accessible *accessible = pEvent->source;
  Accessible_ref(accessible);

  if(Accessible_isText(accessible) || Accessible_isEditableText(accessible)) {
    pPrivate->pAccessibleText = Accessible_getText(accessible);
    AccessibleText_ref(pPrivate->pAccessibleText);

//     g_iExpectedPosition = AccessibleText_getCaretOffset(pPrivate->pAccessibleText);
//     g_iOldPosition = g_iExpectedPosition;

  g_signal_emit(pSelf, idasher_buffer_set_signals[CONTEXT_CHANGED], 0, NULL);

  }

  Accessible_unref(accessible);
}

void dasher_external_buffer_edit_convert(DasherExternalBuffer *pSelf) {
}

void dasher_external_buffer_edit_protect(DasherExternalBuffer *pSelf) {
}

void dasher_external_buffer_conversion_mode(DasherExternalBuffer *pSelf, gboolean bMode) {
}

void dasher_external_buffer_handle_caret(DasherExternalBuffer *pSelf, const AccessibleEvent *pEvent) {
//   Accessible_ref(event->source);

//   AccessibleStateSet *pAStateSet = Accessible_getStateSet(event->source);
//   AccessibleStateSet_ref(pAStateSet);

//   if(!AccessibleStateSet_contains(pAStateSet, SPI_STATE_FOCUSED)) {
//     return;
//   }

//   AccessibleStateSet_unref(pAStateSet);
//   Accessible_unref(event->source);
  
//   if(g_pAccessibleText) {
//     int iActualPosition = event->detail1;

//     std::cout << "Expected: " << g_iExpectedPosition << " Actual: " << iActualPosition << std::endl;

//     if((g_iExpectedPosition - iActualPosition) * (g_iOldPosition - iActualPosition) > 0) {
//       gtk_dasher_control_invalidate_context(GTK_DASHER_CONTROL(pDasherWidget));
//       g_iExpectedPosition = iActualPosition;
//     }

//     g_iOldPosition = iActualPosition;
//   }

}

void focus_listener(const AccessibleEvent *pEvent, void *pUserData) {
  dasher_external_buffer_handle_focus((DasherExternalBuffer *)pUserData, pEvent);
}

void caret_listener(const AccessibleEvent *pEvent, void *pUserData) {
  dasher_external_buffer_handle_caret((DasherExternalBuffer *)pUserData, pEvent);
}

#endif
