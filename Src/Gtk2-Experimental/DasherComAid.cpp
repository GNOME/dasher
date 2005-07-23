#include <gtk/gtk.h>
#include <gdk/gdk.h>

#ifdef GNOME_LIBS
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#endif

#include <pango/pango.h>

#include <string.h>

#include "../Gtk2/GtkDasherControl.h"

extern "C" void insert_callback( GtkDasherControl *pDasherControl, const gchar *szText, gpointer pUserData );
extern "C" void delete_callback( GtkDasherControl *pDasherControl, const gchar *szText, gpointer pUserData );

int main( int argc, char **argv ) {

#ifdef GNOME_LIBS
  gnome_program_init("Dasher", "0.1", LIBGNOMEUI_MODULE, argc, argv, GNOME_PARAM_NONE );
#endif
  gtk_init( &argc, &argv );
  
  GtkWidget *pWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );
  gtk_window_fullscreen(GTK_WINDOW(pWindow));

  GtkWidget *pEditBox;
  pEditBox = gtk_text_view_new();
  gtk_widget_set_size_request(pEditBox, -1, 128);
  gtk_widget_modify_font(pEditBox, pango_font_description_from_string("Sans 36"));
  
  GtkTextBuffer *pTextBuffer( gtk_text_view_get_buffer( GTK_TEXT_VIEW(pEditBox)));

  GtkWidget *pDasherControl;
  pDasherControl = gtk_dasher_control_new();
  
  g_signal_connect(pDasherControl, "dasher_edit_insert", G_CALLBACK(insert_callback), pTextBuffer);
  g_signal_connect(pDasherControl, "dasher_edit_delete", G_CALLBACK(delete_callback), pTextBuffer);

  GtkWidget *pVBox;
  pVBox = gtk_vbox_new(false, 0);

  gtk_box_pack_start( GTK_BOX( pVBox ), pEditBox, false, false, 0 );
  gtk_box_pack_start( GTK_BOX( pVBox ), pDasherControl, true, true, 0 );

  gtk_container_add( GTK_CONTAINER( pWindow ), pVBox );

  gtk_widget_show_all( pWindow );
  
  gtk_main();

  return 0;
}

extern "C" void insert_callback( GtkDasherControl *pDasherControl, const gchar *szText, gpointer pUserData ) {
  gtk_text_buffer_insert_at_cursor( (GtkTextBuffer*)pUserData, szText, -1);
}

extern "C" void delete_callback( GtkDasherControl *pDasherControl, const gchar *szText, gpointer pUserData ) {

  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;
  int length;

  length = strlen( szText );
  gtk_text_buffer_get_iter_at_mark( (GtkTextBuffer*)pUserData, end, gtk_text_buffer_get_insert( (GtkTextBuffer*)pUserData));

  *start = *end;

  gtk_text_iter_backward_chars(start, length);

  gtk_text_buffer_delete( (GtkTextBuffer*)pUserData, start, end);


}
