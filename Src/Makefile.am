LIBS = @INTLLIBS@ @LIBS@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

bin_PROGRAMS = dasher

#if DOGTK

SUBDIRS = Common DasherCore DasherIM Gtk2 Installer InstallerTobii InstallerW2K Win32
dasher_SOURCES = main.cc

AM_CXXFLAGS = \
	-I$(srcdir)/Gtk2 \
	-I$(srcdir)/DasherCore \
	$(GTKBUILD_CFLAGS) \
	$(SPEECH_CPPFLAGS) \
	-DPREFIX=\"$(PREFIX)\" \
	-DLIBDIR=\"$(libdir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DPROGDATA=\"$(pkgdatadir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DPACKAGE_LOCALE_DIR=\"$(datadir)/locale\"
if USE_CSPI
AM_CXXFLAGS += @CSPI_CFLAGS@
endif

dasher_LDADD = \
	Common/libdashermisc.a \
	Gtk2/libdashergtk.la \
	Gtk2/libdashercontrol.la \
	DasherCore/libdashercore.a \
	DasherCore/libdasherprefs.a \
	DasherCore/LanguageModelling/libdasherlm.a \
	$(GTKBUILD_LIBS) \
	$(SPEECH_LIBS) \
	-lexpat
#endif

#if DOQTE

#QTDIR=/usr/share/qte2

#SUBDIRS = DasherCore

#endif
