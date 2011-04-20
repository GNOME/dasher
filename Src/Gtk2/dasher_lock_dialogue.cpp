#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "GtkDasherControl.h"
#include "dasher.h"
#include "dasher_lock_dialogue.h"

#ifdef WITH_MAEMO
#include <hildon-widgets/hildon-banner.h>
#endif
#include <iostream>

// TODO: Make this a real class

GtkWindow *m_pLockWindow;
GtkProgressBar *m_pLockProgress;
GtkLabel *m_pLockMessage;
unsigned long lastTime;

void dasher_lock_dialogue_new(GtkBuilder *pXML, GtkWindow *pMainWindow) {
#ifndef WITH_MAEMO
  m_pLockWindow = GTK_WINDOW(gtk_builder_get_object(pXML, "lock_window"));
  m_pLockProgress = GTK_PROGRESS_BAR(gtk_builder_get_object(pXML, "lock_progress"));
  m_pLockMessage = GTK_LABEL(gtk_builder_get_object(pXML, "lock_message"));
  lastTime=0;

  gtk_widget_hide(GTK_WIDGET(m_pLockWindow));
  
  gtk_window_set_transient_for(m_pLockWindow, pMainWindow);
#else
  m_pLockWindow = 0;
#endif
}

extern "C" void on_lock_info(GtkDasherControl *pDasherControl, gpointer pLockInfo, gpointer pUserData) {
  // TODO: signals are connected after the Dasher control is created,
  // which is too late to receive notification about intial training
  // etc.
  DasherLockInfo *pInfo = (DasherLockInfo *)pLockInfo;

  //Whether to perform a full refresh of GUI components (too expensive to do every time):
  bool bFullRefresh(pInfo->time-lastTime > 100);
#ifndef WITH_MAEMO
  GtkWidget *pWinWidget(GTK_WIDGET(m_pLockWindow));
  if(pInfo->iPercent!=-1) {
    gtk_label_set_text(m_pLockMessage, pInfo->szMessage);
    gtk_progress_bar_set_fraction(m_pLockProgress, pInfo->iPercent / 100.0);
    if (!gtk_widget_get_visible(pWinWidget)) {
      bFullRefresh = true;
      gtk_widget_show(pWinWidget);
    }
  }
  else if (gtk_widget_get_visible(pWinWidget)) {
    bFullRefresh = true;
    gtk_widget_hide(pWinWidget);
  }
#else
  if(pInfo->iPercent!=-1) {
    if(!m_pLockWindow) {
      bFullRefresh = true;
      m_pLockWindow = hildon_banner_show_progress(NULL, NULL, pInfo->szMessage);
    }
    hildon_banner_set_fraction(HILDON_BANNER(m_pLockWindow), pInfo->iPercent / 100.0);
  }
  else {
    if(m_pLockWindow) {
      bFullRefresh = true;
      gtk_widget_destroy(GTK_WIDGET(m_pLockWindow));
    }
    m_pLockWindow = 0;
  }
#endif

  if (bFullRefresh) {
    //This takes too long, on some hardware / software versions, to perform every time
    // (or else training takes much longer than without progress display!), but we do
    // it every so often to ensure the progress display is visible onscreen.
    // (I've measured it at 40+ iterations upon show() of progress dialogue! and averages 5-6)
    while(gtk_events_pending())
        gtk_main_iteration();
      lastTime=pInfo->time;
  }
}
