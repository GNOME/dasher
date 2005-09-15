LIBS = @INTLLIBS@ @LIBS@

noinst_LIBRARIES = libdashergtk.a libdashercontrol.a
noinst_PROGRAMS = generate-schema

if WITH_GPE
gladefile = dashergpe.glade
settings_store = gpesettings_store.cc gpesettings_store.h
non_settings_store = 
else
gladefile = dasher.glade
settings_store = 
non_settings_store = gpesettings_store.cc gpesettings_store.h
endif

EXTRA_DIST = $(non_settings_store)

generate_schema_SOURCES = GenerateSchema.cpp

libdashergtk_a_SOURCES = dasher.cc dasher.h edit.cc edit.h accessibility.cc accessibility.h speech.cc speech.h fileops.cc fileops.h dasher.glade dashergpe.glade DasherControl.cpp Timer.cpp Menu.cc Menu.h Preferences.cpp FontDialogues.cpp Canvas.cpp GtkDasherControl.cpp PangoCache.cpp $(settings_store) dashergtktextview.c dashergtktextview.h AppSettings.cpp socket_input.cc

libdashergtk_a_LIBADD = @SPEECH_SOURCES@
libdashergtk_a_DEPENDENCIES = @SPEECH_SOURCES@

AM_CXXFLAGS = $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) $(wnck_CFLAGS) -DWNCK_I_KNOW_THIS_IS_UNSTABLE -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl 

AM_CFLAGS = $(GTK2_CFLAGS)

libdashercontrol_a_SOURCES = Timer.cpp Canvas.cpp GtkDasherControl.cpp PangoCache.cpp DasherControl.cpp socket_input.cc
#libdashercontrol_a_LIBADD = ../DasherCore/libdashercore.a ../DasherCore/LanguageModelling/libdasherlm.a ../DasherCore/Alphabet/libdasheralphabet.a
#libdashercontrol_a_LDFLAGS = -L../DasherCore -L../DasherCore/LanguageModelling -L../DasherCore/Alphabet -ldashercore -ldasherlm -ldasheralphabet

pkgdata_DATA = $(gladefile) 
