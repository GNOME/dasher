#ifndef DASHER_EDITOR_EXTERNAL_H
#define DASHER_EDITOR_EXTERNAL_H

#include "dasher_editor.h"

void dasher_editor_external_finalize(GObject*);
void dasher_editor_external_create_buffer(DasherEditor*); //  for dasher_editor_external_initialise, and calls focus bits
void dasher_editor_external_output(DasherEditor *pSelf, const char *szText, int iOffset);
void dasher_editor_external_delete(DasherEditor *pSelf, int iLength, int iOffset);
std::string dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength);
int dasher_editor_external_get_offset(DasherEditor *pSelf);
void dasher_editor_external_toggle_direct_mode(DasherEditor *, bool);
void dasher_editor_external_move(DasherEditor *pSelf, bool bForwards, Dasher::CControlManager::EditDistance iDist);
std::string dasher_editor_external_get_text_around_cursor(
    DasherEditor *pSelf, Dasher::CControlManager::EditDistance distance);
#endif
