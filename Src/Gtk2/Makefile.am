LIBS = @INTLLIBS@ @LIBS@

noinst_LIBRARIES = libdashergtk.a

libdashergtk_a_SOURCES = dasher.cc dasher.h global.h settings_store.cc settings_store.h canvas.cc canvas.h edit.cc edit.h

libdashergtk_a_LIBADD = @SPEECH_SOURCES@ @A11Y_SOURCES@
libdashergtk_a_DEPENDENCIES = @SPEECH_SOURCES@ @A11Y_SOURCES@

AM_CXXFLAGS = -O2 $(GTK2_CFLAGS) $(GCONF_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) -I../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl
