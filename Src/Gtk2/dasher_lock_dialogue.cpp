#include "GtkDasherControl.h"
#include "dasher.h"
#include "dasher_lock_dialogue.h"

#include <iostream>

GtkWidget *m_pLockWindow;
GtkWidget *m_pLockProgress;
GtkWidget *m_pLockMessage;

void dasher_lock_dialogue_new(GladeXML *pGladeXML) {
  m_pLockWindow = glade_xml_get_widget(pGladeXML, "lock_window");
  m_pLockProgress = glade_xml_get_widget(pGladeXML, "lock_progress");
  m_pLockMessage = glade_xml_get_widget(pGladeXML, "lock_message");
}

extern "C" void on_lock_info(GtkDasherControl *pDasherControl, gpointer pLockInfo, gpointer pUserData) {
  // TODO: signals are connected after the Dasher control is created,
  // which is too late to receive notification about intial training
  // etc.

  DasherLockInfo *pInfo = (DasherLockInfo *)pLockInfo;

  gtk_label_set_text(GTK_LABEL(m_pLockMessage), pInfo->szMessage);
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(m_pLockProgress), pInfo->iPercent / 100.0);

  if(pInfo->bLock)
    gtk_widget_show(m_pLockWindow);
  else
    gtk_widget_hide(m_pLockWindow);

  // Keep the GTK interface responsive
  while(gtk_events_pending())
    gtk_main_iteration();
}
