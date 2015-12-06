#ifndef DASHER_EDITOR_PRIVATE_H
#define DASHER_EDITOR_PRIVATE_H

#include <gtk/gtk.h>

// Forward declarations
class DasherAppSettings;
typedef struct _GtkDasherControl GtkDasherControl;
struct _GtkDasherControl;
typedef struct _DasherEditorExternalPrivate DasherEditorExternalPrivate;
struct _DasherEditorExternalPrivate;

typedef struct _DasherEditorPrivate DasherEditorPrivate;

struct _DasherEditorPrivate {
  GtkDasherControl *pDasherCtrl;
  GtkTextView *pTextView;
  GtkTextBuffer *pBuffer;
  GtkClipboard *pTextClipboard;
  GtkClipboard *pPrimarySelection;
  GtkTextMark *pNewMark;
  DasherAppSettings *pAppSettings;
  gchar *szFilename;
  gboolean bFileModified; // TODO: Make this work properly, export to main for quit etc

  // for conversion mode:
  GtkTextTag *pOutputTag;
  GtkTextTag *pHiddenTag;
  GtkTextTag *pVisibleTag;
  gboolean bConversionMode;
  gint iLastOffset;
  gint iCurrentState; // 0 = unconverted, 1 = converted

  // for direct mode:
  DasherEditorExternalPrivate *pExtPrivate;

  //Paralleling the previous approach in dasher_main, we _don't_ send context_changed
  // events if we're in the middle of executing a control action (as this would rebuild
  // the canvas)
  gboolean bInControlAction;
};

#define DASHER_EDITOR_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), DASHER_TYPE_EDITOR, DasherEditorPrivate))

#endif
