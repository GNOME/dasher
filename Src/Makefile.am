LIBS = @INTLLIBS@ @LIBS@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

if DOGTK2

SUBDIRS = DasherCore Gtk2
dasher_SOURCES = main.cc
LDADD =  Gtk2/libdashergtk.a DasherCore/libdashercore.a

CXXFLAGS = $(GTK2_CFLAGS) $(GCONF_CFLAGS) -I./Gtk2/ -I./DasherCore/ -I../intl -I$(top_srcdir)/intl -DPACKAGE_LOCALE_DIR=\"$(PREFIX)/share/locale\"
LDFLAGS = $(GTK2_LIBS) $(GCONF_LIBS) -lexpat

else

SUBDIRS = DasherCore Gtk
dasher_SOURCES = Main.cc
LDADD =  Gtk/libdashergtk.a DasherCore/libdashercore.a

CXXFLAGS = $(GTKMM_CFLAGS) -I./DasherCore -I./Gtk -I../intl -I$(top_srcdir)/intl
LDFLAGS = $(GTKMM_LIBS) -lexpat

endif



bin_PROGRAMS = dasher








