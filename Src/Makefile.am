SUBDIRS = DasherCore Gtk

bin_PROGRAMS = dasher
dasher_SOURCES = Main.cc

LDADD =  Gtk/libdashergtk.a DasherCore/libdashercore.a

CXXFLAGS = $(GTKMM_CFLAGS) `gnome-config --cflags gnomemm` -I./DasherCore -I./Gtk 
LDFLAGS = $(GTKMM_LIBS) `gnome-config --libs gnomemm`
