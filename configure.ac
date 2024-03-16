AC_PREREQ(2.59)
AC_INIT([dasher], [m4_esyscmd_s([build-aux/mkversion])],
  [https://gitlab.gnome.org/GNOME/dasher/-/issues/],
  [dasher],
  [http://www.inference.phy.cam.ac.uk/dasher/])

# This becomes part of AC_INIT in autoconf 2.64
m4_ifndef([AC_PACKAGE_URL],
	[m4_define([AC_PACKAGE_URL],["http://www.inference.phy.cam.ac.uk/dasher/"])
	AC_DEFINE_UNQUOTED([PACKAGE_URL],
                       [AC_PACKAGE_URL],
                       [Define to the home page for this package.])
	AC_SUBST([PACKAGE_URL],[AC_PACKAGE_URL])])

AC_CONFIG_SRCDIR([Src/main.cc])
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_LIBOBJ_DIR([Src/Common])

AM_INIT_AUTOMAKE([1.8 foreign check-news dist-bzip2 subdir-objects])
AM_MAINTAINER_MODE

LT_INIT([disable-static])

AM_GNU_GETTEXT_VERSION([0.19])
AM_GNU_GETTEXT([external])

CXXFLAGS="$CXXFLAGS -std=c++0x"
AC_PROG_CXX

AC_PROG_LD_GNU

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.6)

AC_LANG_PUSH(C++)
AC_CHECK_FUNCS(lldiv)
AC_CHECK_FUNC(socket,,[AC_CHECK_LIB(socket,socket)])
AC_REPLACE_FUNCS([round])
AC_LANG_POP(C++)

AC_ARG_ENABLE(debug,
	AS_HELP_STRING([--enable-debug],
		[build with additional debugging checks (default is NO)]),
	if test "x$enable_debug" = "xno"; then
	  DEBUG=false
	else
	  DEBUG=true
	fi,
	DEBUG=false)

AC_ARG_WITH([gnome],
	AS_HELP_STRING([--with-gnome],[build GNOME documentation (default is YES)]),
	if test "x$with_gnome" = "xno"; then
	  WITHGNOME=false 
	else
	  WITHGNOME=true
	fi, 
	WITHGNOME=true)

AC_ARG_ENABLE([speech],
	[AS_HELP_STRING([--enable-speech=@<:@speechdispatcher@:>@],
		[build with speech support (default is YES)])])

AC_ARG_ENABLE([atspi],
	[AS_HELP_STRING([--enable-atspi],
		[with Gtk3 use atspi rather than Xtst for direct entry mode if found (default is YES)])])

AC_ARG_ENABLE([japanese],
	 AS_HELP_STRING([--enable-japanese],[build with support for Japanese Kanji entry (experimental -- default is NO)]),
	 if test "x$enableval" = "xno"; then
	   WITHJAPANESE=false; 
	 else
	   AC_CHECK_LIB(canna, RkBgnBun,, AC_MSG_ERROR([Canna library not found (required for Japanese).]))
	   WITHJAPANESE=true;
         fi, 
	 WITHJAPANESE=false)

AC_ARG_ENABLE([joystick],
	 AS_HELP_STRING([--enable-joystick],[Build with Linux joystick support (note that other UNIX based operating systems aren't supported here)]),
	 if test "x$enableval" = "xno"; then
	   WITHJOYSTICK=false; 
	 else
	   WITHJOYSTICK=true;
         fi, 
	 WITHJOYSTICK=false)

AC_ARG_ENABLE([tilt],
	 AS_HELP_STRING([--enable-tilt],[Build with experimental support for tilt sensor input (experimental -- default is NO)]),
	 if test "x$enableval" = "xno"; then
	   WITHTILT=false; 
	 else
	   WITHTILT=true;
         fi, 
	 WITHTILT=false)

dnl AC_ARG_WITH([darwin],
dnl 	AS_HELP_STRING([--with-darwin],[build with Darwin (Mac OS X) support (default is NO)]),
dnl 	if test "x$withval" = "xyes"; then
dnl 	   WITHDARWIN=true;
dnl 	else
dnl 	   WITHDARWIN=false;
dnl 	fi,
dnl 	WITHDARWIN=false)


PKG_CHECK_MODULES(GTK, gtk+-3.0)

PKG_CHECK_MODULES(GIO, gio-2.0,
	[AC_DEFINE(HAVE_GIO, 1, [gio exists so use gvfs])])

save_CFLAGS="$CFLAGS"
save_LIBS="$LIBS"
CFLAGS="$CFLAGS $GTK_CFLAGS"
CPPFLAGS="$CPPFLAGS $GTK_CFLAGS"
LIBS="$LIBS $GTK_LIBS"
AC_CHECK_HEADERS([gdk/gdkkeysyms-compat.h])
AC_CHECK_FUNCS([gtk_show_uri_on_window])
CFLAGS="$save_CFLAGS"
LIBS="$save_LIBS"

# GtkBuilder's autoconnect feature requires dasher to be
# linked with the --export-dynamic flag. pkg-config does not
# provide it, and libtool knows what to do with -export-dynamic.
# (We need this e.g., for <object class="DasherEditor">)
GTK_LIBS="$GTK_LIBS -export-dynamic"

if test x"$DEBUG" = xtrue; then
	AC_DEFINE([DEBUG], 1, [Additional debug checks enabled])
fi

dnl default: enable speech
dnl if speechdispatcher or default
dnl   check for speechdispatcher
dnl if nothing found
dnl   OK if wanted no speech
dnl   warn if default
dnl   error if yes
dnl   error if selected particular
dnl BTW test: The -a operator has higher precedence than the -o operator.
speech_module=none
AS_IF(
	[test "x$enable_speech" = x],
	[enable_speech=yes; only_warn_if_speech_not_found=yes])
AS_IF(
	[test "x$enable_speech" = xspeechdispatcher -o "x$enable_speech" = xyes],
	[AC_CHECK_HEADER([speech-dispatcher/libspeechd.h],
		[AC_CHECK_LIB([speechd], [spd_open],
			[AC_DEFINE([HAVE_SPEECHD], 1,
				[speechdispatcher libraries are present])
			 speech_module="speechdispatcher"
	 		 SPEECH_CPPFLAGS=
	 		 SPEECH_LDFLAGS=
	 		 SPEECH_LIBS=-lspeechd])])])
AS_IF(
	[test $speech_module = none],
	[AS_CASE(["x$enable_speech"],
		[xno],
			[:],
		[xyes],
			[AS_IF([test x$only_warn_if_speech_not_found = xyes],
				[AC_MSG_WARN([no speech module found])],
				[AC_MSG_ERROR([speech requested but no speech module found])])],
		[AC_MSG_ERROR([speech module "$enable_speech" requested but not found])])])
AC_MSG_CHECKING([which speech module to use])
AC_MSG_RESULT([$speech_module])
AS_IF([test $speech_module != none],
	[AC_DEFINE([WITH_SPEECH], 1, [text-to-speech is present])])

AC_SUBST(SPEECH_CPPFLAGS)
AC_SUBST(SPEECH_LDFLAGS)
AC_SUBST(SPEECH_LIBS)

AC_PATH_XTRA
AC_CHECK_LIB(expat, XML_Parse,,[
	if test	x$no_x = xyes ; then
		AC_MSG_ERROR([Expat library not found.])
	else
		CPPFLAGS="$CPPFLAGS $X_CFLAGS"
		LDFLAGS="$LDFLAGS $X_LIBS"
		dnl Check for different function to avoid cached "no" result.
		AC_CHECK_LIB(expat, XML_SetElementHandler,,
		             [AC_MSG_ERROR([Expat library not found.])])
	fi
])

PKG_CHECK_MODULES([ATSPI],
	[atspi-2 >= 2.11],
	[have_libatspi=yes],
	[have_libatspi=no])

AS_IF(	[test x$no_x = xyes],
	[AC_MSG_WARN([X development libraries not found])],
	[X_LIBS="$X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS"])

AC_CHECK_LIB([Xtst], [XTestFakeKeyEvent],
	[have_libxtst=yes],
	[have_libxtst=no],
	[$X_LIBS])

AS_IF(	[test $have_libatspi = yes -a x$enable_atspi != xno],
		[connect_using=libatspi],
	[test $have_libatspi = no -a x$enable_atspi = xyes],
		[AC_MSG_ERROR([atspi 2 requested but not found])],
	[test $have_libxtst = yes],
		[connect_using=libxtst
		 X_LIBS="$X_LIBS -lXtst"],
	[AC_MSG_ERROR([No method to send characters into another application found])])

AM_CONDITIONAL(USE_ATSPI, test $connect_using = libatspi)

if test x"$WITHJAPANESE" = xtrue; then
	AC_DEFINE([JAPANESE], 1, [Japanese support enabled])
	JAPANESE_SOURCES='CannaConversionHelper.$(OBJEXT)'
else
	JAPANESE_SOURCES=
fi

AC_SUBST(JAPANESE_SOURCES)

if test x"$WITHJOYSTICK" = xtrue; then
	AC_DEFINE([JOYSTICK], 1, [Linux joystick support enabled])
fi

if test x"$WITHTILT" = xtrue; then
	AC_DEFINE([TILT], 1, [Tilt input support enabled])
fi

dnl if test x"$WITHDARWIN" = xtrue ; then
dnl         AC_DEFINE([WITH_DARWIN], 1, [Targeting Darwin/X11])
dnl fi

PKG_CHECK_MODULES(cairo, cairo)

PKG_CHECK_MODULES(gthread, gthread-2.0)
AC_SUBST(gthread_LIBS)
AC_SUBST(gthread_CFLAGS)

AM_CONDITIONAL(USE_SPEECHDISPATCHER, test $speech_module = speechdispatcher)
AM_CONDITIONAL(GNOME_HELP, test x$WITHGNOME = xtrue)
AM_CONDITIONAL(JAPANESE, test x$WITHJAPANESE = xtrue)
AM_CONDITIONAL(JOYSTICK, test x$WITHJOYSTICK = xtrue)

AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

GTKBUILD_CFLAGS="$GTK_CFLAGS $GIO_CFLAGS $SETTINGS_CFLAGS $gthread_CFLAGS"

GTKBUILD_LIBS="$X_LIBS $GTK_LIBS $GIO_LIBS $SETTINGS_LIBS $gthread_LIBS"

AC_SUBST(GTKBUILD_CFLAGS)
AC_SUBST(GTKBUILD_LIBS)

YELP_HELP_INIT

AC_CONFIG_FILES([Data/dasher.desktop.in
		 Data/Makefile
		 Data/training/Makefile
		 Data/alphabets/Makefile
		 Data/colours/Makefile
		 Data/control/Makefile
		 Data/GUI/Makefile
		 Data/Help/Makefile
		 Data/Help/Gnome/Makefile
		 Data/settings/Makefile
		 Doc/Makefile
		 Doc/user/Makefile
		 Makefile
		 Src/Makefile
		 Src/Common/Makefile
		 Src/DasherCore/Makefile
		 Src/DasherCore/LanguageModelling/Makefile
		 Src/Gtk2/Makefile
		 po/Makefile.in
])

AC_OUTPUT
