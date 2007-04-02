#include "dasher_buffer_set.h"

enum {
  BUFFER_CHANGED,
  OFFSET_CHANGED,
  NUM_SIGNALS
};

// FIXME - figure out the correct way to make this available to classes implementing this interface
guint idasher_buffer_set_signals[NUM_SIGNALS];

static void idasher_buffer_set_base_init(gpointer g_class) {
  static gboolean initialised = FALSE;

  if(!initialised) {
    idasher_buffer_set_signals[BUFFER_CHANGED] = g_signal_newv( "buffer_changed", 
								G_TYPE_FROM_CLASS(g_class), 
								static_cast < GSignalFlags > (G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION), 
								NULL,
								NULL, 
								NULL, 
								g_cclosure_marshal_VOID__VOID, 
								G_TYPE_NONE, 
								0,
								NULL);

    idasher_buffer_set_signals[OFFSET_CHANGED] = g_signal_newv( "offset_changed", 
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

void idasher_buffer_set_insert(IDasherBufferSet *pSelf, const gchar *szText, int iOffset) {
  IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->insert_text(pSelf, szText, iOffset);
}

void idasher_buffer_set_delete(IDasherBufferSet *pSelf, gint iLength, int iOffset) {
  IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->delete_text(pSelf, iLength, iOffset);
}

gchar *idasher_buffer_set_get_context(IDasherBufferSet *pSelf, gint iOffset, gint iLength) {
  return IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->get_context(pSelf, iOffset, iLength);
}

void idasher_buffer_set_edit_move(IDasherBufferSet *pSelf, gint iDirection, gint iDist) {
  IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->edit_move(pSelf, iDirection, iDist);
}

void idasher_buffer_set_edit_delete(IDasherBufferSet *pSelf, gint iDirection, gint iDist) {
  IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->edit_delete(pSelf, iDirection, iDist);
}

void idasher_buffer_set_edit_convert(IDasherBufferSet *pSelf) {
  IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->edit_convert(pSelf);
}

void idasher_buffer_set_edit_protect(IDasherBufferSet *pSelf) {
  IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->edit_protect(pSelf);
}

void idasher_buffer_set_conversion_mode(IDasherBufferSet *pSelf, gboolean bMode) {
  IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->conversion_mode(pSelf, bMode);
}

gint idasher_buffer_set_get_offset(IDasherBufferSet *pSelf) {
  return IDASHER_BUFFER_SET_GET_INTERFACE(pSelf)->get_offset(pSelf);
}
