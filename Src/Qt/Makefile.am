QTDIR=/usr/share/qt3

noinst_LIBRARIES = libdasherqt.a

AM_CXXFLAGS = -O2 -fno-rtti -I../DasherCore -I$(QTDIR)/include -DPROGDATA=\"$(pkgdatadir)/\"

libdasherqt_a_SOURCES = QtDasherPrefs.cc QtDasherEdit.cc QtDasherScreen.cc QtDasherWidget.cc moc_QtDasherScreen.cc moc_QtDasherWidget.cc moc_QtDasherEdit.cc moc_QtDasherPrefs.cc

moc_%.cc: %.h
	/usr/share/qt3/bin/moc $< -o $@
