LIBS = @INTLLIBS@ @LIBS@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = dasher

#if DOGTK2

SUBDIRS = Common DasherCore DasherIM Gtk2 Installer Win32
dasher_SOURCES = main.cc

AM_CXXFLAGS = \
	$(GTK2BUILD_CFLAGS) \
	-I$(srcdir)/Gtk2 \
	-I$(srcdir)/DasherCore \
	-I$(top_srcdir)/intl \
	-DPREFIX=\"$(PREFIX)\" \
	-DLIBDIR=\"$(libdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DPROGDATA=\"$(pkgdatadir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DPACKAGE_LOCALE_DIR=\"$(datadir)/locale\"

dasher_LDADD = \
	$(GTK2BUILD_LIBS) \
	-lexpat \
	$(POPT_LIBS) \
	$(GPELIB) \
	Common/libdashermisc.a \
	Gtk2/libdashergtk.a \
	Gtk2/libdashercontrol.a \
	DasherCore/libdashercore.a \
	DasherCore/libdasherprefs.a \
	DasherCore/LanguageModelling/libdasherlm.a \
	DasherCore/Alphabet/libdasheralphabet.a \
	Common/libdashermisc.a
#endif

#if DOQTE

#QTDIR=/usr/share/qte2

#SUBDIRS = DasherCore

#endif
