#ifndef __dasher_spi_h__
#define __dasher_spi_h__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GNOME_A11Y
bool initSPI();
#endif
///tries to send text to another app (the currently focused one?)
/// - using ATSPI if we've got GNOME_A11Y defined, else with X events
void sendText(const char *szText);
#endif
