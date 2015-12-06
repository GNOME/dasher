#ifndef __dasher_main_h__
#define __dasher_main_h__

#include <glib.h>
#include <glib-object.h>
#include "Preferences.h"
#include "KeyboardHelper.h"
#include "DasherAppSettings.h"

G_BEGIN_DECLS
#define DASHER_TYPE_MAIN            (dasher_main_get_type())
#define DASHER_MAIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), DASHER_TYPE_MAIN, DasherMain ))
#define DASHER_MAIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DASHER_TYPE_MAIN, DasherMainClass ))
#define DASHER_IS_MAIN(obj)	    (G_TYPE_CHECK_INSTANCE_TYPE((obj), DASHER_TYPE_MAIN))
#define DASHER_IS_MAIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DASHER_TYPE_MAIN))
#define DASHER_MAIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DASHER_TYPE_MAIN, DasherMainClass))
// TODO: Make sure this is actually used
#define DASHER_MAIN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), DASHER_TYPE_MAIN, DasherMainPrivate))

typedef struct _DasherMain DasherMain;
typedef struct _DasherMainClass DasherMainClass;

struct _DasherMain {
  GObject parent;
};

struct _DasherMainClass {
  GObjectClass parent_class;

  void (*realized)(DasherMain *pDasherMain);
};

struct SCommandLine {
  gchar *szFilename = nullptr;
  gchar *szAppStyle = nullptr;
  gchar *szOptions = nullptr;
  gchar *szConfigFile = nullptr;
};

DasherMain *dasher_main_new(int *argc, char ***argv, SCommandLine *pCommandLine);
GType dasher_main_get_type();
void dasher_main_show(DasherMain *pSelf);
G_END_DECLS

#endif
