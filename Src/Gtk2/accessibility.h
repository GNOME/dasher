#ifndef __accessibility_h
#define __accessibility_h

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <gtk/gtk.h>
#ifdef GNOME_A11Y
#include <bonobo/bonobo-exception.h>
#include <cspi/spi.h>
#include <libgnomeui/gnome-ui-init.h>
#endif
#include <popt.h>
#include <vector>
#include "DasherTypes.h"

using namespace Dasher;

ControlTree* gettree();
ControlTree* buildcontroltree();
ControlTree* buildmovetree(ControlTree* movetree);
ControlTree* builddeletetree(ControlTree* movetree);
#ifdef GNOME_A11Y
bool buildmenutree(Accessible *parent, ControlTree *ctree);
#endif
#endif
