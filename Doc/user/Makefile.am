man_MANS = dasher.1 

EXTRA_DIST = dasher.1.in 
DISTCLEANFILES=dasher.1

dasher.1: $(srcdir)/dasher.1.in
	sed -e s/PACKAGE_VERSION/$(PACKAGE_VERSION)/ -e s:DOCDIR:$(docdir): < $(srcdir)/dasher.1.in > dasher.1
