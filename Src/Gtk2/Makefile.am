LIBS = @INTLLIBS@ @LIBS@

noinst_LIBRARIES = libdashergtk.a
libdashergtk_a_SOURCES = dasher.cc dasher.h global.h settings_store.cc settings_store.h canvas.cc canvas.h edit.cc edit.h
CXXFLAGS = $(GTK2_CFLAGS) $(GCONF_CFLAGS) -I../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl
