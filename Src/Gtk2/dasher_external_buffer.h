#ifndef __dasher_external_buffer_h__
#define __dasher_external_buffer_h__

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS
#define TYPE_DASHER_EXTERNAL_BUFFER            (dasher_external_buffer_get_type())
#define DASHER_EXTERNAL_BUFFER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_DASHER_EXTERNAL_BUFFER, DasherExternalBuffer ))
#define DASHER_EXTERNAL_BUFFER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_DASHER_EXTERNAL_BUFFER, DasherExternalBufferClass ))
#define IS_DASHER_EXTERNAL_BUFFER(obj)	       (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_DASHER_EXTERNAL_BUFFER))
#define IS_DASHER_EXTERNAL_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_DASHER_EXTERNAL_BUFFER))
#define DASHER_EXTERNAL_BUFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_DASHER_EXTERNAL_BUFFER, DasherExternalBufferClass))

typedef struct _DasherExternalBuffer DasherExternalBuffer;
typedef struct _DasherExternalBufferClass DasherExternalBufferClass;

struct _DasherExternalBuffer {
  GObject parent;
  gpointer private_data;
};

struct _DasherExternalBufferClass {
  GObjectClass parent_class;
};

DasherExternalBuffer *dasher_external_buffer_new();
GType dasher_external_buffer_get_type();

G_END_DECLS

#endif
