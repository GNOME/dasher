noinst_LIBRARIES = libdashermisc.a
libdashermisc_a_SOURCES = \
                AppSettingsData.h \
                AppSettingsHeader.h \
		Common.h \
		Hash.h \
		IOstreamDasherEdit.cc \
		IOstreamDasherEdit.h \
		MSVC_Unannoy.h \
		myassert.h \
		NoClones.h \
		Platform.h \
		Random.h \
		Trace.h \
		Allocators/PooledAlloc.h \
		Allocators/SimplePooledAlloc.h \
		Expat/lib/expat.h \
		Platform/stdminmax.h \
		Types/int.h \
		Types/int32.h \
		Types/int64.h

AM_CXXFLAGS = $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) $(wnck_CFLAGS) -DWNCK_I_KNOW_THIS_IS_UNSTABLE -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl 

EXTRA_DIST = myassert.h Platform.h Common.h MSVC_Unannoy.h Types/int.h NoClones.h Types/int32.h Types/int64.h Expat/lib/expat.h
