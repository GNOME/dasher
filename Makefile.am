SUBDIRS = Src Data po Doc

distuninstallcheck_listfiles = \
		find . -type f -print | \
		grep -v '^\./var/scrollkeeper' | \
		grep -v '^\./share/gnome/help'

EXTRA_DIST = \
		INSTALL.Linux \
		INSTALL.WinCE \
		INSTALL.Windows \
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

maemo-install: all
	- mkdir tmp_build
	cp -a Data/maemo/* tmp_build
	cp Src/dasher tmp_build/usr/bin/dasher
	strip tmp_build/usr/bin/dasher
	cp Src/Gtk2/dashermaemo.glade tmp_build/usr/share/dasher
	cp Data/system.rc/training_english_GB.txt Data/system.rc/alphabet.english.xml Data/system.rc/colour.xml tmp_build/usr/share/dasher
	ln -sf /var/lib/install/usr/share/applications/hildon/dasher.desktop tmp_build/etc/others-menu/extra_applications/0301_dasher.desktop
	- find tmp_build -name CVS -exec rm -rf \{\} \;
	dpkg-deb -b tmp_build dasher.deb 
	rm -rf tmp_build
