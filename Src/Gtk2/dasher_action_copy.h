#ifndef __dasher_action_copy_h__
#define __dasher_action_copy_h__

#include "dasher_action.h"
#include "dasher_editor.h"

G_BEGIN_DECLS
#define TYPE_DASHER_ACTION_COPY           (dasher_action_copy_get_type())
#define DASHER_ACTION_COPY(obj)           (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_ACTION_COPY, DasherActionCopy ))
#define DASHER_ACTION_COPY_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_ACTION_COPY, DasherActionCopyClass ))
#define IS_DASHER_ACTION_COPY(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_ACTION_COPY))
#define IS_DASHER_ACTION_COPY_CLASS(klass)(G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_ACTION_COPY))
#define DASHER_ACTION_COPY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_ACTION_COPY, DasherActionCopyClass))

typedef struct _DasherActionCopy DasherActionCopy;
typedef struct _DasherActionCopyClass DasherActionCopyClass;

struct _DasherActionCopy {
  DasherAction parent;
  gpointer private_data;
};

struct _DasherActionCopyClass {
  DasherActionClass parent_class;
};

DasherActionCopy *dasher_action_copy_new(DasherEditor *pEditor);
GType dasher_action_copy_get_type();

G_END_DECLS

#endif
