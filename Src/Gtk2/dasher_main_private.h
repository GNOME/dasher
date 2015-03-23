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

  // Widgets used for maemo
#ifdef WITH_MAEMO
  DasherMaemoHelper *pMaemoHelper;
#ifdef WITH_MAEMOFULLSCREEN
  HildonProgram *pProgram;
  HildonWindow *pHWindow;
#endif
#endif

  // Properties of the main window
  int iWidth;
  int iHeight;
  bool bWidgetsInitialised;
};

typedef struct _DasherMainPrivate DasherMainPrivate;

#define DASHER_MAIN_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), DASHER_TYPE_MAIN, DasherMainPrivate))

#endif
