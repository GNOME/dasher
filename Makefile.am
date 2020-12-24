SUBDIRS = Src Data Doc po

ACLOCAL_AMFLAGS = -I m4

distuninstallcheck_listfiles = \
		find . -type f -print | \
		grep -v '^\./var/scrollkeeper' | \
		grep -v '^\./share/gnome/help'

EXTRA_DIST = \
		INSTALL.Linux \
		INSTALL.WinCE \
		INSTALL.Windows \
		MAINTAINERS \
		m4/pkg.m4 \
		m4/dgconf-2.m4 \
		m4/nls.m4 \
		.tarball-version \
		build-aux/mkversion

DISTCLEANFILES = \
		Data/dasher.desktop \
		Data/Help/Gnome/Makefile

MAINTAINERCLEANFILES = \
		$(srcdir)/INSTALL \
		$(srcdir)/aclocal.m4 \
		$(srcdir)/autoscan.log \
		$(srcdir)/compile \
		$(srcdir)/config.guess \
		$(srcdir)/config.h.in \
		$(srcdir)/config.h.in~ \
		$(srcdir)/config.sub \
		$(srcdir)/configure \
		$(srcdir)/configure.scan \
		$(srcdir)/depcomp \
		$(srcdir)/install-sh \
		$(srcdir)/ltmain.sh \
		$(srcdir)/missing \
		$(srcdir)/mkinstalldirs \
		$(srcdir)/omf.make \
		$(srcdir)/xmldocs.make \
		$(srcdir)/po/Makefile.in.in
