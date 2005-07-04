noinst_LIBRARIES = libdashermisc.a
libdashermisc_a_SOURCES = IOstreamDasherEdit.h IOstreamDasherEdit.cc

AM_CXXFLAGS = -O2 $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) $(wnck_CFLAGS) -DWNCK_I_KNOW_THIS_IS_UNSTABLE -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl 

EXTRA_DIST = assert.h Platform.h Common.h MSVC_Unannoy.h Types/int.h NoClones.h Types/int32.h Types/int64.h Expat/lib/expat.h
