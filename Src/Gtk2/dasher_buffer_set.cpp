#include "dasher_buffer_set.h"

// FIXME - figure out the correct way to make this available to classes implementing this interface
guint idasher_buffer_set_signals[NUM_SIGNALS];

static void idasher_buffer_set_base_init(gpointer g_class) {
  static gboolean initialised = FALSE;

  if(!initialised) {

    idasher_buffer_set_signals[CONTEXT_CHANGED] = g_signal_newv( "context_changed", 
								 G_TYPE_FROM_CLASS(g_class), 
								 static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), 
								 NULL,
								 NULL, 
								 NULL, 
								 g_cclosure_marshal_VOID__VOID, 
								 G_TYPE_NONE, 
								 0,
								 NULL);


    initialised = TRUE;
  }
}

GType idasher_buffer_set_get_type() {

  static GType idasher_buffer_set_type = 0;

  if(!idasher_buffer_set_type) {
    static const GTypeInfo dasher_buffer_set_info = {
      sizeof(IDasherBufferSetInterface),
      idasher_buffer_set_base_init,
      NULL,
      NULL,
      NULL,
      NULL,
      0,
      0,
      NULL
    };

    idasher_buffer_set_type = g_type_register_static(G_TYPE_INTERFACE, "IDasherBufferSet", &dasher_buffer_set_info, (GTypeFlags)0);
  }

  return idasher_buffer_set_type;
}

// FIXME - check for NULL pointers (is that contrary to the idea of an interface?)

void idasher_buffer_set_insert(IDasherBufferSet *pSelf, const gchar *szText) {
  IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->insert_text(pSelf, szText);
}

void idasher_buffer_set_delete(IDasherBufferSet *pSelf, gint iLength) {
  IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->delete_text(pSelf, iLength);
}

gchar *idasher_buffer_set_get_context(IDasherBufferSet *pSelf, gint iMaxLength) {
  return IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->get_context(pSelf, iMaxLength);
}

void idasher_buffer_set_edit_move(IDasherBufferSet *pSelf, gint iDirection, gint iDist) {
  IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->edit_move(pSelf, iDirection, iDist);
}

void idasher_buffer_set_edit_delete(IDasherBufferSet *pSelf, gint iDirection, gint iDist) {
  IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->edit_delete(pSelf, iDirection, iDist);
}
