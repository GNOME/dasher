PREFIX = /usr/local
PACKAGE = dasher

GTKCFLAGS = `pkg-config --cflags gtk+-2.0` 
FOO = -DGTK_ENABLE_BROKEN
GTKLDFLAGS += `pkg-config --libs gtk+-2.0`

CPPFLAGS += -g -D_GNU_SOURCE

# was -Wall as well
CFLAGS += -DPACKAGE=\"dasher\" -DPREFIX=\"$(PREFIX)\" -DPACKAGE_LOCALE_DIR=\"$(PREFIX)/share/locale\" $(GTKCFLAGS)
CFLAGS += -MD

CXXFLAGS = $(CFLAGS) -I../DasherCore -I /usr/local/include/libgpewidget

MEMBERS = main

DEPS = $(patsubst %,%.d,$(MEMBERS))
OBJS = $(patsubst %,%.o,$(MEMBERS))
SOURCES = $(patsubst %,%.cc,$(MEMBERS))

all: dasher

dasher: $(OBJS)
	g++ -o $@ $^ -L../DasherCore -lgpewidget -ldashercore -lstdc++ -lexpat $(GTKLDFLAGS) -g
#	g++ -o $@ $^ -L../DasherCore -lgpewidget -ldashercore -lstdc++ -lexpat $(GTKLDFLAGS) -g

clean:
	rm -f $(OBJS) $(DEPS)

install: all
	install -D $(PACKAGE) $(DESTDIR)$(PREFIX)/bin/$(PACKAGE)
#        strip $(DESTDIR)$(PREFIX)/bin/$(PACKAGE)

-include $(DEPS)



