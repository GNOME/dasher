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

enum accessibletype { menus=1, toolbars=2, pushbuttons=3, focusables=4 };

void setupa11y();
void cleanupa11y();

ControlTree* gettree();
ControlTree* buildcontroltree();
ControlTree* buildmovetree(ControlTree* movetree);
ControlTree* builddeletetree(ControlTree* movetree);
ControlTree* buildspeaktree(ControlTree* speaktree);
void deletemenutree();
#ifdef GNOME_A11Y
bool buildmenutree(Accessible *parent, ControlTree *ctree, accessibletype Type);
ControlTree* getmenutree();
gboolean dasher_check_window(AccessibleRole role);
gboolean findpanels(Accessible *parent);
#endif
#endif

