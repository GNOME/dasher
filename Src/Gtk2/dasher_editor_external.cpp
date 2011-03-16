#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>

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

#include "dasher_editor_external.h"
#include "dasher_lock_dialogue.h"
#include "dasher_main.h"
#include "../DasherCore/ControlManager.h"
//#include "game_mode_helper.h"
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

typedef struct _DasherEditorExternalPrivate DasherEditorExternalPrivate;

struct _DasherEditorExternalPrivate {
  DasherMain *pDasherMain;
  DasherAppSettings *pAppSettings;
#ifdef GNOME_A11Y
  AccessibleEventListener *pFocusListener;
  AccessibleEventListener *pCaretListener;
  AccessibleText *pAccessibleText;
#endif
};

#define DASHER_EDITOR_EXTERNAL_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), DASHER_TYPE_EDITOR_EXTERNAL, DasherEditorExternalPrivate))

static DasherEditorExternal *g_pEditor;

G_DEFINE_TYPE(DasherEditorExternal, dasher_editor_external, DASHER_TYPE_EDITOR);


static void dasher_editor_external_finalize(GObject *pObject);

/* Private Method Declarations */

static gboolean dasher_editor_external_command(DasherEditor *pSelf, const gchar *szCommand);
static void dasher_editor_external_initialise(DasherEditor *pSelf, DasherAppSettings *pAppSettings, 
					      DasherMain *pDasherMain, GtkBuilder *pXML, 
					      const gchar *szFullPath);
static void dasher_editor_external_create_buffer(DasherEditor *pSelf);
static void dasher_editor_external_output(DasherEditor *pSelf, const gchar *szText, int iOffset);
static void dasher_editor_external_delete(DasherEditor *pSelf, int iLength, int iOffset);
static const gchar *dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength);
static gint dasher_editor_external_get_offset(DasherEditor *pSelf);
static void dasher_editor_external_grab_focus(DasherEditor *pSelf);
static const gchar *dasher_editor_external_get_all_text(DasherEditor *pSelf);
static const gchar *dasher_editor_external_get_new_text(DasherEditor *pSelf);
//from dasher_external_buffer:
//These were no-ops, so removing.
//static void dasher_editor_external_edit_convert(DasherEditor *pSelf);
//static void dasher_editor_external_edit_protect(DasherEditor *pSelf);
//TODO: Implement ctrl_move/ctrl_delete (ATM uses dasher_editor default i.e. returns get_offset)
#ifdef GNOME_A11Y
void dasher_editor_external_handle_focus(DasherEditorExternal *pSelf, const AccessibleEvent *pEvent);
void dasher_editor_external_handle_caret(DasherEditorExternal *pSelf, const AccessibleEvent *pEvent);

void focus_listener(const AccessibleEvent *pEvent, void *pUserData);
void caret_listener(const AccessibleEvent *pEvent, void *pUserData);
#endif

/* Callback Declarations */

extern "C" void external_context_changed_handler(GObject *pSource, gpointer pUserData);

/* Method Definitions */

static void 
dasher_editor_external_class_init(DasherEditorExternalClass *pClass) {

  g_type_class_add_private(pClass, sizeof(DasherEditorExternalPrivate));

  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_editor_external_finalize;

  DasherEditorClass *pParentClass = (DasherEditorClass *)pClass;

  pParentClass->initialise = dasher_editor_external_initialise;
  pParentClass->command = dasher_editor_external_command;
  pParentClass->output = dasher_editor_external_output;
  pParentClass->delete_text = dasher_editor_external_delete;
  pParentClass->get_context = dasher_editor_external_get_context;
  pParentClass->get_offset = dasher_editor_external_get_offset;
  pParentClass->grab_focus = dasher_editor_external_grab_focus;
  pParentClass->get_all_text = dasher_editor_external_get_all_text;
  pParentClass->get_new_text = dasher_editor_external_get_new_text;
}

static void 
dasher_editor_external_init(DasherEditorExternal *pDasherControl) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pDasherControl);

  pPrivate->pDasherMain = NULL;
  pPrivate->pAppSettings = NULL;
}

static void 
dasher_editor_external_finalize(GObject *pObject) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(DASHER_EDITOR_EXTERNAL(pObject));
#ifdef GNOME_A11Y
  SPI_deregisterGlobalEventListener(pPrivate->pFocusListener, "focus:");
  SPI_deregisterGlobalEventListener(pPrivate->pCaretListener, "object:text-caret-moved");
#endif
}

/* Public methods */
DasherEditorExternal *
dasher_editor_external_new() {
  // TODO: Handle singleton class a little more sensibly

  DasherEditorExternal *pDasherEditor;
  pDasherEditor =
        DASHER_EDITOR_EXTERNAL(g_object_new(DASHER_TYPE_EDITOR_EXTERNAL, NULL));
  g_pEditor = pDasherEditor;

  // TODO: Just add a simple placeholder widget here (eventually need
  // to figure out how to have a null (ie non-visible) widget.

  g_message("Creating new external editor");

  return pDasherEditor;
}

static void
dasher_editor_external_initialise(DasherEditor *pSelf, DasherAppSettings *pAppSettings, DasherMain *pDasherMain, GtkBuilder *pXML, const gchar *szFullPath) {

  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

  pPrivate->pAppSettings = pAppSettings;
  pPrivate->pDasherMain = pDasherMain;

  dasher_editor_external_create_buffer(pSelf);
}

static void 
dasher_editor_external_grab_focus(DasherEditor *pSelf) {
}

static void 
dasher_editor_external_create_buffer(DasherEditor *pSelf) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

#ifdef GNOME_A11Y

  if(!initSPI()) {
    g_message("Could not initialise SPI - accessibility options disabled");
  } else {
    pPrivate->pFocusListener = SPI_createAccessibleEventListener(focus_listener, pSelf);
    pPrivate->pCaretListener = SPI_createAccessibleEventListener(caret_listener, pSelf);
    
    // TODO: Need to deregister these on destruction
    
    if(pPrivate->pFocusListener && pPrivate->pCaretListener) {
      SPI_registerGlobalEventListener(pPrivate->pFocusListener, "focus:");
      SPI_registerGlobalEventListener(pPrivate->pCaretListener, "object:text-caret-moved");
    } else {
      g_message("Could not obtain an SPI listener");
    }
  }    

  pPrivate->pAccessibleText = 0;
#endif
}

static void 
dasher_editor_external_output(DasherEditor *pSelf, const gchar *szText, int iOffset) {
  sendText(szText);
}

static void 
dasher_editor_external_delete(DasherEditor *pSelf, int iLength, int iOffset) {
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

static const gchar *
dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength) {
#ifdef GNOME_A11Y
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);
  if(pPrivate->pAccessibleText)
    return AccessibleText_getText(pPrivate->pAccessibleText, iOffset, iOffset + iLength);
  else
    return "";
#else
  return "";
#endif
}

static gint 
dasher_editor_external_get_offset(DasherEditor *pSelf) {
#ifdef GNOME_A11Y
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);
  
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

static gboolean 
dasher_editor_external_command(DasherEditor *pSelf, const gchar *szCommand) {
  return FALSE;
}

static const gchar *
dasher_editor_external_get_all_text(DasherEditor *pSelf) { 
  return "";
}

static const gchar *
dasher_editor_external_get_new_text(DasherEditor *pSelf) { 
  return NULL;
}

/* Callback Functions */

#ifdef GNOME_A11Y
void dasher_editor_external_handle_focus(DasherEditorExternal *pSelf, const AccessibleEvent *pEvent) {
  DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

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

    //ACL: in old code, external_buffer emitted signal, for which the editor_external had
    // registered a callback, which then emitted the same/corresponding signal from the
    // editor_external; so by combining buffer into editor, seems we don't need any of that
    // and can just emit the signal from the editor directly. However, the callback also said:
    //TODO: plumb signal back into control
    // ...if that makes any sense?

    g_signal_emit_by_name(G_OBJECT(pSelf), "buffer_changed", G_OBJECT(pSelf), NULL, NULL);

  }

  Accessible_unref(accessible);
}

void dasher_editor_external_handle_caret(DasherEditorExternal *pSelf, const AccessibleEvent *pEvent) {
  //  g_message("Caret");
 DasherEditorExternalPrivate *pPrivate = DASHER_EDITOR_EXTERNAL_GET_PRIVATE(pSelf);

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

    //ACL: in old code, dasher_external_buffer emitted offset_changed signal,
    // picked up by editor_external's external_context_changed_handler,
    // which just emitted a context_changed signal from the editor_external.
    // So, just emit that directly:
    g_signal_emit_by_name(G_OBJECT(pSelf), "context_changed", G_OBJECT(pSelf), NULL, NULL);
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
  dasher_editor_external_handle_focus((DasherEditorExternal *)pUserData, pEvent);
}

void caret_listener(const AccessibleEvent *pEvent, void *pUserData) {
  dasher_editor_external_handle_caret((DasherEditorExternal *)pUserData, pEvent);
}

#endif
