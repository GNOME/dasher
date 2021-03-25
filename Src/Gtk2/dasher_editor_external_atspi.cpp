#include <X11/keysym.h>
#include <string.h>

#include <atspi/atspi.h>

#include "dasher_editor_external.h"
#include "dasher_editor_private.h"

typedef struct _DasherEditorExternalPrivate DasherEditorExternalPrivate;

struct _DasherEditorExternalPrivate {
  AtspiEventListener *pFocusListener;
  AtspiEventListener *pCaretListener;
  AtspiText *pAccessibleText;
  // Used to distinguish dasher initiated cursor moves from external moves so that the
  // interface stays in control mode. When dasher moves the caret it will also set
  // current_caret_position.
  glong current_caret_position;
};

void dasher_editor_external_handle_focus(DasherEditor *pSelf, const AtspiEvent *pEvent);
void dasher_editor_external_handle_caret(DasherEditor *pSelf, const AtspiEvent *pEvent);

void focus_listener(AtspiEvent *pEvent, void *pUserData);
void caret_listener(AtspiEvent *pEvent, void *pUserData);

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
  p->current_caret_position = -1;
  pPrivate->pExtPrivate = p;

#ifdef DEBUG_ATSPI
  printf("dasher_editor_external_create_buffer: pPrivate=%p, pExtPrivate=%p\n", pPrivate, p);
#endif
}

static void
listen_to_bus(DasherEditor *pSelf) {
  DasherEditorPrivate *p = DASHER_EDITOR_GET_PRIVATE(pSelf);
  //atspi_event_listener_register(p->pExtPrivate->pFocusListener, "focus:", NULL);
  atspi_event_listener_register(p->pExtPrivate->pFocusListener, "object:state-changed:focused", NULL);
  atspi_event_listener_register(p->pExtPrivate->pCaretListener, "object:text-caret-moved", NULL);
}

static void
unlisten_to_bus(DasherEditor *pSelf) {
  DasherEditorPrivate *p = DASHER_EDITOR_GET_PRIVATE(pSelf);
//  atspi_event_listener_deregister(p->pExtPrivate->pFocusListener, "focus:", NULL);
  atspi_event_listener_deregister(p->pExtPrivate->pFocusListener, "object:state-changed:focused", NULL);
  atspi_event_listener_deregister(p->pExtPrivate->pCaretListener, "object:text-caret-moved", NULL);
}

void
dasher_editor_external_toggle_direct_mode(DasherEditor *pSelf, bool direct) {
  DasherEditorPrivate *p = DASHER_EDITOR_GET_PRIVATE(pSelf);
  p->pExtPrivate->current_caret_position = -1;
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

std::string
dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength) {
  DasherEditorPrivate *pPrivate = DASHER_EDITOR_GET_PRIVATE(pSelf);

#ifdef DEBUG_ATSPI
  printf("dasher_editor_external_get_context(pPrivate=%p)\n", pPrivate);
#endif

  DASHER_ASSERT(pPrivate->pExtPrivate != NULL);
  AtspiText *textobj = pPrivate->pExtPrivate->pAccessibleText;

  if (textobj != nullptr) {
    auto text = atspi_text_get_text(textobj, iOffset, iOffset + iLength, NULL);
    if (text != nullptr) {
      std::string context = text;
      g_free(text);
      return context;
    }
  }
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
  pPrivate->pExtPrivate->current_caret_position = -1;
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

  textobj = atspi_accessible_get_text_iface(acc);
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
    // If dasher moved the caret don't send a notification to the control.
    glong caret = atspi_text_get_caret_offset(textobj, NULL);
    if (caret == pPrivate->pExtPrivate->current_caret_position) {
      return;
    }
    pPrivate->pExtPrivate->current_caret_position = -1;

    g_object_ref(textobj);

    //ACL: in old code, external_buffer emitted signal, for which the editor_external had
    // registered a callback, which then emitted the same/corresponding signal from the
    // editor_external; so by combining buffer into editor, seems we don't need any of that
    // and can just emit the signal from the editor directly. However, the callback also said:
    //TODO: plumb signal back into control
    // ...if that makes any sense?

    g_signal_emit_by_name(G_OBJECT(pSelf), "context_changed", G_OBJECT(pSelf), NULL, NULL);
    pPrivate->bInControlAction = false;
  }
#ifdef DEBUG_ATSPI
  else {
    printf("XXX Received text-caret-moved from source which doesn't implemenent AtspiText\n");
  }
#endif

  g_object_unref(acc);
}

void
focus_listener(AtspiEvent *pEvent, void *pUserData) {
  dasher_editor_external_handle_focus(DASHER_EDITOR(pUserData), pEvent);
}

void
caret_listener(AtspiEvent *pEvent, void *pUserData) {
  dasher_editor_external_handle_caret(DASHER_EDITOR(pUserData), pEvent);
}

void dasher_editor_external_move(DasherEditor *pSelf, bool bForwards, Dasher::CControlManager::EditDistance iDist) {
  DasherEditorPrivate *pPrivate = DASHER_EDITOR_GET_PRIVATE(pSelf);
  DASHER_ASSERT(pPrivate->pExtPrivate != nullptr);
  AtspiText *textobj = pPrivate->pExtPrivate->pAccessibleText;
  if (textobj == nullptr) {
    return;
  }

  GError *err = nullptr;
  auto caret_pos = atspi_text_get_caret_offset(textobj, &err);
  if (err != nullptr) {
    fprintf(stderr, "Failed to get the caret: %s\n", err->message);
    g_error_free(err);
    return;
  }

  auto length = atspi_text_get_character_count(textobj, &err);
  if (err != nullptr) {
    fprintf(stderr, "Failed to get the character count: %s\n", err->message);
    g_error_free(err);
    return;
  }
  if (length == 0) {
    return;
  }

  AtspiTextRange* range = nullptr;
  glong new_position = caret_pos;

  AtspiTextBoundaryType boundary = ATSPI_TEXT_BOUNDARY_CHAR;
  switch (iDist) {
    case Dasher::CControlManager::EDIT_CHAR:
      if (bForwards) {
        new_position = std::min<glong>(caret_pos + 1, length - 1);
      } else {
        new_position = std::max<glong>(caret_pos - 1, 0);
      }
      break;
    case Dasher::CControlManager::EDIT_FILE:
      if (bForwards) {
        new_position = length - 1;
      } else {
        new_position = 0;
      }
      break;
    case Dasher::CControlManager::EDIT_WORD:
      boundary = ATSPI_TEXT_BOUNDARY_WORD_START;
      break;
    case Dasher::CControlManager::EDIT_LINE:
      boundary = ATSPI_TEXT_BOUNDARY_LINE_START;
      break;
    case Dasher::CControlManager::EDIT_SENTENCE:
      boundary = ATSPI_TEXT_BOUNDARY_SENTENCE_START;
      break;
    default:
      break;
  }
  if (boundary != ATSPI_TEXT_BOUNDARY_CHAR) {
    if (bForwards) {
      range = atspi_text_get_text_after_offset(textobj, caret_pos, boundary, &err);
    } else {
      range = atspi_text_get_text_before_offset(textobj, caret_pos, boundary, &err);
    }
    if (err != nullptr) {
      fprintf(stderr, "Failed to get the text after/befor the offset: %s\n", err->message);
      g_error_free(err);
      return;
    }
    if (range != nullptr) {
      new_position = range->start_offset;
    }
    g_free(range);
  }
  atspi_text_set_caret_offset(textobj, new_position, &err);
  if (err != nullptr) {
    fprintf(stderr, "Failed to set the caret offset: %s\n", err->message);
    g_error_free(err);
    return;
  }
  pPrivate->pExtPrivate->current_caret_position = new_position;
}

std::string dasher_editor_external_get_text_around_cursor(
    DasherEditor *pSelf, Dasher::CControlManager::EditDistance distance) {
  DasherEditorPrivate *pPrivate = DASHER_EDITOR_GET_PRIVATE(pSelf);
  DASHER_ASSERT(pPrivate->pExtPrivate != nullptr);
  AtspiText *textobj = pPrivate->pExtPrivate->pAccessibleText;
  if (textobj == nullptr) {
    return "";
  }
  GError *err = nullptr;
  auto caret_pos = atspi_text_get_caret_offset(textobj, &err);
  if (err != nullptr) {
    fprintf(stderr, "Failed to get the caret offset: %s\n", err->message);
    g_error_free(err);
    return "";
  }

  std::string text;
  AtspiTextGranularity granularity;
  switch (distance) {
    case Dasher::CControlManager::EDIT_FILE: {
      auto length = atspi_text_get_character_count(textobj, nullptr);
      auto gtext = atspi_text_get_text(textobj, 0, length, nullptr);
      text = gtext;
      g_free(gtext);
      return text;
    }
      break;
    case Dasher::CControlManager::EDIT_WORD:
      granularity = ATSPI_TEXT_GRANULARITY_WORD;
      break;
    case Dasher::CControlManager::EDIT_LINE:
      granularity = ATSPI_TEXT_GRANULARITY_LINE;
      break;
    case Dasher::CControlManager::EDIT_SENTENCE:
      granularity = ATSPI_TEXT_GRANULARITY_SENTENCE;
      break;
    case Dasher::CControlManager::EDIT_PARAGRAPH:
      // TODO: figure out why ATSPI_TEXT_GRANULARITY_PARAGRAPH doesn't work.
      granularity = ATSPI_TEXT_GRANULARITY_PARAGRAPH;
      break;
    default:
      return "";
  }
  auto* range = atspi_text_get_string_at_offset(textobj, caret_pos, granularity, &err);
  if (err != nullptr) {
    fprintf(stderr, "Failed to get the caret offset: %s\n", err->message);
    g_error_free(err);
    return "";
  }
  if (range != nullptr) {
    text = range->content;
    g_free(range);
  }
  return text;
}
