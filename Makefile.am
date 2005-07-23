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
		$(srcdir)/intltool-extract.in \
		$(srcdir)/intltool-merge.in \
		$(srcdir)/intltool-update.in \
		$(srcdir)/omf.make \
		$(srcdir)/xmldocs.make \
		$(srcdir)/po/Makefile.in.in \
		$(srcdir)/Src/Test/COPYING \
		$(srcdir)/Src/Test/INSTALL \
		$(srcdir)/Src/Test/aclocal.m4 \
		$(srcdir)/Src/Test/config.guess \
		$(srcdir)/Src/Test/config.h.in \
		$(srcdir)/Src/Test/config.sub \
		$(srcdir)/Src/Test/configure \
		$(srcdir)/Src/Test/depcomp \
		$(srcdir)/Src/Test/install-sh \
		$(srcdir)/Src/Test/missing \
		$(srcdir)/Src/Test/mkinstalldirs \
		$(srcdir)/Src/Test/Makefile.in \
		$(srcdir)/Src/Test/omf.make \
		$(srcdir)/Src/Test/xmldocs.make \
		`find "$(srcdir)" -regex '.*/Src/Common/Expat' -prune -o -type f -name Makefile.in -print`
