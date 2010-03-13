SUBDIRS = LanguageModelling

noinst_LIBRARIES = libdashercore.a libdasherprefs.a

libdasherprefs_a_SOURCES = \
		Parameters.h \
		SettingsStore.cpp \
		SettingsStore.h 

libdashercore_a_SOURCES = \
		Alphabet/AlphIO.cpp \
		Alphabet/AlphIO.h \
		Alphabet/Alphabet.cpp \
		Alphabet/Alphabet.h \
		Alphabet/AlphabetMap.cpp \
		Alphabet/AlphabetMap.h \
		Alphabet/GroupInfo.h \
		AlternatingDirectMode.cpp \
		AlternatingDirectMode.h \
		ActionButton.cpp \
		ActionButton.h \
		AlphabetManager.cpp \
		AlphabetManager.h \
		AutoSpeedControl.cpp \
		AutoSpeedControl.h \
		BasicLog.cpp \
		BasicLog.h \
		ButtonMode.cpp \
		ButtonMode.h \
		ButtonMultiPress.h \
		ButtonMultiPress.cpp \
		CircleStartHandler.cpp \
		CircleStartHandler.h \
		ClickFilter.cpp \
		ClickFilter.h \
		ColourIO.cpp \
		ColourIO.h \
		CompassMode.cpp \
		CompassMode.h \
		ControlManager.cpp \
		ControlManager.h \
		ConversionHelper.cpp \
		ConversionHelper.h \
		ConversionManager.cpp \
		ConversionManager.h \
		DasherButtons.cpp \
		DasherButtons.h \
		DasherComponent.cpp \
		DasherComponent.h \
		DasherGameMode.cpp \
		DasherGameMode.h \
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
		FrameRate.cpp \
		GameLevel.cpp \
		GameLevel.h \
		GameMessages.h \
		GameScorer.cpp \
		GameScorer.h \
		GameStatistics.h \
		GnomeSettingsStore.cpp \
		GnomeSettingsStore.h \
		InputFilter.h \
		MandarinAlphMgr.cpp \
		MandarinAlphMgr.h \
		MemoryLeak.cpp \
		MemoryLeak.h \
		ModuleManager.cpp \
		ModuleManager.h \
		NodeCreationManager.cpp \
		NodeCreationManager.h \
		ExpansionPolicy.cpp \
		ExpansionPolicy.h \
		OneButtonDynamicFilter.cpp \
		OneButtonDynamicFilter.h \
		OneButtonFilter.cpp \
		OneButtonFilter.h \
		OneDimensionalFilter.cpp \
		OneDimensionalFilter.h \
		SCENode.cpp \
		SCENode.h \
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
		Trainer.cpp \
		Trainer.h \
		TrainingHelper.cpp \
		TrainingHelper.h \
		TwoBoxStartHandler.cpp \
		TwoBoxStartHandler.h \
		TwoButtonDynamicFilter.cpp \
		TwoButtonDynamicFilter.h \
		TwoPushDynamicFilter.cpp \
		TwoPushDynamicFilter.h \
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
		XMLUtil.cpp \
		XMLUtil.h

libdashercore_a_LIBADD = @CHINESE_SOURCES@ @JAPANESE_SOURCES@
libdashercore_a_DEPENDENCIES = @CHINESE_SOURCES@ @JAPANESE_SOURCES@

EXTRA_libdashercore_a_SOURCES = \
		CannaConversionHelper.cpp \
		CannaConversionHelper.h \
		PinYinConversionHelper.cpp \
		PinYinConversionHelper.h \
		PinyinParser.cpp \
		PinyinParser.h

AM_CXXFLAGS = -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(gnome_CFLAGS)

EXTRA_DIST = \
		LanguageModelling/BigramLanguageModel.cpp \
		LanguageModelling/BigramLanguageModel.h \
		LanguageModelling/KanjiConversionIME.cpp \
		LanguageModelling/KanjiConversionIME.h \
		DasherCore.vcproj \
		DasherCore_vc80.vcproj \
		IMEConversionHelper.cpp \
		IMEConversionHelper.h 
