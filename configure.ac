AC_PREREQ(2.59)
AC_INIT([dasher],
  m4_esyscmd([build-aux/mkversion]),
  [http://bugzilla.gnome.org/enter_bug.cgi?product=dasher],
  [dasher])

# This becomes part of AC_INIT in autoconf 2.64
PACKAGE_URL="http://www.inference.phy.cam.ac.uk/dasher/"
AC_DEFINE_UNQUOTED([PACKAGE_URL], ["$PACKAGE_URL"],
                   [Define to the home page for this package.])
AC_SUBST([PACKAGE_URL])

AC_CONFIG_SRCDIR([Src/main.cc])
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.8 gnu check-news dist-bzip2])
AM_MAINTAINER_MODE

AC_PROG_MAKE_SET

# IT_PROG_INTLTOOL requires AM_NLS but ignores --enable-nls
AM_NLS
AM_CONDITIONAL(USE_INTLTOOL, test "x$USE_NLS" = "xyes")
AS_IF([test "x$USE_NLS" = "xyes"],[
IT_PROG_INTLTOOL([0.40.1])
],
[touch po/Makefile.in.in]) dnl as intltoolize insists

LT_INIT([disable-static])

dnl AM_ICONV

GETTEXT_PACKAGE=dasher
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])
AM_GLIB_GNU_GETTEXT

AC_PROG_CXX

AC_PROG_LD_GNU

WITHGTK2=true;

PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.6)

AC_LANG_PUSH(C++)
AC_CHECK_FUNCS(lldiv)
AC_CHECK_FUNC(socket,,[AC_CHECK_LIB(socket,socket)])
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
	  # Need to work out how to translate replaced variables before uncommenting
	  # AC_CONFIG_FILES([Data/Help/Gnome/C/dasher.xml])
	fi, 
	WITHGNOME=true)

dnl Just useful for debugging
AC_ARG_WITH([gvfs],
	AS_HELP_STRING([--with-gvfs],
		[build with gvfs from gio in glib (default is YES)]),
	[],
	with_gvfs="yes")
if test "x$with_gvfs" != "xno"; then
	PKG_CHECK_MODULES(GIO, gio-2.0,
		[have_gio=true
		AC_DEFINE(HAVE_GIO, 1, [gio exists so use gvfs])],
		[have_gio=false])
fi

AC_ARG_ENABLE([speech],
	 AS_HELP_STRING([--enable-speech],[build with speech support (default is NO)]),
	 if test "x$enableval" = "xno"; then
	   WITHSPEECH=false; 
	 else
	   WITHSPEECH=true;
         fi, 
	 WITHSPEECH=false)

AC_ARG_WITH([qte],
	 AS_HELP_STRING([--with-qte],[build with QTE support (default is NO -- currently untested)]), 
	 if test "x$withval" = "xno"; then
	   WITHQTE=false; 
	 else
	   WITHQTE=true;
         fi, 
	 WITHQTE=false)

AC_ARG_WITH([gpe],
	 AS_HELP_STRING([--with-gpe],[build with GPE support (default is NO -- currently untested)]), 
	 if test "x$withval" = "xno"; then
	   WITHGPE=false; 
	 else
	   WITHGPE=true;
         fi, 
	 WITHGPE=false)

AC_ARG_ENABLE([a11y],
	 AS_HELP_STRING([--enable-a11y],[build with support for the GNOME accessibility framework (default is YES)]),
	 if test "x$enableval" = "xno"; then
	   WITHA11Y=false; 
	 else
	   WITHA11Y=true;
         fi, 
	 WITHA11Y=true)

AC_ARG_ENABLE([japanese],
	 AS_HELP_STRING([--enable-japanese],[build with support for Japanese Kanji entry (experimental -- default is NO)]),
	 if test "x$enableval" = "xno"; then
	   WITHJAPANESE=false; 
	 else
	   AC_CHECK_LIB(canna, RkBgnBun,, AC_MSG_ERROR([Canna library not found (required for Japanese).]))
	   WITHJAPANESE=true;
         fi, 
	 WITHJAPANESE=false)

AC_ARG_ENABLE([chinese],
	 AS_HELP_STRING([--enable-chinese],[build with support for Chinese PinYin entry (experimental -- default is NO)]),
	 if test "x$enableval" = "xno"; then
	   WITHCHINESE=false; 
	 else
	   WITHCHINESE=true;
         fi, 
	 WITHCHINESE=false)

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


AC_ARG_WITH([maemo],
	AS_HELP_STRING([--with-maemo],[build with Maemo support (default is NO)]),
	if test "x$withval" = "xyes"; then
	   WITHMAEMO=true;
	   WITHGNOME=false;
	   WITHA11Y=false;
	else
	   WITHMAEMO=false;
        fi,
	WITHMAEMO=false)

AC_ARG_WITH([maemo-fullscreen],
	AS_HELP_STRING([--with-maemo-fullscreen],[build as standalone Maemo fullscreen app (default is NO)]),
	if test "x$withval" = "xyes"; then
	   WITHMAEMOFULLSCREEN=true;
	   WITHMAEMO=true;
	   WITHGNOME=false;
	   WITHA11Y=false;
	else
	   WITHMAEMOFULLSCREEN=false;
        fi,
	WITHMAEMOFULLSCREEN=false)


dnl AC_ARG_WITH([darwin],
dnl 	AS_HELP_STRING([--with-darwin],[build with Darwin (Mac OS X) support (default is NO)]),
dnl 	if test "x$withval" = "xyes"; then
dnl 	   WITHDARWIN=true;
dnl 	else
dnl 	   WITHDARWIN=false;
dnl 	fi,
dnl 	WITHDARWIN=false)

AC_ARG_WITH([cairo],
	 AS_HELP_STRING([--with-cairo],[build with cairo support (default is YES)]),
	 if test "x$withval" = "xno"; then
	   WITHCAIRO=false; 
	 else
	   WITHCAIRO=true;
	 fi, 
	 WITHCAIRO=true)

if test x"$WITHGTK2" = xtrue
then 
	PKG_CHECK_MODULES(GTK2, gtk+-2.0 >= 2.6.0,[GTK2HERE=true],[GTK2HERE=false]) 
fi

if test x"$GTK2HERE" = xtrue -a x"$WITHGTK2" = xtrue -a x"$WITHQTE" != xtrue
then
	BUILDGTK2=true
elif test x"$GTK2HERE" = xtrue -a x"$WITHQTE" != xtrue
then
	BUILDGTK2=true
else 
	BUILDGTK2=false
fi

DAM_GCONF_SOURCE_2
if test x$BUILDGTK2 = xtrue
then
# GtkBuilder's autoconnect feature requires dasher to be linked with
# the --export-dynamic flag. pkg-config does not provide it, and libtool
# knows what to do with -export-dynamic.
# (We need this e.g., for <object class="DasherEditor">)
	GTK2_LIBS="$GTK2_LIBS -export-dynamic"

	PKG_CHECK_MODULES(GCONF, gconf-2.0,
		[AC_DEFINE([WITH_GCONF], 1, [Use GConf])],
		[AC_MSG_WARN([GConf was not detected - user preferences will not be stored.])])
	save_CFLAGS="$CFLAGS"
	save_LIBS="$LIBS"
	CFLAGS="$CFLAGS $GTK2_CFLAGS"
	LIBS="$LIBS $GTK2_LIBS"
	AC_CHECK_FUNCS([gtk_builder_new gtk_show_uri], [],
		[if test "x$ac_cv_func_gtk_builder_new" = "xno"; then
			AC_MSG_ERROR([A version of GTK+ providing GtkBuilder is necessary.])
		fi
		if test "x$ac_cv_func_gtk_show_uri" = "xno"; then
			AC_MSG_WARN([This version of GTK+ doesn't provide gtk_show_uri - help will not be displayed from within dasher.])
		fi]
	)
	CFLAGS="$save_CFLAGS"
	LIBS="$save_LIBS"
else
	AC_MSG_ERROR("Unable to find GTK2 or another necessary library.")
fi

if test x"$DEBUG" = xtrue; then
	AC_DEFINE([DEBUG], 1, [Additional debug checks enabled])
fi

if test x"$WITHSPEECH" = xtrue; then
	PKG_CHECK_MODULES(gnome_speech, gnome-speech-1.0 bonobo-activation-2.0 libbonobo-2.0 ORBit-2.0)
	AC_DEFINE([GNOME_SPEECH], 1, [gnome_speech-1 libraries are present])
#	SPEECH_SOURCES='speech.$(OBJEXT)'
fi

if test x"$WITHA11Y" = xtrue; then
	PKG_CHECK_MODULES(gnome_a11y, bonobo-activation-2.0 libbonobo-2.0 ORBit-2.0 libgnomeui-2.0 cspi-1.0 atk)
	AC_DEFINE([GNOME_A11Y], 1, [gnome_a11y libraries are present])
fi

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
if test x"$WITHQTE" != xtrue -a x"$WITHA11Y" != xtrue; then
	if test	x$no_x = xyes ; then
		AC_MSG_ERROR([X development libraries not found])
	else
		X_LIBS="$X_LIBS $X_PRE_LIBS -lX11 $X_EXTRA_LIBS"
	fi

	save_LIBS="$LIBS"
	AC_CHECK_LIB([Xtst], [XTestFakeKeyEvent],
	             [X_LIBS="$X_LIBS -lXtst"
	              AC_DEFINE(HAVE_XTST, 1, [The XTest extension is present])],
	             [AC_MSG_ERROR([Couldn't find the Xtst library. Check config.log for details])],
	             $X_LIBS)
	LIBS="$save_LIBS"
	AC_SUBST(X_LIBS)
fi

if test x"$WITHJAPANESE" = xtrue; then
	AC_DEFINE([JAPANESE], 1, [Japanese support enabled])
	JAPANESE_SOURCES='CannaConversionHelper.$(OBJEXT)'
else
	JAPANESE_SOURCES=
fi

AC_SUBST(JAPANESE_SOURCES)

if test x"$WITHCHINESE" = xtrue; then
	AC_DEFINE([CHINESE], 1, [Chinese support enabled])
	CHINESE_SOURCES='PinYinConversionHelper.$(OBJEXT) PinyinParser.$(OBJEXT)'

else
	CHINESE_SOURCES=
fi

AC_SUBST(CHINESE_SOURCES)

if test x"$WITHJOYSTICK" = xtrue; then
	AC_DEFINE([JOYSTICK], 1, [Linux joystick support enabled])
fi

if test x"$WITHTILT" = xtrue; then
	AC_DEFINE([TILT], 1, [Tilt input support enabled])
fi

if test x"$WITHGPE" = xtrue; then
	AC_DEFINE([WITH_GPE], 1, [gpe is present])
fi

dnl if test x"$WITHDARWIN" = xtrue ; then
dnl         AC_DEFINE([WITH_DARWIN], 1, [Targeting Darwin/X11])
dnl fi

if test x"$WITHMAEMO" = xtrue; then
      AC_DEFINE([WITH_MAEMO], 1, [Maemo is present])
      PKG_CHECK_MODULES(hildon, hildon-libs)
      AC_CHECK_LIB(osso, osso_initialize,, AC_MSG_ERROR([Osso library not found.]))
fi			

if test x"$WITHMAEMOFULLSCREEN" = xtrue; then
      AC_DEFINE([WITH_MAEMOFULLSCREEN], 1, [Building as fullscreen Maemo app])
fi

if test x"$WITHCAIRO" = xtrue; then
	PKG_CHECK_MODULES(cairo, gtk+-2.0 >= 2.8.0)
	AC_DEFINE([WITH_CAIRO], 1, [use cairo (needs Gtk+ >= 2.8.0)])
fi

if test x"$BUILDGTK2" = xtrue; then 
	PKG_CHECK_MODULES(gthread, gthread-2.0)
fi

AC_SUBST(gnome_speech_LIBS)
AC_SUBST(gnome_speech_CFLAGS)
AC_SUBST([SPEECH_SOURCES])

AC_SUBST(gnome_a11y_LIBS)
AC_SUBST(gnome_a11y_CFLAGS)
AC_SUBST([A11Y_SOURCES])

AC_SUBST(gnome_LIBS)
AC_SUBST(gnome_CFLAGS)

AC_SUBST(gthread_LIBS)
AC_SUBST(gthread_CFLAGS)

AC_SUBST(hildon_LIBS)
AC_SUBST(hildon_CFLAGS)

AC_SUBST(GETTEXT_PACKAGE)

AM_CONDITIONAL(DOGTK2, test x$BUILDGTK2 = xtrue)	
AM_CONDITIONAL(DOQTE, test x$WITHQTE = xtrue)
AM_CONDITIONAL(GNOME_SPEECH, test x$WITHSPEECH = xtrue)
AM_CONDITIONAL(GNOME_A11Y, test x$WITHA11Y = xtrue)
AM_CONDITIONAL(GNOME_HELP, test x$WITHGNOME = xtrue)
AM_CONDITIONAL(WITH_GPE, test x$WITHGPE = xtrue)
AM_CONDITIONAL(WITH_MAEMO, test x$WITHMAEMO = xtrue)
AM_CONDITIONAL(WITH_MAEMOFULLSCREEN, test x$WITHMAEMOFULLSCREEN = xtrue)
AM_CONDITIONAL(JAPANESE, test x$WITHJAPANESE = xtrue)
AM_CONDITIONAL(CHINESE, test x$WITHCHINESE = xtrue)
AM_CONDITIONAL(JOYSTICK, test x$WITHJOYSTICK = xtrue)

AC_SUBST(GTK2_CFLAGS)
AC_SUBST(GTK2_LIBS)

AC_SUBST(GCONF_CFLAGS)
AC_SUBST(GCONF_LIBS)

if test x"$WITHGPE" = xtrue ; then
	SETTINGS_CFLAGS=" "
	SETTINGS_LIBS="-lgpewidget -lXsettings-client -lXsettings"
else
	SETTINGS_CFLAGS=$GCONF_CFLAGS
	SETTINGS_LIBS=$GCONF_LIBS
fi
AC_SUBST(SETTINGS_CFLAGS)
AC_SUBST(SETTINGS_LIBS)

GTK2BUILD_CFLAGS="$GTK2_CFLAGS $GIO_CFLAGS $SETTINGS_CFLAGS $gthread_CFLAGS $gnome_speech_CFLAGS $gnome_a11y_CFLAGS $gnome_CFLAGS $hildon_CFLAGS"

GTK2BUILD_LIBS="$X_LIBS $GTK2_LIBS $GIO_LIBS $SETTINGS_LIBS $gthread_LIBS $gnome_speech_LIBS $gnome_a11y_LIBS $gnome_LIBS $hildon_LIBS"

AC_SUBST(GTK2BUILD_CFLAGS)
AC_SUBST(GTK2BUILD_LIBS)

AC_MSG_CHECKING([for gnome-doc-utils])
dnl Avoid default action which is to complain if not found
GNOME_DOC_INIT([0.9.0],[gdu_cv_have_gdu=yes],[gdu_cv_have_gdu=no])
AC_MSG_RESULT($gdu_cv_have_gdu)

if test x"$WITHGNOME" = xtrue -a x"$gdu_cv_have_gdu" = xno ; then
    AC_MSG_ERROR([gnome-doc-utils not found!])
fi

dnl XXX PRLW How is this better than the standard
dnl gdu_cv_have_gdu && enable_scrollkeeper ?
AM_CONDITIONAL(ENABLE_SK, test x"$WITHGNOME" = xfalse -a x"$enable_scrollkeeper" = "xyes")

AC_CONFIG_FILES([Data/dasher.desktop.in
		 Data/Makefile
		 Data/training/Makefile
		 Data/alphabets/Makefile
		 Data/colours/Makefile
		 Data/controllabels/Makefile
		 Data/GUI/Makefile
		 Data/Help/Makefile
		 Data/Help/Gnome/Makefile
		 Doc/Makefile
		 Doc/user/Makefile
		 Makefile
		 Src/Makefile
		 Src/Common/Makefile
		 Src/DasherCore/Makefile
		 Src/DasherCore/LanguageModelling/Makefile
		 Src/DasherIM/Makefile
		 Src/Gtk2/Makefile
		 Src/Installer/Makefile
		 Src/InstallerTobii/Makefile
		 Src/InstallerW2K/Makefile
		 Src/Win32/Makefile
		 po/Makefile.in
])

AC_OUTPUT
