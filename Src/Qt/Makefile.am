QTDIR=/usr/share/qte2

noinst_LIBRARIES = libdasherqt.a

CXXFLAGS = -O2 -fno-rtti -DQWS -I../DasherCore -I$(QTDIR)/include -DPROGDATA=\"$(pkgdatadir)/\"

libdasherqt_a_SOURCES = QtDasherEdit.cc QtDasherScreen.cc QtDasherWidget.cc moc_QtDasherScreen.cc moc_QtDasherWidget.cc moc_QtDasherEdit.cc

moc_%.cc: %.h
	/usr/share/qte2/bin/moc $< -o $@
