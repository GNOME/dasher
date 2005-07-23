LIBS = @INTLLIBS@ @LIBS@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = dasher

if DOGTK2

SUBDIRS = DasherCore Gtk2 Common

dasher_SOURCES = main.cc

AM_CXXFLAGS = $(GTK2BUILD_CFLAGS) -I$(srcdir)/Gtk2/ -I$(srcdir)/DasherCore/ -I$(top_srcdir)/intl -DPACKAGE_LOCALE_DIR=\"$(PREFIX)/share/locale\" -DPROGDATA=\"$(pkgdatadir)\"

dasher_LDADD = $(GTK2BUILD_LIBS) -lexpat Gtk2/libdashergtk.a DasherCore/libdashercore.a DasherCore/LanguageModelling/libdasherlm.a DasherCore/Alphabet/libdasheralphabet.a

endif

if DOQTE

QTDIR=/usr/share/qte2

SUBDIRS = DasherCore Qt

endif

