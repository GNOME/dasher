LIBS = @INTLLIBS@ @LIBS@

noinst_LIBRARIES = libdashergtk.a
libdashergtk_a_SOURCES = GtkDasherCanvas.h GtkDasherEdit.h GtkDasherPane.h GtkDasherWindow.h GtkDoubleBuffer.h GtkDasherCanvas.cc GtkDasherEdit.cc GtkDasherPane.cc GtkDasherWindow.cc GtkDoubleBuffer.cc GtkDasherSave.h GtkDasherSave.cc GtkDasherStore.h GtkDasherStore.cc GtkDasherSlider.h GtkDasherSlider.cc GtkDasherAlphabetBox.h GtkDasherAlphabetBox.cc
AM_CXXFLAGS = -O2 $(GTKMM_CFLAGS) -I../DasherCore -DPROGDATA=\"$(pkgdatadir)\" -I../../intl -I$(top_srcdir)/intl
