LIBS = @INTLLIBS@ @LIBS@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = dasher

if WITH_GPE
SETTINGS_LIBS = -lgpewidget -lXsettings-client -lXsettings
else
SETTINGS_CFLAGS = $(GCONF_CFLAGS)
SETTINGS_LIBS = $(GCONF_LIBS)
endif

if DOGTK2

SUBDIRS = DasherCore Gtk2 Common
dasher_SOURCES = main.cc
LDADD =  Gtk2/libdashergtk.a DasherCore/libdashercore.a

AM_CXXFLAGS = -O2 $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) $(gthread_CFLAGS) -I$(srcdir)/Gtk2/ -I$(srcdir)/DasherCore/ -I$(srcdir)/../intl -I$(top_srcdir)/intl -DPACKAGE_LOCALE_DIR=\"$(PREFIX)/share/locale\" -DPROGDATA=\"$(pkgdatadir)\"
AM_LDFLAGS = $(GTK2_LIBS) $(SETTINGS_LIBS) $(gnome_speech_LIBS) $(gnome_a11y_LIBS) $(glade_LIBS) $(gnome_LIBS) $(gthread_LIBS) -lexpat -L/usr/X11R6/lib -lXtst -lX11 -export-dynamic $(GPELIB)

endif

if DOQTE

QTDIR=/usr/share/qt3
dasher_SOURCES = QMain.cc
AM_CXXFLAGS = -O2 -fno-rtti -IDasherCore -I$(QTDIR)/include -DPROGDATA=\"$(pkgdatadir)/\" -lqt-mt -lqui
LDADD = Qt/libdasherqt.a DasherCore/libdashercore.a

SUBDIRS = DasherCore Qt

endif

