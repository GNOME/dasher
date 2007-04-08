#ifndef __dasher_maemo_helper_h__
#define __dasher_maemo_helper_h__

#include <glib.h>
#include <glib-object.h>
#include <X11/Xlib.h>

// TODO: sort this out
extern GdkWindow *g_pFocusWindow;
extern GdkWindow *g_pRandomWindow;
extern Window g_FWindow;
extern Window g_FRandomWindow;

G_BEGIN_DECLS
#define TYPE_DASHER_MAEMO_HELPER            (dasher_maemo_helper_get_type())
#define DASHER_MAEMO_HELPER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_MAEMO_HELPER, DasherMaemoHelper ))
#define DASHER_MAEMO_HELPER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_MAEMO_HELPER, DasherMaemoHelperClass ))
#define IS_DASHER_MAEMO_HELPER(obj)	    (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_MAEMO_HELPER))
#define IS_DASHER_MAEMO_HELPER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_MAEMO_HELPER))
#define DASHER_MAEMO_HELPER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_MAEMO_HELPER, DasherMaemoHelperClass))

typedef struct _DasherMaemoHelper DasherMaemoHelper;
typedef struct _DasherMaemoHelperClass DasherMaemoHelperClass;

struct _DasherMaemoHelper {
  GObject parent;
  gpointer private_data;
};

struct _DasherMaemoHelperClass {
  GObjectClass parent_class;
};

DasherMaemoHelper *dasher_maemo_helper_new(GtkWindow *pWindow);
GType dasher_maemo_helper_get_type();
void dasher_maemo_helper_setup_window(DasherMaemoHelper *pSelf);
G_END_DECLS

#endif
