SUBDIRS = Src Data po Doc

distuninstallcheck_listfiles = \
		find . -type f -print | \
		grep -v '^\./var/scrollkeeper' | \
		grep -v '^\./share/gnome/help'

EXTRA_DIST = \
		NEWS \
		MAINTAINERS \
		intltool-merge.in \
		intltool-extract.in \
		intltool-update.in

DISTCLEANFILES = \
		intltool-merge \
		intltool-extract \
		intltool-update

MAINTAINERCLEANFILES = \
		$(srcdir)/INSTALL \
		$(srcdir)/aclocal.m4 \
		$(srcdir)/autoscan.log \
		$(srcdir)/compile \
		$(srcdir)/config.guess \
		$(srcdir)/config.h.in \
		$(srcdir)/config.sub \
		$(srcdir)/configure \
		$(srcdir)/configure.scan \
		$(srcdir)/depcomp \
		$(srcdir)/install-sh \
		$(srcdir)/ltmain.sh \
		$(srcdir)/missing \
		$(srcdir)/mkinstalldirs \
		`find "$(srcdir)" -regex '.*/Src/Common/Expat' -prune -o -type f -name Makefile.in -print`
