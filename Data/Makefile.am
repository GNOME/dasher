SUBDIRS = Help Translations system.rc

Applicationsdir = $(datadir)/applications
Applications_DATA = dasher.desktop

appicondir = $(datadir)/icons/hicolor/48x48/apps
appicon_DATA = dasher.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = dasher.svg

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

install-schemas:
	      GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	      gconftool --makefile-install-rule dasher.schemas

EXTRA_DIST = dasher.desktop dasher.png dasher.svg dasher.schemas
