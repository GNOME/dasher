SUBDIRS = DasherCore Gtk

bin_PROGRAMS = dasher
dasher_SOURCES = Main.cc

LDADD = Gtk/libdashergtk.a DasherCore/libdashercore.a

CXXFLAGS = `gtkmm-config --cflags --libs` -I./DasherCore -I./Gtk -lexpat