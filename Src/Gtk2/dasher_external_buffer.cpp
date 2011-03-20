// TODO: Make build system conditional
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>

#ifdef GNOME_A11Y
#include <cspi/spi.h>
#else
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <gdk/gdkx.h>
#endif

#include <X11/keysym.h>
#include <algorithm>

#include "dasher_buffer_set.h"
#include "dasher_external_buffer.h"
#include "DasherSpi.h"

// TODO: Figure out if we need this stuff and re-implement

// X_HAVE_UTF8_STRING -> attempt to do keyboard mapping? (This won't work without extended keysyms being defined) - no or otherwise?

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
void dasher_external_buffer_insert(DasherExternalBuffer *pSelf, const gchar *szText, int iOffset);
void dasher_external_buffer_delete(DasherExternalBuffer *pSelf, int iLength, int iOffset);
void dasher_external_buffer_edit_convert(DasherExternalBuffer *pSelf);
void dasher_external_buffer_edit_protect(DasherExternalBuffer *pSelf);
gchar *dasher_external_buffer_get_context(DasherExternalBuffer *pSelf, gint iOffset, gint iLength);
void dasher_external_buffer_edit_move(DasherExternalBuffer *pSelf, int iDirection, int iDist);
void dasher_external_buffer_edit_delete(DasherExternalBuffer *pSelf, int iDirection, int iDist);
gint dasher_external_buffer_get_offset(DasherExternalBuffer *pSelf);

#ifdef GNOME_A11Y
void dasher_external_buffer_handle_focus(DasherExternalBuffer *pSelf, const AccessibleEvent *pEvent);
void dasher_external_buffer_handle_caret(DasherExternalBuffer *pSelf, const AccessibleEvent *pEvent);

void focus_listener(const AccessibleEvent *pEvent, void *pUserData);
void caret_listener(const AccessibleEvent *pEvent, void *pUserData);
#endif

typedef struct _DasherExternalBufferPrivate DasherExternalBufferPrivate;

struct _DasherExternalBufferPrivate {
#ifdef GNOME_A11Y
  AccessibleEventListener *pFocusListener;
  AccessibleEventListener *pCaretListener;
  AccessibleText *pAccessibleText;
#endif
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
  iface->insert_text = (void (*)(IDasherBufferSet *pSelf, const gchar *szText, int iOffset))dasher_external_buffer_insert;
  iface->delete_text = (void (*)(IDasherBufferSet *pSelf, gint iLength, int iOffset))dasher_external_buffer_delete;
  iface->get_context = (gchar *(*)(IDasherBufferSet *pSelf, gint iOffset, gint iLength))dasher_external_buffer_get_context;
  iface->edit_move = (void (*)(IDasherBufferSet *pSelf, gint iDirection, gint iDist))dasher_external_buffer_edit_move;
  iface->edit_delete = (void (*)(IDasherBufferSet *pSelf, gint iDirection, gint iDist))dasher_external_buffer_edit_delete;
  iface->edit_convert = (void (*)(IDasherBufferSet *pSelf))dasher_external_buffer_edit_convert;
  iface->edit_protect = (void (*)(IDasherBufferSet *pSelf))dasher_external_buffer_edit_protect;
  iface->get_offset = (gint (*)(IDasherBufferSet *pSelf))dasher_external_buffer_get_offset;
}

static void dasher_external_buffer_destroy(GObject *pObject) {
  DasherExternalBufferPrivate *pPrivate = (DasherExternalBufferPrivate *)(((DasherExternalBuffer *)pObject)->private_data);
#ifdef GNOME_A11Y
  SPI_deregisterGlobalEventListener(pPrivate->pFocusListener, "focus:");
  SPI_deregisterGlobalEventListener(pPrivate->pCaretListener, "object:text-caret-moved");
#endif

  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

DasherExternalBuffer *dasher_external_buffer_new() {
  DasherExternalBuffer *pDasherControl;
  pDasherControl = (DasherExternalBuffer *)(g_object_new(dasher_external_buffer_get_type(), NULL));

#ifdef GNOME_A11Y

  DasherExternalBufferPrivate *pPrivate = (DasherExternalBufferPrivate *)(pDasherControl->private_data);

  if(!initSPI()) {
    g_message("Could not initialise SPI - accessibility options disabled");
  }
  else {
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
  }    

  pPrivate->pAccessibleText = 0;
#endif

  return pDasherControl;
}

void dasher_external_buffer_insert(DasherExternalBuffer *pSelf, const gchar *szText, int iOffset) { 
  sendText(szText);
}

void dasher_external_buffer_delete(DasherExternalBuffer *pSelf, int iLength, int iOffset) {

#ifdef GNOME_A11Y
  if(!initSPI()) return;

  SPI_generateKeyboardEvent(XK_BackSpace, NULL, SPI_KEY_SYM);
#else
  Display *dpy;
  dpy = gdk_x11_get_default_xdisplay();
  KeyCode code;
  code = XKeysymToKeycode(dpy, XK_BackSpace);
  for(int i = 0; i < iLength; i++) {
    XTestFakeKeyEvent(dpy, code, True, 0);
    XTestFakeKeyEvent(dpy, code, False, 0);
  }
  XFlush(dpy);
#endif
}

void dasher_external_buffer_edit_move(DasherExternalBuffer *pSelf, int iDirection, int iDist) {
  // TODO: Implement
}

void dasher_external_buffer_edit_delete(DasherExternalBuffer *pSelf, int iDirection, int iDist) {
  // TODO: Implement
}

gchar *dasher_external_buffer_get_context(DasherExternalBuffer *pSelf, gint iOffset, gint iLength) {
#ifdef GNOME_A11Y
  DasherExternalBufferPrivate *pPrivate = (DasherExternalBufferPrivate *)(pSelf->private_data);

  if(pPrivate->pAccessibleText)
    return AccessibleText_getText(pPrivate->pAccessibleText, iOffset, iOffset + iLength);
  else
    return "";
#else
  return "";
#endif
}

gint 
dasher_external_buffer_get_offset(DasherExternalBuffer *pSelf) {
#ifdef GNOME_A11Y
  DasherExternalBufferPrivate *pPrivate = (DasherExternalBufferPrivate *)(pSelf->private_data);

  if(!pPrivate->pAccessibleText)
    return 0;
  if (AccessibleText_getNSelections(pPrivate->pAccessibleText)==0)
    return AccessibleText_getCaretOffset(pPrivate->pAccessibleText);
  long int start,end;
  AccessibleText_getSelection(pPrivate->pAccessibleText, 0, &start, &end);
  return std::min(start,end);
#else
  return 0;
#endif
}


void dasher_external_buffer_edit_convert(DasherExternalBuffer *pSelf) {
}

void dasher_external_buffer_edit_protect(DasherExternalBuffer *pSelf) {
}

#ifdef GNOME_A11Y
void dasher_external_buffer_handle_focus(DasherExternalBuffer *pSelf, const AccessibleEvent *pEvent) {
  DasherExternalBufferPrivate *pPrivate = (DasherExternalBufferPrivate *)(pSelf->private_data);

  //  g_message("Focus");
  
  if(pPrivate->pAccessibleText) {
    AccessibleText_unref(pPrivate->pAccessibleText);
    pPrivate->pAccessibleText = 0;
  }
  
  Accessible *accessible = pEvent->source;
  Accessible_ref(accessible);
  
  //  g_message("%s", Accessible_getName(accessible));
  // g_message("%s", Accessible_getRoleName(accessible));
  //g_message("%s", Accessible_getDescription(accessible));

  if(Accessible_isText(accessible) || Accessible_isEditableText(accessible)) {
    pPrivate->pAccessibleText = Accessible_getText(accessible);
    AccessibleText_ref(pPrivate->pAccessibleText);

//     g_iExpectedPosition = AccessibleText_getCaretOffset(pPrivate->pAccessibleText);
//     g_iOldPosition = g_iExpectedPosition;

    
    g_signal_emit_by_name(G_OBJECT(pSelf), "buffer_changed", G_OBJECT(pSelf), NULL, NULL);

  }

  Accessible_unref(accessible);
}

void dasher_external_buffer_handle_caret(DasherExternalBuffer *pSelf, const AccessibleEvent *pEvent) {
  //  g_message("Caret");
 DasherExternalBufferPrivate *pPrivate = (DasherExternalBufferPrivate *)(pSelf->private_data);

 //  g_message("Focus");
  
  if(pPrivate->pAccessibleText) {
    AccessibleText_unref(pPrivate->pAccessibleText);
    pPrivate->pAccessibleText = 0;
  }
  
  Accessible *accessible = pEvent->source;
  Accessible_ref(accessible);
  
  // g_message("%s", Accessible_getName(accessible));
  //g_message("%s", Accessible_getRoleName(accessible));
  //g_message("%s", Accessible_getDescription(accessible));

  if(Accessible_isText(accessible) || Accessible_isEditableText(accessible)) {
    pPrivate->pAccessibleText = Accessible_getText(accessible);
    AccessibleText_ref(pPrivate->pAccessibleText);

//     g_iExpectedPosition = AccessibleText_getCaretOffset(pPrivate->pAccessibleText);
//     g_iOldPosition = g_iExpectedPosition;

    
    g_signal_emit_by_name(G_OBJECT(pSelf), "offset_changed", G_OBJECT(pSelf), NULL, NULL);

  }

  Accessible_unref(accessible);
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
