#include "../Common/Common.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include "fileops.h"
#include "dasher.h"
#include "../DasherCore/DasherTypes.h"

gboolean file_modified;
extern const gchar *filename;
//extern GtkWidget *window;
//extern gint fileencoding;

static GtkWidget *window = NULL; // TODO: Fix this

static GtkWidget *the_text_view = NULL; // TODO: fix
static GtkTextBuffer *the_text_buffer = NULL; // TODO: fix


