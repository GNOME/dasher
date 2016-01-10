SUBDIRS =  training alphabets colours control settings GUI Help

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

CLEANFILES = dasher.schemas dasher.gschema.xml
