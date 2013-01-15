#ifndef DASHER_EDITOR_EXTERNAL_H
#define DASHER_EDITOR_EXTERNAL_H

#include "dasher_editor.h"

void dasher_editor_external_finalize(GObject*);
void dasher_editor_external_create_buffer(DasherEditor*); //  for dasher_editor_external_initialise, and calls focus bits
void dasher_editor_external_output(DasherEditor *pSelf, const char *szText, int iOffset);
void dasher_editor_external_delete(DasherEditor *pSelf, int iLength, int iOffset);
const char * dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength);
int dasher_editor_external_get_offset(DasherEditor *pSelf);

#endif
