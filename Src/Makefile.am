LIBS = @INTLLIBS@ @LIBS@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = dasher

if DOGTK2

SUBDIRS = DasherCore Gtk2
dasher_SOURCES = main.cc
LDADD =  Gtk2/libdashergtk.a DasherCore/libdashercore.a

AM_CXXFLAGS = -O2 $(GTK2_CFLAGS) $(GCONF_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) -I./Gtk2/ -I./DasherCore/ -I../intl -I$(top_srcdir)/intl -DPACKAGE_LOCALE_DIR=\"$(PREFIX)/share/locale\" -DPROGDATA=\"$(pkgdatadir)\"
LDFLAGS = $(GTK2_LIBS) $(GCONF_LIBS) $(gnome_speech_LIBS) $(gnome_a11y_LIBS) $(glade_LIBS) -lexpat -L/usr/X11R6/lib -lXtst -export-dynamic

endif

if DOGTKMM

SUBDIRS = DasherCore Gtk
dasher_SOURCES = Main.cc
LDADD =  Gtk/libdashergtk.a DasherCore/libdashercore.a

AM_CXXFLAGS = -O2 $(GTKMM_CFLAGS) -I./DasherCore -I./Gtk -I../intl -I$(top_srcdir)/intl
LDFLAGS = $(GTKMM_LIBS) -lexpat

endif

if DOQTE

QTDIR=/usr/share/qte2

SUBDIRS = DasherCore Qt

endif

