//
// This is obsolete code and is no longer included in the build system
//

#include "../Common/Common.h"

#include "accessibility.h"
#include "edit.h"
#include "AppSettings.h"

#include <libintl.h>
#include <iostream>

#ifdef HAVE_WNCK
// #include <libwnck/libwnck.h>
// WnckScreen *wnckscreen;
#endif

ControlTree *menutree;
ControlTree *buttontree;
ControlTree *paneltree;
ControlTree *edittree;
ControlTree *widgettree;
ControlTree *windowtree;
ControlTree *stoptree;
ControlTree *dummy;             // This one is used to fake another control node
extern ControlTree *controltree;
#define _(x) gettext(x)

extern gboolean textentry;
extern gboolean training;
extern gboolean quitting;
extern GtkWidget *the_canvas;
extern GtkWidget *window;


gboolean panels = FALSE;
gboolean building = FALSE;

#ifdef GNOME_A11Y

std::vector < Accessible * >menuitems;

Accessible *desktop = NULL;
Accessible *focusedwindow = NULL;
Accessible *dasherwindow = NULL;

static void dasher_focus_listener(const AccessibleEvent * event, void *user_data);
static void dasher_caret_listener(const AccessibleEvent * event, void *user_data);
static AccessibleEventListener *focusListener;
static AccessibleEventListener *caretListener;

AccessibleText *g_pAccessibleText = 0;

#endif

void setupa11y() {
#ifdef GNOME_A11Y
//   focusListener = SPI_createAccessibleEventListener(dasher_focus_listener, NULL);
//   caretListener = SPI_createAccessibleEventListener(dasher_caret_listener, NULL);

//   register_listeners();
#endif
}

void register_listeners() {
//   if(dasher_app_settings_get_long(g_pDasherAppSettings, APP_LP_STYLE) == 2) {
//     SPI_registerGlobalEventListener(focusListener, "focus:");
//     SPI_registerGlobalEventListener(caretListener, "object:text-caret-moved");
//   }
//   else {
//     SPI_deregisterGlobalEventListener(focusListener, "focus:");
//     SPI_deregisterGlobalEventListener(caretListener, "object:text-caret-moved");
//   }
}

#ifdef GNOME_A11Y

void dasher_caret_listener(const AccessibleEvent *event, void *user_data) {

  Accessible_ref(event->source);

  AccessibleStateSet *pAStateSet = Accessible_getStateSet(event->source);
  AccessibleStateSet_ref(pAStateSet);

  if(!AccessibleStateSet_contains(pAStateSet, SPI_STATE_FOCUSED)) {
    return;
  }

  AccessibleStateSet_unref(pAStateSet);
  Accessible_unref(event->source);
  
  if(g_pAccessibleText) {
    int iActualPosition = event->detail1;

    std::cout << "Expected: " << g_iExpectedPosition << " Actual: " << iActualPosition << std::endl;

    if((g_iExpectedPosition - iActualPosition) * (g_iOldPosition - iActualPosition) > 0) {
      gtk_dasher_control_invalidate_context(GTK_DASHER_CONTROL(pDasherWidget));
      g_iExpectedPosition = iActualPosition;
    }

    g_iOldPosition = iActualPosition;
  }
}

void dasher_focus_listener(const AccessibleEvent *event, void *user_data) {
  if(g_pAccessibleText) {
    AccessibleText_unref(g_pAccessibleText);
    g_pAccessibleText = 0;
  }
  
  Accessible *accessible = event->source;
  Accessible_ref(accessible);

  if(Accessible_isText(accessible) || Accessible_isEditableText(accessible)) {
    g_pAccessibleText = Accessible_getText(accessible);
    AccessibleText_ref(g_pAccessibleText);

    g_iExpectedPosition = AccessibleText_getCaretOffset(g_pAccessibleText);
    g_iOldPosition = g_iExpectedPosition;
  }

  Accessible_unref(accessible);

  gtk_dasher_control_invalidate_context(GTK_DASHER_CONTROL(pDasherWidget));
}

const char *get_accessible_context(int iLength) {
  if(g_pAccessibleText) {
    int iOffset(AccessibleText_getCaretOffset(g_pAccessibleText));
    return AccessibleText_getText(g_pAccessibleText, iOffset-iLength, iOffset);
  }
  else {
    return 0;
  }
}

//FIXME - ripped straight from gok. The same qualms as they have apply.

gboolean dasher_check_window(AccessibleRole role) {
  /* TODO - improve efficiency here? Also, roles get added and we need
     to maintain this...  maybe we need to go about this differently */
  if((role == SPI_ROLE_WINDOW) || (role == SPI_ROLE_DIALOG) || (role == SPI_ROLE_FILE_CHOOSER) || (role == SPI_ROLE_FRAME) || (role == SPI_ROLE_DESKTOP_FRAME) || (role == SPI_ROLE_FONT_CHOOSER) || (role == SPI_ROLE_COLOR_CHOOSER) || (role == SPI_ROLE_APPLICATION) || (role == SPI_ROLE_ALERT)
    ) {
    return TRUE;
  }
  return FALSE;
}
#endif
