LIBS = @INTLLIBS@ @LIBS@

noinst_LIBRARIES = libdashergtk.a libdashercontrol.a
noinst_PROGRAMS = generate-schema

if WITH_GPE
gladefile = dashergpe.glade
settings_store = gpesettings_store.cc gpesettings_store.h

else

if WITH_MAEMO
gladefile = dashermaemo.glade
else
gladefile = dasher.glade
endif

settings_store = 
endif

EXTRA_DIST = \
			 accessibility.cc \
			 accessibility.h \
                         dasher.glade \
                         dashergpe.glade \
                         dashermaemo.glade

generate_schema_SOURCES = GenerateSchema.cpp

libdashergtk_a_SOURCES = AppSettings.cpp \
                         AppSettings.h \
                         Canvas.cpp \
                         Canvas.h \
                         DasherControl.cpp \
                         DasherControl.h \
                         FileDialogues.cpp \
                         FontDialogues.cpp \
                         FontDialogues.h \
                         GenerateSchema.cpp \
                         GtkDasherControl.cpp \
                         GtkDasherControl.h \
                         Menu.cc \
                         Menu.h \
                         MouseInputFactory.cpp \
                         MouseInputFactory.h \
			 Output.cpp \
			 Output.h \
                         PangoCache.cpp \
                         PangoCache.h \
                         Preferences.cpp \
                         Preferences.h \
                         Timer.cpp \
                         Timer.h \
                         dasher.cc \
                         dasher.h \
                         edit.cc \
                         edit.h \
                         fileops.cc \
                         fileops.h \
                         mouse_input.h \
                         speech.cc \
                         speech.h \
                         $(settings_store)

libdashergtk_a_LIBADD = @SPEECH_SOURCES@
libdashergtk_a_DEPENDENCIES = @SPEECH_SOURCES@

AM_CXXFLAGS = $(GTK2BUILD_CFLAGS) -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl 

AM_CFLAGS = $(GTK2_CFLAGS)

libdashercontrol_a_SOURCES = Timer.cpp Canvas.cpp GtkDasherControl.cpp PangoCache.cpp DasherControl.cpp
#libdashercontrol_a_LIBADD = ../DasherCore/libdashercore.a ../DasherCore/LanguageModelling/libdasherlm.a ../DasherCore/Alphabet/libdasheralphabet.a
#libdashercontrol_a_LDFLAGS = -L../DasherCore -L../DasherCore/LanguageModelling -L../DasherCore/Alphabet -ldashercore -ldasherlm -ldasheralphabet

pkgdata_DATA = dasher.glade dashermaemo.glade
