SUBDIRS =  Translations system.rc Help
# Help - commented out for now


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

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

if GCONF_SCHEMAS_INSTALL
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = dasher.schemas

install-data-local: install-schemas

dasher.schemas:
	echo -n "Generating schema ... "; \
	../Src/Gtk2/generate-schema > dasher.schemas; \
	echo "done.";

install-schemas: dasher.schemas
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	gconftool --makefile-install-rule dasher.schemas
else
install-data-local:
endif


install-data-hook:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

EXTRA_DIST = dasher.desktop dasher.png dasher.svg
