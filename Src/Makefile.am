SUBDIRS = DasherCore Gtk

bin_PROGRAMS = dasher
dasher_SOURCES = Main.cc

LDADD = Gtk/libdashergtk.a DasherCore/libdashercore.a

CXXFLAGS = `gtkmm-config --cflags` -I./DasherCore -I./Gtk 
LDFLAGS = `gtkmm-config --cflags --libs` -lexpat
