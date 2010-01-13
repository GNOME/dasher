SUBDIRS =  training alphabets colours controllabels GUI Help

desktopdir = $(datadir)/applications
desktop_in_files = dasher.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
if USE_INTLTOOL
@INTLTOOL_DESKTOP_RULE@
else
dasher.desktop:	dasher.desktop.in
	cp dasher.desktop.in dasher.desktop
endif

icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = dasher.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = dasher.svg

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

if GCONF_SCHEMAS_INSTALL
schemasdir = $(GCONF_SCHEMA_FILE_DIR)
schemas_DATA = dasher.schemas

install-data-local: install-schemas

# A note on what happens here, as it's a custome step. The GConf
# schema is generated directely from the data structures in the
# SettingsStore and AppSettings objects, so they stay in sync. This is
# done by building generate-schema, a small, non-installed program in
# Src/Gtk2/. At install time this gets run to build the schema file,
# which is then moved installed in the usual way. None of this happens
# if configure is told not to install schemas.

dasher.schemas: ../Src/Gtk2/generate-schema
	echo -n "Generating schema ... "; \
	../Src/Gtk2/generate-schema > dasher.schemas; \
	echo "done.";

install-schemas: dasher.schemas
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
	gconftool-2 --makefile-install-rule dasher.schemas
else
install-data-local:
endif


install-data-hook: update-icon-cache

uninstall-hook: update-icon-cache

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi

EXTRA_DIST = \
		dasher.desktop \
		$(icon_DATA) \
		$(svgicon_DATA)

DISTCLEANFILES = dasher.schemas

