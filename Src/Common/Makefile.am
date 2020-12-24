noinst_LTLIBRARIES = libdashermisc.la
libdashermisc_la_SOURCES = \
                AppSettingsData.h \
                AppSettingsHeader.h \
		Common.h \
		Globber.h \
		Globber.cpp \
		I18n.h \
		MSVC_Unannoy.h \
		myassert.h \
		mydebug.h \
		ModuleSettings.h \
		NoClones.h \
                Trace.cpp \
		Trace.h \
		Allocators/PooledAlloc.h \
		Allocators/SimplePooledAlloc.h \
		Platform/stdminmax.h \
		Types/int.h

libdashermisc_la_LIBADD = $(LTLIBOBJS)

AM_CXXFLAGS = -I$(srcdir)/../DasherCore

EXTRA_DIST = Common.vcxproj
