#ifndef DASHER_MAIN_PRIVATE_H
#define DASHER_MAIN_PRIVATE_H

#include "dasher_main.h"

struct _DasherMainPrivate {
  GtkBuilder *pXML;
  GtkBuilder *pPrefXML;

  // Child objects owned here
  DasherAppSettings *pAppSettings;
  DasherPreferencesDialogue *pPreferencesDialogue;
  DasherEditor *pEditor;

  CKeyboardHelper *pKeyboardHelper;

  // Various widgets which need to be cached:
  // GtkWidget *pBufferView;
  GtkPaned  *pDivider;
  GtkWindow *pMainWindow;
  GtkWidget *pToolbar;
  GtkSpinButton *pSpeedBox;
  GtkWidget *pAlphabetCombo;
  GtkWidget *pStatusControl;
  GtkWidget *pDasherWidget;

  GtkListStore *pAlphabetList;
  GtkAccelGroup *pAccel;

  // Properties of the main window
  int iWidth;
  int iHeight;
  bool bWidgetsInitialised;
  int parameter_callback_id_ = 0;
};

typedef struct _DasherMainPrivate DasherMainPrivate;

#define DASHER_MAIN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), DASHER_TYPE_MAIN, DasherMainPrivate))

#endif
