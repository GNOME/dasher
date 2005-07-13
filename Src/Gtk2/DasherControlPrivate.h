#ifndef __dashercontrol_private_h__
#define __dashercontrol_private_h__

#include "DasherControl.h"
#include "mouse_input.h"

extern gboolean paused;
extern gboolean training;
extern GAsyncQueue* trainqueue;
extern GtkWidget *train_dialog;
extern std::string alphabet;
extern ControlTree *controltree;
extern bool eyetrackermode;
extern gboolean direction;
extern bool onedmode;
extern gint dasherwidth;
extern gint dasherheight;
extern long yscale;
extern long mouseposstartdist;
extern gboolean mouseposstart;
extern gboolean firstbox;
extern gboolean secondbox;
extern time_t starttime;
extern time_t starttime2;
extern int oldx;
extern int oldy;
extern time_t dasherstarttime;
extern CDasherMouseInput *pMouseInput;

// Classes - to eventually be split into separate files

#endif
