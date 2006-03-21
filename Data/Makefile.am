SUBDIRS = Help Translations system.rc

#Applicationsdir = $(datadir)/applications
#Applications_DATA = dasher.desktop

desktopdir = $(datadir)/applications
desktop_in_files = dasher.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

appicondir = $(datadir)/icons/hicolor/48x48/apps
appicon_DATA = dasher.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = dasher.svg

install-data-hook:
	gtk-update-icon-cache -f -t $(datadir)/icons/hicolor || :

install-schemas:
	      GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	      gconftool --makefile-install-rule dasher.schemas

EXTRA_DIST = dasher.desktop dasher.png dasher.svg dasher.schemas
