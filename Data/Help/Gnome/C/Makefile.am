figdir = figures
docname = dasher
lang = C
omffile = dasher-C.omf
include $(top_srcdir)/Data/Help/Gnome/xmldocs.make
dist-hook: app-dist-hook
entities = version.xml
DISTCLEANFILES=dasher-C.omf.out version.xml

noinst_SCRIPTS = version.xml

version.xml:
	echo $(PACKAGE_VERSION) >version.xml