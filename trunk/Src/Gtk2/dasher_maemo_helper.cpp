#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include "dasher_maemo_helper.h"

// TODO: Make member variable, give sensible names
GdkWindow *g_pFocusWindow;
GdkWindow *g_pRandomWindow;
Window g_FWindow;
Window g_FRandomWindow;

struct _DasherMaemoHelperPrivate {
  GtkWindow *pWindow;
};

typedef struct _DasherMaemoHelperPrivate DasherMaemoHelperPrivate;

// Private member functions
static void dasher_maemo_helper_class_init(DasherMaemoHelperClass *pClass);
static void dasher_maemo_helper_init(DasherMaemoHelper *pMain);
static void dasher_maemo_helper_destroy(GObject *pObject);
static GdkFilterReturn dasher_maemo_helper_handle_xevent(DasherMaemoHelper *pSelf, GdkXEvent *pXEvent);

// Private functions not part of the class
GdkFilterReturn peek_filter(GdkXEvent *pXEvent, GdkEvent *event, gpointer pUserData);

GType dasher_maemo_helper_get_type() {

  static GType dasher_maemo_helper_type = 0;

  if(!dasher_maemo_helper_type) {
    static const GTypeInfo dasher_maemo_helper_info = {
      sizeof(DasherMaemoHelperClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_maemo_helper_class_init,
      NULL,
      NULL,
      sizeof(DasherMaemoHelper),
      0,
      (GInstanceInitFunc) dasher_maemo_helper_init,
      NULL
    };

    dasher_maemo_helper_type = g_type_register_static(G_TYPE_OBJECT, "DasherMaemoHelper", &dasher_maemo_helper_info, static_cast < GTypeFlags > (0));
  }

  return dasher_maemo_helper_type;
}

static void dasher_maemo_helper_class_init(DasherMaemoHelperClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_maemo_helper_destroy;
}

static void dasher_maemo_helper_init(DasherMaemoHelper *pDasherControl) {
  pDasherControl->private_data = new DasherMaemoHelperPrivate;
}

static void dasher_maemo_helper_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

// Public methods

DasherMaemoHelper *dasher_maemo_helper_new(GtkWindow *pWindow) {
  DasherMaemoHelper *pDasherControl;
  pDasherControl = (DasherMaemoHelper *)(g_object_new(dasher_maemo_helper_get_type(), NULL));

  DasherMaemoHelperPrivate *pPrivate = (DasherMaemoHelperPrivate *)(pDasherControl->private_data);
  pPrivate->pWindow = pWindow;

  return pDasherControl;
}

void dasher_maemo_helper_setup_window(DasherMaemoHelper *pSelf) {
  DasherMaemoHelperPrivate *pPrivate = (DasherMaemoHelperPrivate *)(pSelf->private_data);
  gdk_window_add_filter(GDK_WINDOW(GTK_WIDGET(pPrivate->pWindow)->window), peek_filter, pSelf);
}

// TODO: Make better use of the GDK X11 wrapper functions
GdkFilterReturn dasher_maemo_helper_handle_xevent(DasherMaemoHelper *pSelf, GdkXEvent *pXEvent) {
  DasherMaemoHelperPrivate *pPrivate = (DasherMaemoHelperPrivate *)(pSelf->private_data);
  XEvent *xev = (XEvent *) pXEvent;

  if(xev->xany.type == ClientMessage) {
//     g_message("Atom: %s %d", XGetAtomName(xev->xany.display, xev->xclient.message_type), xev->xclient.format );
    
//     for(int i(0); i < 5; ++i) {
//       g_print("%x ", (unsigned int)(xev->xclient.data.l[i]));
//     }

//     g_print("\n");
    
    if((xev->xclient.data.l[3] == 0xc) || (xev->xclient.data.l[3] == 0xd)){
      gtk_widget_show(GTK_WIDGET(pPrivate->pWindow));
      g_FRandomWindow = (Window)xev->xclient.data.l[0];
      g_FWindow = (Window)xev->xclient.data.l[1];
      g_pFocusWindow = gdk_window_foreign_new(g_FWindow);
      g_pRandomWindow = gdk_window_foreign_new(g_FRandomWindow);
      gdk_window_set_transient_for(GDK_WINDOW(GTK_WIDGET(pPrivate->pWindow)->window), g_pFocusWindow);
    } 

    else if((xev->xclient.data.l[3] == 0x11) || (xev->xclient.data.l[3] == 0x12))  {
      GdkEventClient sMyEvent;
      
      sMyEvent.type = GDK_CLIENT_EVENT;
      sMyEvent.window = g_pRandomWindow;
      sMyEvent.send_event = true;
      sMyEvent.message_type = gdk_atom_intern("_HILDON_IM_COM", true);
      sMyEvent.data_format = 8; // I know this is wrong...
      sMyEvent.data.l[0] = g_FRandomWindow;
      sMyEvent.data.l[1] = 0x7;
      sMyEvent.data.l[2] = 0;
      sMyEvent.data.l[3] = 0;
      sMyEvent.data.l[4] = 0;
      
      gdk_event_send_client_message((GdkEvent *)(&sMyEvent), g_FRandomWindow); 
    }
    else if(xev->xclient.data.l[3] == 0xb) {
      gtk_widget_hide(GTK_WIDGET(pPrivate->pWindow));
    }
  }
  
  return GDK_FILTER_CONTINUE;
}

GdkFilterReturn peek_filter(GdkXEvent *pXEvent, GdkEvent *event, gpointer pUserData) {
  return dasher_maemo_helper_handle_xevent(DASHER_MAEMO_HELPER(pUserData), pXEvent);
}
