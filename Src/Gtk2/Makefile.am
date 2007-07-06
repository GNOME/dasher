LIBS = @INTLLIBS@ @LIBS@

noinst_LIBRARIES = libdashergtk.a libdashercontrol.a
noinst_PROGRAMS = generate-schema

#bin_PROGRAMS = dasher-config

if WITH_GPE
gladefile = dashergpe.glade
settings_store = gpesettings_store.cc gpesettings_store.h

else

if WITH_MAEMO

if WITH_MAEMOFULLSCREEN
gladefile = dashermaemofullscreen.glade
else
gladefile = dashermaemo.glade
endif

else
gladefile = dasher.glade
endif

settings_store = 
endif

generate_schema_SOURCES = GenerateSchema.cpp

#dasher_config_SOURCES = \
#		DasherAppSettings.cpp \
#		DasherAppSettings.h \
#		Preferences.cpp \
#		Preferences.h \
#		dasher_config.cpp \
#		module_settings_window.cpp \
#		module_settings_window.h#
#
#dasher_config_LDADD = \
#	$(GTK2BUILD_LIBS)

libdashercontrol_a_SOURCES = \
		Canvas.cpp \
		Canvas.h \
		DasherControl.cpp \
		DasherControl.h \
		GtkDasherControl.cpp \
		GtkDasherControl.h \
		KeyboardHelper.cpp \
		KeyboardHelper.h \
		PangoCache.cpp \
		PangoCache.h \
		Timer.cpp \
		Timer.h \
		custom_marshal.cpp \
		custom_marshal.h \
		joystick_input.h \
		mouse_input.h \
		$(settings_store)

libdashergtk_a_SOURCES = \
                         DasherAppSettings.cpp \
                         DasherAppSettings.h \
                         GtkDasherControl.h \
                         Preferences.cpp \
                         Preferences.h \
                         dasher.h \
                         dasher_action.cpp \
                         dasher_action.h \
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
			 dasher_editor_external.cpp \
			 dasher_editor_external.h \
                         dasher_editor_internal.cpp \
                         dasher_editor_internal.h \
                         dasher_external_buffer.cpp \
                         dasher_external_buffer.h \
                         dasher_internal_buffer.cpp \
                         dasher_internal_buffer.h \
			 dasher_lock_dialogue.cpp \
			 dasher_lock_dialogue.h \
			 dasher_maemo_helper.cpp \
			 dasher_maemo_helper.h \
                         dasher_main.cpp \
                         dasher_main.h \
			 game_mode_helper.cpp \
			 game_mode_helper.h \
			 module_settings_window.cpp \
			 module_settings_window.h 

libdashergtk_a_LIBADD = @SPEECH_SOURCES@
libdashergtk_a_DEPENDENCIES = @SPEECH_SOURCES@

AM_CXXFLAGS = $(GTK2BUILD_CFLAGS) -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl 

AM_CFLAGS = $(GTK2_CFLAGS)

