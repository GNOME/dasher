SUBDIRS = LanguageModelling Alphabet

noinst_LIBRARIES = libdashercore.a
libdashercore_a_SOURCES = \
		ActionButton.cpp \
		ActionButton.h \
		AlphabetManager.cpp \
		AlphabetManager.h \
                AlphabetManagerFactory.cpp \
		AlphabetManagerFactory.h \
		AutoSpeedControl.cpp \
		AutoSpeedControl.h \
		BasicLog.cpp \
		BasicLog.h \
		CircleStartHandler.cpp \
		CircleStartHandler.h \
		ClickFilter.cpp \
		ClickFilter.h \
		ColourIO.cpp \
		ColourIO.h \
		ControlManager.cpp \
		ControlManager.h \
		ControlManagerFactory.cpp \
		ControlManagerFactory.h \
		ConversionHelper.h \
		ConversionManager.cpp \
		ConversionManager.h \
		ConversionManagerFactory.cpp \
		ConversionManagerFactory.h \
		DasherButtons.cpp \
		DasherButtons.h \
		DasherComponent.cpp \
		DasherComponent.h \
		DasherInput.h \
		DasherInterfaceBase.cpp \
		DasherInterfaceBase.h \
		DasherModel.cpp \
		DasherModel.h \
		DasherModule.cpp \
		DasherModule.h \
		DasherNode.cpp \
		DasherNode.h \
		DasherScreen.h \
		DasherTypes.h \
		DasherView.cpp \
		DasherView.h \
		DasherView.inl \
		DasherViewSquare.cpp \
		DasherViewSquare.h \
		DasherViewSquare.inl \
		DefaultFilter.cpp \
		DefaultFilter.h \
		DelayedDraw.cpp \
		DynamicFilter.h \
		DynamicFilter.cpp \
		Event.h \
		EventHandler.cpp \
		EventHandler.h \
		EyetrackerFilter.cpp \
		EyetrackerFilter.h \
		FileLogger.cpp \
		FileLogger.h \
		FrameRate.h \
		GnomeSettingsStore.cpp \
		GnomeSettingsStore.h \
		InputFilter.h \
		MemoryLeak.cpp \
		MemoryLeak.h \
		ModuleFactory.h \
		ModuleManager.cpp \
		ModuleManager.h \
		NodeCreationManager.cpp \
		NodeCreationManager.h \
		NodeManager.h \
		NodeManagerFactory.h \
		OneDimensionalFilter.cpp \
		OneDimensionalFilter.h \
		Parameters.h \
		SCENode.cpp \
		SCENode.h \
		SettingsStore.cpp \
		SettingsStore.h \
		SimpleTimer.cpp \
		SimpleTimer.h \
		SocketInput.cpp \
		SocketInput.h \
		SocketInputBase.cpp \
		SocketInputBase.h \
		StartHandler.h \
		StylusFilter.cpp \
		StylusFilter.h \
		TimeSpan.cpp \
		TimeSpan.h \
		TwoBoxStartHandler.cpp \
		TwoBoxStartHandler.h \
		TwoButtonDynamicFilter.cpp \
		TwoButtonDynamicFilter.h \
		UserButton.cpp \
		UserButton.h \
		UserLocation.cpp \
		UserLocation.h \
		UserLog.cpp \
		UserLog.h \
		UserLogBase.h \
		UserLogParam.cpp \
		UserLogParam.h \
		UserLogTrial.cpp \
		UserLogTrial.h \
		View/DelayedDraw.h \
		WrapperFactory.cpp \
		WrapperFactory.h \
		XMLUtil.cpp \
		XMLUtil.h

libdashercore_a_LIBADD = @CHINESE_SOURCES@ @JAPANESE_SOURCES@
libdashercore_a_DEPENDENCIES = @CHINESE_SOURCES@ @JAPANESE_SOURCES@

EXTRA_libdashercore_a_SOURCES = \
		CannaConversionHelper.cpp \
		CannaConversionHelper.h \
		PinYinConversionHelper.cpp \
		PinYinConversionHelper.h

AM_CXXFLAGS = $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) $(wnck_CFLAGS) -DWNCK_I_KNOW_THIS_IS_UNSTABLE -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl 

EXTRA_DIST = \
		LanguageModelling/BigramLanguageModel.cpp \
		LanguageModelling/BigramLanguageModel.h \
		LanguageModelling/KanjiConversionIME.cpp \
		LanguageModelling/KanjiConversionIME.h \
		DasherCore.vcproj \
		DasherCore_vc70.vcproj \
		DasherCore_vc71.vcproj \
		DasherCore_vc80.vcproj \
		IMEConversionHelper.cpp \
		IMEConversionHelper.h 
