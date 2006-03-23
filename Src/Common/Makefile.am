
noinst_LIBRARIES = libdashermisc.a
libdashermisc_a_SOURCES = \
                AppSettingsData.h \
                AppSettingsHeader.h \
		Common.h \
		Hash.h \
		MSVC_Unannoy.h \
		myassert.h \
		NoClones.h \
		Platform.h \
		Random.h \
                Trace.cpp \
		Trace.h \
		Allocators/PooledAlloc.h \
		Allocators/SimplePooledAlloc.h \
		Platform/stdminmax.h \
		Types/int.h \
		Types/int32.h \
		Types/int64.h

#		IOstreamDasherEdit.cc   I have no idea what this does, but it's broken now
#		IOstreamDasherEdit.h 

AM_CXXFLAGS = $(GTK2_CFLAGS) $(SETTINGS_CFLAGS) $(gnome_speech_CFLAGS) $(gnome_a11y_CFLAGS) $(glade_CFLAGS) $(gnome_CFLAGS) $(wnck_CFLAGS) -DWNCK_I_KNOW_THIS_IS_UNSTABLE -I$(srcdir)/../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl 

# Note that I've included the expat subdirectory recursively here, in
# order to make sure that everything needed for the Windows build gets
# included. We'll get CVS dirs and the like with this, so it's bad to
# start off with, but we also probably want to think about whether
# including our own expat source tree here is a good idea or not.


EXTRA_DIST = Common.vcproj Common_vc70.vcproj Common_vc71.vcproj Expat
