#ifndef __dasher_main_h__
#define __dasher_main_h__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS
#define DASHER_TYPE_MAIN            (dasher_main_get_type())
#define DASHER_MAIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), DASHER_TYPE_MAIN, DasherMain ))
#define DASHER_MAIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DASHER_TYPE_MAIN, DasherMainClass ))
#define DASHER_IS_MAIN(obj)	    (G_TYPE_CHECK_INSTANCE_TYPE((obj), DASHER_TYPE_MAIN))
#define DASHER_IS_MAIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DASHER_TYPE_MAIN))
#define DASHER_MAIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DASHER_TYPE_MAIN, DasherMainClass))

typedef struct _DasherMain DasherMain;
typedef struct _DasherMainClass DasherMainClass;

struct _DasherMain {
  GObject parent;
};

struct _DasherMainClass {
  GObjectClass parent_class;

  void (*realized)(DasherMain *pDasherMain);
};


typedef struct _SCommandLine SCommandLine;

struct _SCommandLine {
  gchar *szFilename;
  gchar *szAppStyle;
  gchar *szOptions;
};

DasherMain *dasher_main_new(int *argc, char ***argv, SCommandLine *pCommandLine);
GType dasher_main_get_type();
//DasherEditorInternal *dasher_main_get_editor(DasherMain *pSelf);
void dasher_main_show(DasherMain *pSelf);
G_END_DECLS

#endif
