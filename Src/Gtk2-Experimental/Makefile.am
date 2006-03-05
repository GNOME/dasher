LIBS = @INTLLIBS@ @LIBS@

noinst_PROGRAMS = dasher_com_aid

SETTINGS_CFLAGS = $(GCONF_CFLAGS)
SETTINGS_LIBS = $(GCONF_LIBS)

dasher_com_aid_SOURCES = DasherComAid.cpp

AM_CXXFLAGS = $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) $(gthread_CFLAGS) $(wnck_CFLAGS) -I$(srcdir)/../Gtk2 -I$(srcdir)/../Common -I$(srcdir)/../DasherCore -I$(srcdir)/../intl -I$(top_srcdir)/intl -DPACKAGE_LOCALE_DIR=\"$(PREFIX)/share/locale\" -DPROGDATA=\"$(pkgdatadir)\"

dasher_com_aid_LDFLAGS = $(GTK2_LIBS) $(SETTINGS_LIBS) $(gnome_speech_LIBS) $(gnome_a11y_LIBS) $(glade_LIBS) $(gnome_LIBS) $(gthread_LIBS) $(wnck_LIBS) -lexpat $(X_LIBS) -L../Gtk2 -L../Common -L../DasherCore -L../DasherCore/LanguageModelling -L../DasherCore/Alphabet -ldashercontrol -ldashercore -ldasherlm -ldasheralphabet -ldashermisc
