#ifndef __dasher_editor_external_h__
#define __dasher_editor_external_h__

typedef struct _DasherEditor DasherEditor;
struct _DasherEditor;

void dasher_editor_external_create_buffer(DasherEditor*); //  for dasher_editor_external_initialise, and calls focus bits
/* dasher_editor_external_output just calls SPI sendText */
void dasher_editor_external_output(DasherEditor *pSelf, const gchar *szText, int iOffset);
void dasher_editor_external_delete(DasherEditor *pSelf, int iLength, int iOffset);
const gchar * dasher_editor_external_get_context(DasherEditor *pSelf, int iOffset, int iLength);
gint dasher_editor_external_get_offset(DasherEditor *pSelf);

#endif
