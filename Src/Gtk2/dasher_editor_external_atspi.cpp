#include <X11/keysym.h>

#include <atspi/atspi.h>

#include "dasher_editor_external.h"
#include "dasher_editor_private.h"

typedef struct _DasherEditorExternalPrivate DasherEditorExternalPrivate;

struct _DasherEditorExternalPrivate {
  AtspiEventListener *pFocusListener;
  AtspiEventListener *pCaretListener;
  AtspiText *pAccessibleText;
};

void dasher_editor_external_handle_focus(DasherEditor *pSelf, const AtspiEvent *pEvent);
void dasher_editor_external_handle_caret(DasherEditor *pSelf, const AtspiEvent *pEvent);

void focus_listener(const AtspiEvent *pEvent, void *pUserData);
void caret_listener(const AtspiEvent *pEvent, void *pUserData);

static void listen_to_bus(DasherEditor *);
static void unlisten_to_bus(DasherEditor *);

bool
initSPI() {
#ifdef DEBUG_ATSPI
  int ret = atspi_init();
  printf("atspi_init() returned %d\n", ret);
  return (ret <= 1);
#else
  return (atspi_init() <= 1);
#endif
}

void
dasher_editor_external_finalize(GObject *pSelf) {
  DasherEditorPrivate *pPrivate = DASHER_EDITOR_GET_PRIVATE(pSelf);
  DasherEditorExternalPrivate *p = pPrivate->pExtPrivate;

#ifdef DEBUG_ATSPI
  printf("dasher_editor_external_finalize()\n");
#endif

  unlisten_to_bus(DASHER_EDITOR(pSelf));

  g_object_unref(p->pFocusListener);
  g_object_unref(p->pCaretListener);

  delete p;
}

void
dasher_editor_external_create_buffer(DasherEditor *pSelf) {
  DasherEditorPrivate *pPrivate = DASHER_EDITOR_GET_PRIVATE(pSelf);
  DasherEditorExternalPrivate *p;

#ifdef DEBUG_ATSPI
  printf("dasher_editor_external_create_buffer()\n");
#endif

  if(!initSPI()) {
    g_message("Could not initialise SPI - accessibility options disabled");
    return;
  }
  p = new DasherEditorExternalPrivate;
  p->pFocusListener = atspi_event_listener_new(focus_listener, pSelf, NULL);
  p->pCaretListener = atspi_event_listener_new(caret_listener, pSelf, NULL);
  p->pAccessibleText = NULL;
  pPrivate->pExtPrivate = p;

#ifdef DEBUG_ATSPI
  printf("dasher_editor_external_create_buffer: pPrivate=%p, pExtPrivate=%p\n", pPrivate, p);
#endif
}

static void
listen_to_bus(DasherEditor *pSelf) {
  DasherEditorPrivate *p = DASHER_EDITOR_GET_PRIVATE(pSelf);
  atspi_event_listener_register(p->pExtPrivate->pFocusListener, "focus:", NULL);
  atspi_event_listener_register(p->pExtPrivate->pCaretListener, "object:text-caret-moved", NULL);
}

static void
unlisten_to_bus(DasherEditor *pSelf) {
  DasherEditorPrivate *p = DASHER_EDITOR_GET_PRIVATE(pSelf);
  atspi_event_listener_deregister(p->pExtPrivate->pFocusListener, "focus:", NULL);
  atspi_event_listener_deregister(p->pExtPrivate->pCaretListener, "object:text-caret-moved", NULL);
}

void
dasher_editor_external_toggle_direct_mode(DasherEditor *pSelf, bool direct) {
  if (direct)
    listen_to_bus(pSelf);
  else
    unlisten_to_bus(pSelf);
}

void
dasher_editor_external_output(DasherEditor *pSelf, const char *szText, int iOffset /* unused */) {
  if (!initSPI()) return;

  atspi_generate_keyboard_event(0, szText, ATSPI_KEY_STRING, NULL);
}

void
dasher_editor_external_delete(DasherEditor *pSelf, int iLength, int iOffset) {
  if (!initSPI()) return;

  atspi_generate_keyboard_event(XK_BackSpace, NULL, ATSPI_KEY_SYM, NULL);
}

const char *
dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength) {
  DasherEditorPrivate *pPrivate = DASHER_EDITOR_GET_PRIVATE(pSelf);

#ifdef DEBUG_ATSPI
  printf("dasher_editor_external_get_context(pPrivate=%p)\n", pPrivate);
#endif

  DASHER_ASSERT(pPrivate->pExtPrivate != NULL);
  AtspiText *textobj = pPrivate->pExtPrivate->pAccessibleText;

  if (textobj)
    return atspi_text_get_text(textobj, iOffset, iOffset + iLength, NULL);
  else
    return "";
}

int
dasher_editor_external_get_offset(DasherEditor *pSelf) {
  DasherEditorPrivate *pPrivate = DASHER_EDITOR_GET_PRIVATE(pSelf);

#ifdef DEBUG_ATSPI
  printf("dasher_editor_external_get_offset()\n");
#endif

  DASHER_ASSERT(pPrivate->pExtPrivate != NULL);
  AtspiText *textobj = pPrivate->pExtPrivate->pAccessibleText;
  AtspiRange *range;
  int ret;
  
  if (!textobj) {
#ifdef DEBUG_ATSPI
    printf("no textobj\n");
#endif
    return 0;
  }

  if (atspi_text_get_n_selections(textobj, NULL) == 0)
    return atspi_text_get_caret_offset(textobj, NULL);

  range = atspi_text_get_selection(textobj, 0, NULL);
  ret = std::min(range->start_offset, range->end_offset);
  g_free(range);

  return ret;
}

void
dasher_editor_external_handle_focus(DasherEditor *pSelf, const AtspiEvent *pEvent) {
  DasherEditorPrivate *pPrivate = DASHER_EDITOR_GET_PRIVATE(pSelf);

#ifdef DEBUG_ATSPI
  printf("dasher_editor_external_handle_focus()\n");
#endif

  DASHER_ASSERT(pPrivate->pExtPrivate != NULL);
  AtspiText *textobj = pPrivate->pExtPrivate->pAccessibleText;

  if (textobj) {
    g_object_unref(textobj);
    textobj = NULL;
  }

  AtspiAccessible *acc = pEvent->source;
  g_object_ref(acc);
  
#ifdef DEBUG_ATSPI
  printf("%s from %s, %s, %s\n",
    pEvent->type,
    atspi_accessible_get_name(acc, NULL),
    atspi_accessible_get_role_name(acc, NULL),
    atspi_accessible_get_description(acc, NULL));
#endif

  textobj = atspi_accessible_get_text(acc);
  pPrivate->pExtPrivate->pAccessibleText = textobj;
  if (textobj) {
    g_object_ref(textobj);

    //ACL: in old code, external_buffer emitted signal, for which the editor_external had
    // registered a callback, which then emitted the same/corresponding signal from the
    // editor_external; so by combining buffer into editor, seems we don't need any of that
    // and can just emit the signal from the editor directly. However, the callback also said:
    //TODO: plumb signal back into control
    // ...if that makes any sense?

    g_signal_emit_by_name(G_OBJECT(pSelf), "buffer_changed", G_OBJECT(pSelf), NULL, NULL);
  }

  g_object_unref(acc);
}

void
dasher_editor_external_handle_caret(DasherEditor *pSelf, const AtspiEvent *pEvent) {
  DasherEditorPrivate *pPrivate = DASHER_EDITOR_GET_PRIVATE(pSelf);

#ifdef DEBUG_ATSPI
  printf("dasher_editor_external_handle_caret()\n");
#endif

  DASHER_ASSERT(pPrivate->pExtPrivate != NULL);
  AtspiText *textobj = pPrivate->pExtPrivate->pAccessibleText;

  if (textobj) {
    g_object_unref(textobj);
    textobj = NULL;
  }

  AtspiAccessible *acc = pEvent->source;
  g_object_ref(acc);
  
#ifdef DEBUG_ATSPI
  printf("%s from %s, %s, %s\n",
    pEvent->type,
    atspi_accessible_get_name(acc, NULL),
    atspi_accessible_get_role_name(acc, NULL),
    atspi_accessible_get_description(acc, NULL));
#endif

  textobj = atspi_accessible_get_text(acc);
  pPrivate->pExtPrivate->pAccessibleText = textobj;
  if (textobj) {
    g_object_ref(textobj);

    //ACL: in old code, external_buffer emitted signal, for which the editor_external had
    // registered a callback, which then emitted the same/corresponding signal from the
    // editor_external; so by combining buffer into editor, seems we don't need any of that
    // and can just emit the signal from the editor directly. However, the callback also said:
    //TODO: plumb signal back into control
    // ...if that makes any sense?

    g_signal_emit_by_name(G_OBJECT(pSelf), "context_changed", G_OBJECT(pSelf), NULL, NULL);
  }
#ifdef DEBUG_ATSPI
  else {
    printf("XXX Received text-caret-moved from source which doesn't implemenent AtspiText\n");
  }
#endif

  g_object_unref(acc);
}

void
focus_listener(const AtspiEvent *pEvent, void *pUserData) {
  dasher_editor_external_handle_focus(DASHER_EDITOR(pUserData), pEvent);
}

void
caret_listener(const AtspiEvent *pEvent, void *pUserData) {
  dasher_editor_external_handle_caret(DASHER_EDITOR(pUserData), pEvent);
}
