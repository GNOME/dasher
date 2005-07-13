LIBS = @INTLLIBS@ @LIBS@

noinst_LIBRARIES = libdashergtk.a

if WITH_GPE
gladefile = dashergpe.glade
settings_store = gpesettings_store.cc gpesettings_store.h
non_settings_store = 
else
gladefile = dasher.glade
settings_store = 
non_settings_store = gpesettings_store.cc gpesettings_store.h
SETTINGS_CFLAGS=$(GCONF_CFLAGS)
endif

EXTRA_DIST = $(non_settings_store)

libdashergtk_a_SOURCES = dasher.cc dasher.h canvas.cc canvas.h edit.cc edit.h accessibility.cc accessibility.h speech.cc speech.h fileops.cc fileops.h dasher.glade dashergpe.glade DasherControl.cpp Timer.cpp Menu.cc Menu.h Preferences.cpp FontDialogues.cpp $(settings_store)

libdashergtk_a_LIBADD = @SPEECH_SOURCES@
libdashergtk_a_DEPENDENCIES = @SPEECH_SOURCES@

AM_CXXFLAGS = -O2 $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) $(wnck_CFLAGS) -DWNCK_I_KNOW_THIS_IS_UNSTABLE -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl 

pkgdata_DATA = $(gladefile) 
