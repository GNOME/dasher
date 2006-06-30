#ifndef __game_mode_helper_h__
#define __game_mode_helper_h__

#include "GtkDasherControl.h"

#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS
#define TYPE_GAME_MODE_HELPER         (game_mode_helper_get_type())
#define GAME_MODE_HELPER(obj)         (GTK_CHECK_CAST((obj), TYPE_GAME_MODE_HELPER, GameModeHelper ))
#define GAME_MODE_HELPER_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), TYPE_GAME_MODE_HELPER, GameModeHelper ))
#define IS_GAME_MODE_HELPER(obj)		(GTK_CHECK_TYPE ((obj), TYPE_GAME_MODE_HELPER))
#define IS_GAME_MODE_HELPER_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), TYPE_GAME_MODE_HELPER))

typedef struct _GameModeHelper GameModeHelper;
typedef struct _GameModeHelperClass GameModeHelperClass;

struct _GameModeHelper {
  GObject parent;
  gpointer private_data;
};

struct _GameModeHelperClass {
  GtkDialogClass parent_class;
};

GObject *game_mode_helper_new(GtkDasherControl *pControl);
GType game_mode_helper_get_type();
G_END_DECLS

#endif
