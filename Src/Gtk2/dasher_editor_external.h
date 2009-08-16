#ifndef __dasher_editor_external_h__
#define __dasher_editor_external_h__

#include "dasher_editor.h"

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS
#define DASHER_TYPE_EDITOR_EXTERNAL            (dasher_editor_external_get_type())
#define DASHER_EDITOR_EXTERNAL(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), DASHER_TYPE_EDITOR_EXTERNAL, DasherEditorExternal ))
#define DASHER_EDITOR_EXTERNAL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DASHER_TYPE_EDITOR_EXTERNAL, DasherEditorExternalClass ))
#define IS_DASHER_EDITOR_EXTERNAL(obj)	       (G_TYPE_CHECK_INSTANCE_TYPE((obj), DASHER_TYPE_EDITOR_EXTERNAL))
#define IS_DASHER_EDITOR_EXTERNAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DASHER_TYPE_EDITOR_EXTERNAL))
#define DASHER_EDITOR_EXTERNAL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DASHER_TYPE_EDITOR_EXTERNAL, DasherEditorExternalClass))

typedef struct _DasherEditorExternal DasherEditorExternal;
typedef struct _DasherEditorExternalClass DasherEditorExternalClass;

struct _DasherEditorExternal {
  DasherEditor parent;
};

struct _DasherEditorExternalClass {
  DasherEditorClass parent_class;

  void (*filename_changed)(DasherEditorExternal *pDasherEditorExternal);
  void (*buffer_changed)(DasherEditorExternal *pDasherEditorExternal);
  void (*context_changed)(DasherEditorExternal *pDasherEditorExternal);
};

DasherEditorExternal *dasher_editor_external_new();

GType dasher_editor_external_get_type();
G_END_DECLS

#endif

