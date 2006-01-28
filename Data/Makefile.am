SUBDIRS = Help Translations system.rc

Applicationsdir = $(datadir)/applications
Applications_DATA = dasher.desktop
Iconsdir = $(datadir)/icons/hicolor/48x48/apps
Icons_DATA = dasher.png

install-schemas:
	      GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	      gconftool --makefile-install-rule dasher.schemas

EXTRA_DIST = dasher.desktop dasher.png dasher.schemas
