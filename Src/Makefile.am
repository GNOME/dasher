LIBS = @INTLLIBS@ @LIBS@

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

SUBDIRS = DasherCore Gtk

bin_PROGRAMS = dasher
dasher_SOURCES = Main.cc

LDADD =  Gtk/libdashergtk.a DasherCore/libdashercore.a

CXXFLAGS = $(GTKMM_CFLAGS) -I./DasherCore -I./Gtk -I../intl -I$(top_srcdir)/intl
LDFLAGS = $(GTKMM_LIBS) -lexpat
