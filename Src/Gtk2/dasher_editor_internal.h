#ifndef __dasher_editor_internal_h__
#define __dasher_editor_internal_h__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "DasherAppSettings.h"
#include "dasher_editor.h"

/* Forward declaration */
typedef struct _DasherMain DasherMain;
struct _DasherMain;

typedef enum {
  CLIPBOARD_CUT,
  CLIPBOARD_COPY,
  CLIPBOARD_PASTE,
  CLIPBOARD_COPYALL,
  CLIPBOARD_SELECTALL,
  CLIPBOARD_CLEAR
} clipboard_action;

G_BEGIN_DECLS
#define DASHER_TYPE_EDITOR_INTERNAL            (dasher_editor_internal_get_type())
#define DASHER_EDITOR_INTERNAL(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), DASHER_TYPE_EDITOR_INTERNAL, DasherEditorInternal ))
#define DASHER_EDITOR_INTERNAL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), DASHER_TYPE_EDITOR_INTERNAL, DasherEditorInternalClass ))
#define DASHER_IS_EDITOR_INTERNAL(obj)	      (G_TYPE_CHECK_INSTANCE_TYPE((obj), DASHER_TYPE_EDITOR_INTERNAL))
#define DASHER_IS_EDITOR_INTERNAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), DASHER_TYPE_EDITOR_INTERNAL))
#define DASHER_EDITOR_INTERNAL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), DASHER_TYPE_EDITOR_INTERNAL, DasherEditorInternalClass))

typedef struct _DasherEditorInternal DasherEditorInternal;
typedef struct _DasherEditorInternalClass DasherEditorInternalClass;

struct _DasherEditorInternal {
  DasherEditor parent;
};

struct _DasherEditorInternalClass {
  DasherEditorClass parent_class;

  void (*filename_changed)(DasherEditorInternal *pDasherEditorInternal);
  void (*buffer_changed)(DasherEditorInternal *pDasherEditorInternal);
  void (*context_changed)(DasherEditorInternal *pDasherEditorInternal);
};

DasherEditorInternal *dasher_editor_internal_new();
void dasher_editor_internal_initialise(DasherEditorInternal *pSelf, DasherAppSettings *pAppSettings, DasherMain *pDasherMain, GtkBuilder *pXML, const gchar *szFullPath);

GType dasher_editor_internal_get_type();

G_END_DECLS

#endif
