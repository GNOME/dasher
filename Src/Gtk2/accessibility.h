#ifndef __accessibility_h
#define __accessibility_h

#include <stdio.h>
#include <string.h>
#include <bonobo/bonobo-exception.h>
#include <sys/types.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <cspi/spi.h>
#include <libgnomeui/gnome-ui-init.h>
#include <popt.h>
#include <vector>
#include "DasherTypes.h"

using namespace Dasher;

ControlTree* gettree();
bool buildmenutree(Accessible *parent, ControlTree *ctree);
#endif
