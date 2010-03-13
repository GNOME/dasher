noinst_LIBRARIES = libdashermisc.a
libdashermisc_a_SOURCES = \
                AppSettingsData.h \
                AppSettingsHeader.h \
		Common.h \
		Hash.h \
		I18n.h \
		MSVC_Unannoy.h \
		myassert.h \
		mydebug.h \
		ModuleSettings.h \
		NoClones.h \
		Platform.h \
		Random.h \
                Trace.cpp \
		Trace.h \
		Allocators/PooledAlloc.h \
		Allocators/SimplePooledAlloc.h \
		Platform/stdminmax.h \
		Types/int.h \
		Types/int32.h \
		Types/int64.h

#		IOstreamDasherEdit.cc   I have no idea what this does, but it's broken now
#		IOstreamDasherEdit.h 

AM_CXXFLAGS = -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(gnome_CFLAGS)

EXTRA_DIST = Common.vcproj Common_vc80.vcproj
