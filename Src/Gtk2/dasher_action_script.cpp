#include "dasher_action_script.h"
#include "dasher_editor.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _DasherActionScriptPrivate DasherActionScriptPrivate;

struct _DasherActionScriptPrivate {
  gchar *szPath;
  gchar *szFilename;
};


#define DASHER_ACTION_SCRIPT_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), TYPE_DASHER_ACTION_SCRIPT, DasherActionScriptPrivate))

G_DEFINE_TYPE(DasherActionScript, dasher_action_script, TYPE_DASHER_ACTION);

static gboolean dasher_action_script_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx);
static const gchar *dasher_action_script_get_name(DasherAction *pSelf);

static void 
dasher_action_script_class_init(DasherActionScriptClass *pClass) {
  g_type_class_add_private(pClass, sizeof(DasherActionScriptPrivate));

  DasherActionClass *pDasherActionClass = (DasherActionClass *) pClass;
  pDasherActionClass->execute = dasher_action_script_execute;
  pDasherActionClass->get_name = dasher_action_script_get_name;
}

static void 
dasher_action_script_init(DasherActionScript *pDasherControl) {
  DasherActionScriptPrivate *pPrivate = DASHER_ACTION_SCRIPT_GET_PRIVATE(pDasherControl);

  pPrivate->szPath = NULL;
  pPrivate->szFilename = NULL;
}

DasherActionScript *
dasher_action_script_new(const gchar *szPath, const gchar *szFilename) {
  DasherActionScript *pDasherControl;
  pDasherControl = (DasherActionScript *)(g_object_new(dasher_action_script_get_type(), NULL));

  DasherActionScriptPrivate *pPrivate = DASHER_ACTION_SCRIPT_GET_PRIVATE(pDasherControl);
  pPrivate->szPath = g_strdup(szPath);
  pPrivate->szFilename = g_strdup(szFilename);

  return pDasherControl;
}

static gboolean 
dasher_action_script_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx) {
  DasherActionScriptPrivate *pPrivate = DASHER_ACTION_SCRIPT_GET_PRIVATE(pSelf);

  gchar *szFullPath = new gchar[strlen(pPrivate->szPath) + strlen(pPrivate->szFilename) + 1];
  strncpy(szFullPath, pPrivate->szPath, strlen(pPrivate->szPath) + 1);
  strcat(szFullPath, pPrivate->szFilename);
  
  // TODO: Really should do more error checking here

  // Create a pipe
  int iPipeFDs[2];
  pipe(iPipeFDs);

  if(fork() == 0) {
    // Do the pipe duplication foobar
    close(0); // Close stdin
    dup(iPipeFDs[0]);
    close(iPipeFDs[0]);
    close(iPipeFDs[1]);

    execl(szFullPath, pPrivate->szFilename, NULL);
    // Under normal conditions execution stops here

    g_warning("Could not execute script");
    exit(1);
  }

  const char *szData = dasher_editor_get_all_text(pEditor);

  // Write data to the pipe
  write(iPipeFDs[1], szData, strlen(szData));
  close(iPipeFDs[1]);

  return true;
}

static const gchar *
dasher_action_script_get_name(DasherAction *pSelf) {
  return _("Script");
}
