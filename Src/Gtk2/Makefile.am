LIBS = @INTLLIBS@ @LIBS@

noinst_LIBRARIES = libdashergtk.a

if WITH_GPE
gladefile = dashergpe.glade
settings_store = gpesettings_store.cc gpesettings_store.h
EXTRA_DIST = settings_store.cc settings_store.h
else
gladefile = dasher.glade
settings_store = settings_store.cc settings_store.h
EXTRA_DIST = gpesettings_store.cc gpesettings_store.h
endif

libdashergtk_a_SOURCES = \
		accessibility.cc \
		accessibility.h \
		canvas.cc \
		canvas.h \
		dasher.cc \
		dasher.h \
		dasher.glade \
		dashergpe.glade \
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

AM_CXXFLAGS = $(GTK2BUILD_CFLAGS) -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I$(top_srcdir)/intl 

pkgdata_DATA = $(gladefile) 
