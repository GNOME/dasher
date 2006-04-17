#ifndef __dasher_internal_buffer_h__
#define __dasher_internal_buffer_h__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS
#define TYPE_DASHER_INTERNAL_BUFFER            (dasher_internal_buffer_get_type())
#define DASHER_INTERNAL_BUFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_INTERNAL_BUFFER, DasherInternalBuffer ))
#define DASHER_INTERNAL_BUFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_INTERNAL_BUFFER, DasherInternalBufferClass ))
#define IS_DASHER_INTERNAL_BUFFER(obj)	       (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_INTERNAL_BUFFER))
#define IS_DASHER_INTERNAL_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_INTERNAL_BUFFER))
#define DASHER_INTERNAL_BUFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_INTERNAL_BUFFER, DasherInternalBufferClass))

typedef struct _DasherInternalBuffer DasherInternalBuffer;
typedef struct _DasherInternalBufferClass DasherInternalBufferClass;

struct _DasherInternalBuffer {
  GObject parent;
  gpointer private_data;
};

struct _DasherInternalBufferClass {
  GObjectClass parent_class;
};

DasherInternalBuffer *dasher_internal_buffer_new(GtkTextView *pTextView);
GType dasher_internal_buffer_get_type();

void dasher_internal_buffer_change_context(DasherInternalBuffer *pSelf);

void dasher_internal_buffer_clear(DasherInternalBuffer *pSelf);

G_END_DECLS

#endif
