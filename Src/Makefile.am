LIBS = @INTLLIBS@ @LIBS@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

if DOGTK2

SUBDIRS = DasherCore Gtk2
dasher_SOURCES = main.cc
LDADD =  Gtk2/libdashergtk.a DasherCore/libdashercore.a

CXXFLAGS = -O2 $(GTK2_CFLAGS) $(GCONF_CFLAGS) $(gnome_speech_CFLAGS) -I./Gtk2/ -I./DasherCore/ -I../intl -I$(top_srcdir)/intl -DPACKAGE_LOCALE_DIR=\"$(PREFIX)/share/locale\"
LDFLAGS = $(GTK2_LIBS) $(GCONF_LIBS) $(gnome_speech_LIBS) -lexpat

endif

if DOGTKMM

SUBDIRS = DasherCore Gtk
dasher_SOURCES = Main.cc
LDADD =  Gtk/libdashergtk.a DasherCore/libdashercore.a

CXXFLAGS = -O2 $(GTKMM_CFLAGS) -I./DasherCore -I./Gtk -I../intl -I$(top_srcdir)/intl
LDFLAGS = $(GTKMM_LIBS) -lexpat

endif

if DOQTE

QTDIR=/usr/share/qte2

SUBDIRS = DasherCore Common Qt
dasher_SOURCES = QMain.cc
LDADD =  Qt/libdasherqt.a DasherCore/libdashercore.a Common/libdashermisc.a

CXXFLAGS = -O2 -DQWS -fno-rtti -I./DasherCore -I../intl -I$(top_srcdir)/intl -I$(QTDIR)/include
LDFLAGS = -lqte -ljpeg -lexpat

endif

bin_PROGRAMS = dasher
