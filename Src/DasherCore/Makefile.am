SUBDIRS = LanguageModelling Alphabet

noinst_LIBRARIES = libdashercore.a
libdashercore_a_SOURCES = AlphabetManager.cpp \
                          AlphabetManager.h \
                          AlphabetManagerFactory.cpp \
                          AlphabetManagerFactory.h \
                          ColourIO.cpp \
                          ColourIO.h \
                          ControlManager.cpp \
                          ControlManager.h \
                          ControlManagerFactory.cpp \
                          ControlManagerFactory.h \
                          CustomColours.cpp \
                          CustomColours.h \
                          DashEdit.cpp \
                          DashEdit.h \
                          DasherAppInterface.h \
                          DasherButtons.cpp \
                          DasherButtons.h \
                          DasherComponent.cpp \
                          DasherComponent.h \
                          DasherGameMode.cpp \
                          DasherGameMode.h \
                          DasherInput.h \
                          DasherInterface.h \
                          DasherInterfaceBase.cpp \
                          DasherInterfaceBase.h \
                          DasherModel.cpp \
                          DasherModel.h \
                          DasherNode.cpp \
                          DasherNode.h \
                          DasherScreen.h \
                          DasherSettingsInterface.cpp \
                          DasherSettingsInterface.h \
                          DasherTypes.h \
                          DasherView.cpp \
                          DasherView.h \
                          DasherView.inl \
                          DasherViewSquare.cpp \
                          DasherViewSquare.h \
                          DasherViewSquare.inl \
                          DasherWidgetInterface.h \
                          DelayedDraw.cpp \
                          Event.h \
                          EventHandler.cpp \
                          EventHandler.h \
                          FileLogger.cpp \
                          FileLogger.h \
                          FrameRate.h \
                          GnomeSettingsStore.cpp \
                          GnomeSettingsStore.h \
                          MemoryLeak.cpp \
                          MemoryLeak.h \
                          NodeManager.h \
                          NodeManagerFactory.h \
                          Parameters.h \
                          SettingsStore.cpp \
                          SettingsStore.h \
                          SimpleTimer.cpp \
                          SimpleTimer.h \
                          SocketInput.cpp \
                          SocketInput.h \
                          SocketInputBase.cpp \
                          SocketInputBase.h \
                          TimeSpan.cpp \
                          TimeSpan.h \
                          UserLocation.cpp \
                          UserLocation.h \
                          UserLog.cpp \
                          UserLog.h \
                          UserLogParam.cpp \
                          UserLogParam.h \
                          UserLogTrial.cpp \
                          UserLogTrial.h \
                          View/DelayedDraw.h \
                          XMLUtil.cpp \
                          XMLUtil.h

AM_CXXFLAGS = $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) $(wnck_CFLAGS) -DWNCK_I_KNOW_THIS_IS_UNSTABLE -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl 

EXTRA_DIST = \
		Win32/DasherInterface.cpp \
		Win32/DasherInterface.h \
		Win32/SocketInput.cpp \
		Win32/SocketInput.h \
		Win32/WinCommon.h \
		Win32/WinHelper.cpp \
		Win32/WinHelper.h \
		Win32/WinLocalisation.cpp \
		Win32/WinLocalisation.h \
		Win32/WinOptions.cpp \
		Win32/WinOptions.h \
		Win32/WinUTF8.cpp \
		Win32/WinUTF8.h \
		Win32/WinWrap.cpp \
		Win32/WinWrap.h \
		LanguageModelling/BigramLanguageModel.cpp \
		LanguageModelling/BigramLanguageModel.h \
		LanguageModelling/KanjiConversionIME.cpp \
		LanguageModelling/KanjiConversionIME.h \
		DasherCore.vcproj \
		DasherCore_vc70.vcproj \
		DasherCore_vc71.vcproj
