SUBDIRS = Translations system.rc

Applicationsdir = $(datadir)/applications
Applications_DATA = dasher.desktop
Iconsdir = $(datadir)/icons
Icons_DATA = dasher.png

install-schemas:
	      GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	      gconftool --makefile-install-rule dasher.schemas
