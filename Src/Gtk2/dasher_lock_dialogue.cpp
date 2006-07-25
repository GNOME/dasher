#include "../../config.h"

#include "GtkDasherControl.h"
#include "dasher.h"
#include "dasher_lock_dialogue.h"

#ifdef WITH_MAEMO
#include <hildon-widgets/hildon-banner.h>
#endif
#include <iostream>

// TODO: Make this a real class

GtkWidget *m_pLockWindow;
GtkWidget *m_pLockProgress;
GtkWidget *m_pLockMessage;

void dasher_lock_dialogue_new(GladeXML *pGladeXML, GtkWindow *pMainWindow) {
#ifndef WITH_MAEMO
  m_pLockWindow = glade_xml_get_widget(pGladeXML, "lock_window");
  m_pLockProgress = glade_xml_get_widget(pGladeXML, "lock_progress");
  m_pLockMessage = glade_xml_get_widget(pGladeXML, "lock_message");

  gtk_widget_hide(m_pLockWindow);
  
  dasher_lock_dialogue_set_transient(pMainWindow);
#else
  m_pLockWindow = 0;
#endif
}

void dasher_lock_dialogue_set_transient(GtkWindow *pMainWindow) {
  gtk_window_set_transient_for(GTK_WINDOW(m_pLockWindow), pMainWindow);
}

extern "C" void on_lock_info(GtkDasherControl *pDasherControl, gpointer pLockInfo, gpointer pUserData) {
  // TODO: signals are connected after the Dasher control is created,
  // which is too late to receive notification about intial training
  // etc.

  DasherLockInfo *pInfo = (DasherLockInfo *)pLockInfo;

#ifndef WITH_MAEMO
  gtk_label_set_text(GTK_LABEL(m_pLockMessage), pInfo->szMessage);
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(m_pLockProgress), pInfo->iPercent / 100.0);

  if(pInfo->bLock)
    gtk_widget_show(m_pLockWindow);
  else
    gtk_widget_hide(m_pLockWindow);
#else
  if(pInfo->bLock) {
    if(!m_pLockWindow)
      m_pLockWindow = hildon_banner_show_progress(NULL, NULL, pInfo->szMessage);

    hildon_banner_set_fraction(HILDON_BANNER(m_pLockWindow), pInfo->iPercent / 100.0);
  }
  else {
    if(m_pLockWindow)
      gtk_widget_destroy(m_pLockWindow);
    m_pLockWindow = 0;
  }
#endif

  // Keep the GTK interface responsive
  while(gtk_events_pending())
    gtk_main_iteration();
}
