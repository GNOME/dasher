LIBS = @INTLLIBS@ @LIBS@

noinst_LIBRARIES = libdashergtk.a

if WITH_GPE
gladefile = dashergpe.glade
else
gladefile = dasher.glade
endif

libdashergtk_a_SOURCES = dasher.cc dasher.h settings_store.cc settings_store.h canvas.cc canvas.h edit.cc edit.h accessibility.cc accessibility.h speech.cc speech.h $(gladefile)

libdashergtk_a_LIBADD = @SPEECH_SOURCES@
libdashergtk_a_DEPENDENCIES = @SPEECH_SOURCES@

AM_CXXFLAGS = -O2 $(GTK2_CFLAGS) $(GCONF_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) -I../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl

pkgdata_DATA = $(gladefile) 
