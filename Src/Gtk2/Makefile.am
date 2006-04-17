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
                         dasher.glade \
                         dashergpe.glade \
                         dashermaemo.glade

generate_schema_SOURCES = GenerateSchema.cpp

libdashergtk_a_SOURCES = Canvas.cpp \
                         Canvas.h \
                         DasherAppSettings.cpp \
                         DasherAppSettings.h \
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
                         PangoCache.cpp \
                         PangoCache.h \
                         Preferences.cpp \
                         Preferences.h \
                         Timer.cpp \
                         Timer.h \
			 accessibility.cc \
			 accessibility.h \
                         dasher.cc \
                         dasher.h \
                         dasher_action.cpp \
                         dasher_action.h \
                         dasher_action_copy.cpp \
                         dasher_action_copy.h \
                         dasher_action_keyboard.cpp \
                         dasher_action_keyboard.h \
                         dasher_action_keyboard_maemo.cpp \
                         dasher_action_keyboard_maemo.h \
                         dasher_action_script.cpp \
                         dasher_action_script.h \
                         dasher_action_speech.cpp \
                         dasher_action_speech.h \
                         dasher_buffer_set.cpp \
                         dasher_buffer_set.h \
                         dasher_editor.cpp \
                         dasher_editor.h \
                         dasher_external_buffer.cpp \
                         dasher_external_buffer.h \
                         dasher_internal_buffer.cpp \
                         dasher_internal_buffer.h \
			 dasher_lock_dialogue.cpp \
			 dasher_maemo_helper.cpp \
			 dasher_maemo_helper.h \
                         dasher_main.cpp \
                         dasher_main.h \
                         edit.cc \
                         edit.h \
                         fileops.cc \
                         fileops.h \
                         mouse_input.h \
                         $(settings_store)

libdashergtk_a_LIBADD = @SPEECH_SOURCES@
libdashergtk_a_DEPENDENCIES = @SPEECH_SOURCES@

AM_CXXFLAGS = $(GTK2BUILD_CFLAGS) -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl 

AM_CFLAGS = $(GTK2_CFLAGS)

libdashercontrol_a_SOURCES = Timer.cpp Canvas.cpp GtkDasherControl.cpp PangoCache.cpp DasherControl.cpp
#libdashercontrol_a_LIBADD = ../DasherCore/libdashercore.a ../DasherCore/LanguageModelling/libdasherlm.a ../DasherCore/Alphabet/libdasheralphabet.a
#libdashercontrol_a_LDFLAGS = -L../DasherCore -L../DasherCore/LanguageModelling -L../DasherCore/Alphabet -ldashercore -ldasherlm -ldasheralphabet

pkgdata_DATA = dasher.glade dashermaemo.glade
