LIBS = @INTLLIBS@ @LIBS@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

SUBDIRS = DasherCore Gtk2

bin_PROGRAMS = dasher
dasher_SOURCES = main.cc

LDADD =  Gtk2/libdashergtk.a DasherCore/libdashercore.a

CXXFLAGS = $(GTK2_CFLAGS) $(GCONF_CFLAGS) -I./Gtk2/ -I./DasherCore/ -I../intl -I$(top_srcdir)/intl -DPACKAGE_LOCALE_DIR=\"$(PREFIX)/share/locale\"
LDFLAGS = $(GTK2_LIBS) $(GCONF_LIBS) -lexpat
