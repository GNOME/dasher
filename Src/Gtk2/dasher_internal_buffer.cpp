#include <gtk/gtk.h>
#include <iostream>
#include <string.h>

#include "../Common/Common.h"
#include "dasher_buffer_set.h"
#include "dasher_internal_buffer.h"

static void dasher_internal_buffer_class_init(DasherInternalBufferClass *pClass);
static void dasher_internal_buffer_init(DasherInternalBuffer *pAction);
static void dasher_internal_buffer_destroy(GObject *pObject);
static void idasher_buffer_set_interface_init (gpointer g_iface, gpointer iface_data);
void dasher_internal_buffer_insert(DasherInternalBuffer *pSelf, const gchar *szText, int iOffset);
void dasher_internal_buffer_delete(DasherInternalBuffer *pSelf, int iLength, int iOffset);
gchar *dasher_internal_buffer_get_context(DasherInternalBuffer *pSelf, gint iOffset, gint iLength);
void dasher_internal_buffer_edit_move(DasherInternalBuffer *pSelf, int iDirection, int iDist);
void dasher_internal_buffer_edit_delete(DasherInternalBuffer *pSelf, int iDirection, int iDist);
void dasher_internal_buffer_edit_convert(DasherInternalBuffer *pSelf);
void dasher_internal_buffer_edit_protect(DasherInternalBuffer *pSelf);
void dasher_internal_buffer_conversion_mode(DasherInternalBuffer *pSelf, gboolean bMode);
gint dasher_internal_buffer_get_offset(DasherInternalBuffer *pSelf);

// Related signal handlers
extern "C" void mark_set_handler(GtkWidget *widget, GtkTextIter *pIter, GtkTextMark *pMark, gpointer pUserData);

typedef struct _DasherInternalBufferPrivate DasherInternalBufferPrivate;

struct _DasherInternalBufferPrivate {
  GtkTextView *pTextView;
  GtkTextBuffer *pBuffer;
  GtkTextTag *pOutputTag;
  GtkTextTag *pHiddenTag;
  GtkTextTag *pVisibleTag;
  gboolean bConversionMode;
  gint iLastOffset;
  gint iCurrentState; // 0 = unconverted, 1 = converted
};

GType dasher_internal_buffer_get_type() {

  static GType dasher_internal_buffer_type = 0;

  if(!dasher_internal_buffer_type) {
    static const GTypeInfo dasher_internal_buffer_info = {
      sizeof(DasherInternalBufferClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_internal_buffer_class_init,
      NULL,
      NULL,
      sizeof(DasherInternalBuffer),
      0,
      (GInstanceInitFunc) dasher_internal_buffer_init,
      NULL
    };

    static const GInterfaceInfo idasher_buffer_set_info = {
      (GInterfaceInitFunc) idasher_buffer_set_interface_init,
      NULL,
      NULL 
    };
    
    dasher_internal_buffer_type = g_type_register_static(G_TYPE_OBJECT, "DasherInternalBuffer", &dasher_internal_buffer_info, static_cast < GTypeFlags > (0));

    g_type_add_interface_static (dasher_internal_buffer_type,
                                 IDASHER_BUFFER_SET_TYPE,
                                 &idasher_buffer_set_info);
  }

  return dasher_internal_buffer_type;
}

static void dasher_internal_buffer_class_init(DasherInternalBufferClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_internal_buffer_destroy;
}

static void dasher_internal_buffer_init(DasherInternalBuffer *pDasherControl) {
  pDasherControl->private_data = (void *)(new DasherInternalBufferPrivate);
}

static void idasher_buffer_set_interface_init (gpointer g_iface, gpointer iface_data) {
  IDasherBufferSetInterface *iface = (IDasherBufferSetInterface *)g_iface;
  iface->insert_text = (void (*)(IDasherBufferSet *pSelf, const gchar *szText, int iOffset))dasher_internal_buffer_insert;
  iface->delete_text = (void (*)(IDasherBufferSet *pSelf, gint iLength, int iOffset))dasher_internal_buffer_delete;
  iface->get_context = (gchar *(*)(IDasherBufferSet *pSelf, gint iOffset, gint iLength))dasher_internal_buffer_get_context;
  iface->edit_move = (void (*)(IDasherBufferSet *pSelf, gint iDirection, gint iDist))dasher_internal_buffer_edit_move;
  iface->edit_delete = (void (*)(IDasherBufferSet *pSelf, gint iDirection, gint iDist))dasher_internal_buffer_edit_delete;
  iface->edit_convert = (void (*)(IDasherBufferSet *pSelf))dasher_internal_buffer_edit_convert;
  iface->edit_protect = (void (*)(IDasherBufferSet *pSelf))dasher_internal_buffer_edit_protect;
  iface->conversion_mode = (void (*)(IDasherBufferSet *pSelf, gboolean bMode))dasher_internal_buffer_conversion_mode;
  iface->get_offset = (gint (*)(IDasherBufferSet *pSelf))dasher_internal_buffer_get_offset;
}

static void dasher_internal_buffer_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...

  delete (DasherInternalBufferPrivate *)(((DasherInternalBuffer *)pObject)->private_data);
}

DasherInternalBuffer *dasher_internal_buffer_new(GtkTextView *pTextView) {
  DasherInternalBuffer *pDasherControl;
  pDasherControl = (DasherInternalBuffer *)(g_object_new(dasher_internal_buffer_get_type(), NULL));

  DasherInternalBufferPrivate *pPrivate = (DasherInternalBufferPrivate *)(pDasherControl->private_data);
  pPrivate->pTextView = pTextView;
  pPrivate->pBuffer = gtk_text_view_get_buffer(pTextView);

  pPrivate->pOutputTag = gtk_text_buffer_create_tag(pPrivate->pBuffer, NULL, NULL);

#if GTK_CHECK_VERSION(2,8,0)
  pPrivate->pHiddenTag = gtk_text_buffer_create_tag(pPrivate->pBuffer, NULL, "invisible", TRUE, NULL);
#else
  // TODO: We really ought to do something a little more sensible with conversion in GTK < 2.8
  pPrivate->pHiddenTag = gtk_text_buffer_create_tag(pPrivate->pBuffer, NULL, NULL);
#endif

  pPrivate->pVisibleTag = gtk_text_buffer_create_tag(pPrivate->pBuffer, NULL, "foreground", "red", NULL);

  pPrivate->bConversionMode = FALSE;
  pPrivate->iLastOffset = 1;
  pPrivate->iCurrentState = 0;

  g_signal_connect(G_OBJECT(pPrivate->pBuffer), "mark-set", G_CALLBACK(mark_set_handler), pDasherControl);

  return pDasherControl;
}

void dasher_internal_buffer_insert(DasherInternalBuffer *pSelf, const gchar *szText, int iOffset) { 
  DasherInternalBufferPrivate *pPrivate = (DasherInternalBufferPrivate *)(pSelf->private_data);

  //  std::cout << "i: " << szText << " (" << iOffset << " " << gtk_text_buffer_get_char_count(pPrivate->pBuffer) << ")" << std::endl;

  // This seems overzealous.
  // DASHER_ASSERT(gtk_text_buffer_get_char_count(pPrivate->pBuffer) == iOffset);

  gtk_text_buffer_delete_selection(pPrivate->pBuffer, false, true );

  GtkTextIter sIter;
  gtk_text_buffer_get_iter_at_mark(pPrivate->pBuffer, &sIter, gtk_text_buffer_get_insert(pPrivate->pBuffer));

  GtkTextTag *pCurrentTag = NULL;

  if(!pPrivate->bConversionMode)
    pCurrentTag = pPrivate->pOutputTag;
  else {
    switch(pPrivate->iCurrentState) {
    case 0:
      pCurrentTag = pPrivate->pVisibleTag;
      break;
    case 1:
      pCurrentTag = pPrivate->pOutputTag;
      break;
    }
  }

  if(!pCurrentTag)
    return;

  gtk_text_buffer_insert_with_tags(pPrivate->pBuffer, &sIter, szText, -1, pCurrentTag, NULL);

  gtk_text_view_scroll_mark_onscreen(pPrivate->pTextView, gtk_text_buffer_get_insert(pPrivate->pBuffer));

  //  g_message("Buffer lenght: %d", gtk_text_buffer_get_char_count(pPrivate->pBuffer));
}

void dasher_internal_buffer_delete(DasherInternalBuffer *pSelf, int iLength, int iOffset) { 
  DasherInternalBufferPrivate *pPrivate = (DasherInternalBufferPrivate *)(pSelf->private_data);
  
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(pPrivate->pBuffer, end, gtk_text_buffer_get_insert(pPrivate->pBuffer));

  *start = *end;

  gtk_text_iter_backward_chars(start, iLength);
  //  g_bIgnoreCursorMove = true;
  gtk_text_buffer_delete(pPrivate->pBuffer, start, end);
  gtk_text_view_scroll_mark_onscreen(pPrivate->pTextView, gtk_text_buffer_get_insert(pPrivate->pBuffer));
  //  g_bIgnoreCursorMove = false;

  delete start;
  delete end;

  //  std::cout << "d: " << iLength << " (" << iOffset << " " << gtk_text_buffer_get_char_count(pPrivate->pBuffer) << ")" << std::endl; 
  // This seems overzealous.
  // DASHER_ASSERT(gtk_text_buffer_get_char_count(pPrivate->pBuffer) == iOffset);
}

gchar *dasher_internal_buffer_get_context(DasherInternalBuffer *pSelf, gint iOffset, gint iLength) {
  DasherInternalBufferPrivate *pPrivate = (DasherInternalBufferPrivate *)(pSelf->private_data);

  //  g_message("Buffer lenght: %d", gtk_text_buffer_get_char_count(pPrivate->pBuffer));

  GtkTextIter start;
  GtkTextIter end; // Refers to end of context, which is start of selection!

  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &start, iOffset);
  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &end, iOffset + iLength);

  return gtk_text_buffer_get_text( pPrivate->pBuffer, &start, &end, false );
}

// TODO: Check that these signals are being used appropriately
void dasher_internal_buffer_change_context(DasherInternalBuffer *pSelf, GtkTextIter *pIter, GtkTextMark *pMark) {
  const char *szMarkName(gtk_text_mark_get_name(pMark));
  if(szMarkName && !strcmp(szMarkName,"insert")) {
    g_signal_emit_by_name(G_OBJECT(pSelf), "offset_changed", G_OBJECT(pSelf), NULL, NULL);
  }
}

void dasher_internal_buffer_edit_move(DasherInternalBuffer *pSelf, int iDirection, int iDist) {
  DasherInternalBufferPrivate *pPrivate = (DasherInternalBufferPrivate *)(pSelf->private_data);
  
  GtkTextIter sPos;

  gtk_text_buffer_get_iter_at_mark(pPrivate->pBuffer, &sPos, gtk_text_buffer_get_insert(pPrivate->pBuffer));
  
  if(iDirection == EDIT_FORWARDS) {
    switch(iDist) {
    case EDIT_CHAR:
      gtk_text_iter_forward_char(&sPos);
      break;
    case EDIT_WORD:
      gtk_text_iter_forward_word_end(&sPos);
      break;
    case EDIT_LINE:
      if(!gtk_text_view_forward_display_line_end(GTK_TEXT_VIEW(pPrivate->pTextView), &sPos))
      {
        gtk_text_view_forward_display_line (GTK_TEXT_VIEW(pPrivate->pTextView), &sPos);
        gtk_text_view_forward_display_line_end(GTK_TEXT_VIEW(pPrivate->pTextView), &sPos);
      }
      break;
    case EDIT_FILE:
      gtk_text_iter_forward_to_end(&sPos);
      break;
    }
  }
  else { 
    switch(iDist) {
    case EDIT_CHAR:
      gtk_text_iter_backward_char(&sPos);
      break;
    case EDIT_WORD:
      gtk_text_iter_backward_word_start(&sPos);
      break;
    case EDIT_LINE:
    
      if(!gtk_text_view_backward_display_line_start(GTK_TEXT_VIEW(pPrivate->pTextView), &sPos))
        gtk_text_view_backward_display_line(GTK_TEXT_VIEW(pPrivate->pTextView), &sPos);
      break;
    case EDIT_FILE:
      gtk_text_buffer_get_start_iter(pPrivate->pBuffer, &sPos);
      break;
    }
  }

  gtk_text_buffer_place_cursor(pPrivate->pBuffer, &sPos);
  gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(pPrivate->pTextView), gtk_text_buffer_get_insert(pPrivate->pBuffer));
}

void dasher_internal_buffer_edit_delete(DasherInternalBuffer *pSelf, int iDirection, int iDist) { 
  DasherInternalBufferPrivate *pPrivate = (DasherInternalBufferPrivate *)(pSelf->private_data);
  
  GtkTextIter sPosStart;
  GtkTextIter sPosEnd;

  gtk_text_buffer_get_iter_at_mark(pPrivate->pBuffer, &sPosStart, gtk_text_buffer_get_insert(pPrivate->pBuffer));

  sPosEnd = sPosStart;
  
  if(iDirection == EDIT_FORWARDS) {
    switch(iDist) {
    case EDIT_CHAR:
      gtk_text_iter_forward_char(&sPosStart);
      break;
    case EDIT_WORD:
      gtk_text_iter_forward_word_end(&sPosStart);
      break;
    case EDIT_LINE:
      if(!gtk_text_view_forward_display_line_end(GTK_TEXT_VIEW(pPrivate->pTextView), &sPosStart))
      {
        gtk_text_view_forward_display_line (GTK_TEXT_VIEW(pPrivate->pTextView), &sPosStart);
        gtk_text_view_forward_display_line_end(GTK_TEXT_VIEW(pPrivate->pTextView), &sPosStart);
      }
      break;
    case EDIT_FILE:
      gtk_text_iter_forward_to_end(&sPosStart);
      break;
    }
  }
  else { 
    switch(iDist) {
    case EDIT_CHAR:
      gtk_text_iter_backward_char(&sPosStart);
      break;
    case EDIT_WORD:
      gtk_text_iter_backward_word_start(&sPosStart);
      break;
    case EDIT_LINE:
      if(!gtk_text_view_backward_display_line_start(GTK_TEXT_VIEW(pPrivate->pTextView), &sPosStart))
        gtk_text_view_backward_display_line(GTK_TEXT_VIEW(pPrivate->pTextView), &sPosStart);
      break;
    case EDIT_FILE:
      gtk_text_buffer_get_start_iter(pPrivate->pBuffer, &sPosStart);
      break;
    }
  }

  gtk_text_buffer_delete(pPrivate->pBuffer, &sPosStart, &sPosEnd);
  gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW(pPrivate->pTextView), gtk_text_buffer_get_insert(pPrivate->pBuffer));
}

void dasher_internal_buffer_edit_convert(DasherInternalBuffer *pSelf) {
  DasherInternalBufferPrivate *pPrivate = (DasherInternalBufferPrivate *)(pSelf->private_data);

  if(!(pPrivate->bConversionMode))
    return;

  GtkTextIter sStartIter;
  GtkTextIter sEndIter;
  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &sStartIter, pPrivate->iLastOffset);
  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &sEndIter, -1);
  gtk_text_buffer_apply_tag(pPrivate->pBuffer, pPrivate->pHiddenTag, &sStartIter, &sEndIter);

  pPrivate->iCurrentState = 1;
  pPrivate->iLastOffset = gtk_text_buffer_get_char_count(pPrivate->pBuffer);
}

void dasher_internal_buffer_edit_protect(DasherInternalBuffer *pSelf) {  
  DasherInternalBufferPrivate *pPrivate = (DasherInternalBufferPrivate *)(pSelf->private_data);

  if(!(pPrivate->bConversionMode))
    return;

  pPrivate->iCurrentState = 0;
  pPrivate->iLastOffset = gtk_text_buffer_get_char_count(pPrivate->pBuffer);
}

void dasher_internal_buffer_conversion_mode(DasherInternalBuffer *pSelf, gboolean bMode) {
  DasherInternalBufferPrivate *pPrivate = (DasherInternalBufferPrivate *)(pSelf->private_data);

  if(bMode) {
    pPrivate->bConversionMode = TRUE;
    pPrivate->iCurrentState = 0; 
    pPrivate->iLastOffset = gtk_text_buffer_get_char_count(pPrivate->pBuffer);
  }
  else {
    pPrivate->bConversionMode = FALSE;
    pPrivate->iCurrentState = 1;
  }
}

void dasher_internal_buffer_clear(DasherInternalBuffer *pSelf) {
  DasherInternalBufferPrivate *pPrivate = (DasherInternalBufferPrivate *)(pSelf->private_data);
  GtkTextIter start, end;

  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &start, 0);
  gtk_text_buffer_get_iter_at_offset(pPrivate->pBuffer, &end, -1);

  gtk_text_buffer_delete(pPrivate->pBuffer, &start, &end);

  /* TODO: this probably shouldn't emit a signal */
  g_signal_emit_by_name(G_OBJECT(pSelf), "buffer_changed", G_OBJECT(pSelf), NULL, NULL);

  pPrivate->iCurrentState = 0;
  pPrivate->iLastOffset = 0;
}

gint dasher_internal_buffer_get_offset(DasherInternalBuffer *pSelf) {
  DasherInternalBufferPrivate *pPrivate = (DasherInternalBufferPrivate *)(pSelf->private_data);
 
  GtkTextIter oIter;
  gtk_text_buffer_get_iter_at_mark(pPrivate->pBuffer, &oIter, gtk_text_buffer_get_mark(pPrivate->pBuffer, "insert"));
  return gtk_text_iter_get_offset(&oIter);
}


// Handlers

extern "C" void mark_set_handler(GtkWidget *widget, GtkTextIter *pIter, GtkTextMark *pMark, gpointer pUserData) {
  dasher_internal_buffer_change_context(DASHER_INTERNAL_BUFFER(pUserData), pIter, pMark);
}
