QTDIR=/opt/Qtopia/sharp

AM_CXXFLAGS = -fno-rtti -DQWS -I../DasherCore -I$(QTDIR)/include -DPROGDATA=\"$(pkgdatadir)/\"

SOURCES = QtDasherScreen.cc moc_QtDasherScreen.cc QtDasherImpl.cc moc_QtDasherImpl.cc QtDasherPlugin.cc moc_QtDasherPlugin.cc

BINARIES = QtDasherScreen.o moc_QtDasherScreen.o QtDasherImpl.o moc_QtDasherImpl.o QtDasherPlugin.o moc_QtDasherPlugin.o

libqdasher.so.0.0.0: $(BINARIES)
	$(CXX) -shared $(BINARIES) ../DasherCore/libdashercore.a -L${QTDIR}/lib -lqte -lqpe -Wl,-static -lexpat -Wl,-soname -Wl,libqdasher.so.0 -o $@
	$(STRIP) $@

moc_%.cc: %.h
	$(QTDIR)/bin/moc $< -o $@

%.o: %.cc
	$(CXX) -c $(AM_CXXFLAGS) $(CXXFLAGS) $< -o $@

all: libqdasher.so.0.0.0

clean:
	-rm *.o moc* libqdasher.so.0.0.0





















