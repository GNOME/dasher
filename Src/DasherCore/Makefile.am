SUBDIRS = LanguageModelling Alphabet

noinst_LIBRARIES = libdashercore.a
libdashercore_a_SOURCES = ColourIO.cpp CustomColours.cpp CustomColours.h Context.h DasherInterfaceBase.cpp DasherSettingsInterface.cpp DasherViewSquare.h SettingsStore.cpp AlphIO.h ColourIO.h DasherScreen.h DasherSettingsInterface.h DasherViewSquare.inl SettingsStore.h CustomAlphabet.h DasherModel.cpp DasherTypes.h DasherWidgetInterface.h FrameRate.h Alphabet.h DashEdit.h DasherModel.h DasherView.cpp  DasherAppInterface.h DasherNode.cpp DasherView.h AlphabetMap.h DasherNode.h DasherView.inl  DasherViewSquare.cpp EventHandler.cpp GnomeSettingsStore.cpp AlphabetManager.cpp AlphabetManagerFactory.cpp ControlManager.cpp ControlManagerFactory.cpp UserLocation.cpp UserLog.cpp UserLogTrial.cpp TimeSpan.cpp SimpleTimer.cpp FileLogger.cpp UserLogParam.cpp DasherGameMode.cpp DasherComponent.cpp DelayedDraw.cpp XMLUtil.cpp

AM_CXXFLAGS = $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) $(wnck_CFLAGS) -DWNCK_I_KNOW_THIS_IS_UNSTABLE -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl 
