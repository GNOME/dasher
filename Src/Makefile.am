SUBDIRS = DasherCore Gtk

bin_PROGRAMS = dasher
dasher_SOURCES = Main.cc

LDADD =  Gtk/libdashergtk.a DasherCore/libdashercore.a

CXXFLAGS = $(GTKMM_CFLAGS) -I./DasherCore -I./Gtk 
LDFLAGS = $(GTKMM_LIBS) -lexpat
