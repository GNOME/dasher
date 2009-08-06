LIBS = @INTLLIBS@ @LIBS@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = dasher

#if DOGTK2

SUBDIRS = Common DasherCore DasherIM Gtk2 Installer InstallerTobii InstallerW2K Win32
dasher_SOURCES = main.cc

AM_CXXFLAGS = \
	-I$(srcdir)/Gtk2 \
	-I$(srcdir)/DasherCore \
	-I$(top_srcdir)/intl \
	$(GTK2BUILD_CFLAGS) \
	-DPREFIX=\"$(PREFIX)\" \
	-DLIBDIR=\"$(libdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DPROGDATA=\"$(pkgdatadir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DPACKAGE_LOCALE_DIR=\"$(datadir)/locale\"

dasher_LDADD = \
	Common/libdashermisc.a \
	Gtk2/libdashergtk.la \
	Gtk2/libdashercontrol.la \
	DasherCore/libdashercore.a \
	DasherCore/libdasherprefs.a \
	DasherCore/LanguageModelling/libdasherlm.a \
	Common/libdashermisc.a \
	$(GTK2BUILD_LIBS) \
	-lexpat \
	$(POPT_LIBS) \
	$(GPELIB)
#endif

#if DOQTE

#QTDIR=/usr/share/qte2

#SUBDIRS = DasherCore

#endif
