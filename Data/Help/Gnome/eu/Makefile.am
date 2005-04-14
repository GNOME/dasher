figdir = figures
docname = dasher
lang = eu
omffile = dasher-eu.omf
include $(top_srcdir)/Data/Help/Gnome/xmldocs.make
dist-hook: app-dist-hook
entities = version.xml
DISTCLEANFILES=dasher-eu.omf.out version.xml

noinst_SCRIPTS = version.xml

version.xml:
	echo $(PACKAGE_VERSION) >version.xml
