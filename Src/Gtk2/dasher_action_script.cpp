#include "dasher_action_script.h"
#include "dasher_editor.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void dasher_action_script_class_init(DasherActionScriptClass *pClass);
static void dasher_action_script_init(DasherActionScript *pActionScript);
static void dasher_action_script_destroy(GObject *pObject);
static gboolean dasher_action_script_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx);
static const gchar *dasher_action_script_get_name(DasherAction *pSelf);

typedef struct _DasherActionScriptPrivate DasherActionScriptPrivate;

struct _DasherActionScriptPrivate {
  gchar *szPath;
  gchar *szFilename;
};

GType dasher_action_script_get_type() {

  static GType dasher_action_script_type = 0;

  if(!dasher_action_script_type) {
    static const GTypeInfo dasher_action_script_info = {
      sizeof(DasherActionScriptClass),
      NULL,
      NULL,
      (GClassInitFunc) dasher_action_script_class_init,
      NULL,
      NULL,
      sizeof(DasherActionScript),
      0,
      (GInstanceInitFunc) dasher_action_script_init,
      NULL
    };

    dasher_action_script_type = g_type_register_static(TYPE_DASHER_ACTION, "DasherActionScript", &dasher_action_script_info, static_cast < GTypeFlags > (0));
  }

  return dasher_action_script_type;
}

static void dasher_action_script_class_init(DasherActionScriptClass *pClass) {
  GObjectClass *pObjectClass = (GObjectClass *) pClass;
  pObjectClass->finalize = dasher_action_script_destroy;

  DasherActionClass *pDasherActionClass = (DasherActionClass *) pClass;
  pDasherActionClass->execute = dasher_action_script_execute;
  pDasherActionClass->get_name = dasher_action_script_get_name;
}

static void dasher_action_script_init(DasherActionScript *pDasherControl) {
  pDasherControl->private_data = new DasherActionScriptPrivate;
}

static void dasher_action_script_destroy(GObject *pObject) {
  // FIXME - I think we need to chain up through the finalize methods
  // of the parent classes here...
}

DasherActionScript *dasher_action_script_new(const gchar *szPath, const gchar *szFilename) {
  DasherActionScript *pDasherControl;
  pDasherControl = (DasherActionScript *)(g_object_new(dasher_action_script_get_type(), NULL));

  DasherActionScriptPrivate *pPrivate((DasherActionScriptPrivate *)pDasherControl->private_data);
  pPrivate->szPath = new gchar[strlen(szPath)+1];
  strncpy(pPrivate->szPath, szPath, strlen(szPath) + 1);
  pPrivate->szFilename = new gchar[strlen(szFilename)+1];
  strncpy(pPrivate->szFilename, szFilename, strlen(szFilename) + 1);

  return pDasherControl;
}

static gboolean dasher_action_script_execute(DasherAction *pSelf, DasherEditor *pEditor, int iIdx) {
  DasherActionScriptPrivate *pPrivate((DasherActionScriptPrivate *)((DasherActionScript *)pSelf)->private_data);

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

static const gchar *dasher_action_script_get_name(DasherAction *pSelf) {
  return "Script";
}
