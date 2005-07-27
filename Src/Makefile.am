LIBS = @INTLLIBS@ @LIBS@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = dasher

if DOGTK2

SUBDIRS = DasherCore Gtk2 Common Gtk2-Experimental
dasher_SOURCES = main.cc

AM_CXXFLAGS = $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) $(gthread_CFLAGS) $(wnck_CFLAGS) -I$(srcdir)/Gtk2/ -I$(srcdir)/DasherCore/ -I$(srcdir)/../intl -I$(top_srcdir)/intl -DPACKAGE_LOCALE_DIR=\"$(PREFIX)/share/locale\" -DPROGDATA=\"$(pkgdatadir)\"

dasher_LDADD = $(GTK2_LIBS) $(SETTINGS_LIBS) $(gnome_speech_LIBS) $(gnome_a11y_LIBS) $(glade_LIBS) $(gnome_LIBS) $(gthread_LIBS) $(wnck_LIBS) -lexpat $(X_LIBS) $(GPELIB) Gtk2/libdashergtk.a DasherCore/libdashercore.a DasherCore/LanguageModelling/libdasherlm.a DasherCore/Alphabet/libdasheralphabet.a DasherCore/View/libdasherview.a Gtk2/Menu.o Gtk2/Preferences.o

endif

if DOQTE

QTDIR=/usr/share/qte2

SUBDIRS = DasherCore Qt 

endif

