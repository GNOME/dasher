#include <gtk/gtk.h>
#include <stdio.h>
#include "fileops.h"
#include "dasher.h"
#include "edit.h"

extern gboolean file_modified;
extern const gchar* filename;
extern GtkWidget *window;
extern gint fileencoding;

extern "C" void 
open_file (const char *myfilename)
{
  unsigned long long size;
  gchar *buffer;

#ifdef GNOME_LIBS
  if (gnome_vfs_open_file(myfilename, &buffer, &size)==FALSE) {
    return;
  }
#else
  if (unix_vfs_open_file(myfilename, &buffer, &size)==FALSE) {
    return;
  }
#endif

  dasher_clear();
  
  file_modified = TRUE;

  if (size!=0) {
    // Don't attempt to insert new text if the file is empty as it makes
    // GTK cry
    if (g_utf8_validate(buffer,size,NULL)==FALSE) {
      // It's not UTF8, so we do the best we can...
      gchar* buffer2=g_locale_to_utf8(buffer,size,NULL,NULL,NULL);
      g_free(buffer);
      buffer=buffer2;
    }
    gtk_text_buffer_insert_at_cursor(GTK_TEXT_BUFFER (the_text_buffer), buffer, size);
    gtk_text_view_scroll_mark_onscreen(GTK_TEXT_VIEW (the_text_view),gtk_text_buffer_get_insert(GTK_TEXT_BUFFER(the_text_buffer)));
  }

  gtk_window_set_title(GTK_WINDOW(window), myfilename);

  if (filename!=myfilename) {
    g_free((void *)filename);
    filename = g_strdup(myfilename);
  }

  dasher_start();
  dasher_redraw();
}

#ifdef GNOME_LIBS
gboolean gnome_vfs_open_file (const char* myfilename, gchar** buffer, unsigned long long* size)
{
  GnomeVFSHandle *read_handle;
  GnomeVFSResult result;
  GnomeVFSFileInfo info;
  GnomeVFSFileSize bytes_read;
  GnomeVFSURI *uri;

  uri=gnome_vfs_uri_new(myfilename);

  if (uri==NULL) { // It's not a URI we can cope with - assume it's a filename
    char *tmpfilename=gnome_vfs_get_uri_from_local_path(myfilename);
    if (myfilename!=filename) {
      g_free((void *)myfilename);
    }
    myfilename=tmpfilename;
    uri=gnome_vfs_uri_new(myfilename);
    if (uri==NULL) {
      return FALSE;
    }
  }

  result=gnome_vfs_open_uri (&read_handle, uri, GNOME_VFS_OPEN_READ);
  if (result != GNOME_VFS_OK) {
    vfs_print_error(&result,myfilename);
    g_free(uri);
    return FALSE;
  }

  result=gnome_vfs_get_file_info_uri(uri,&info,GNOME_VFS_FILE_INFO_FOLLOW_LINKS);
  if (result != GNOME_VFS_OK) {
    vfs_print_error(&result,myfilename);
    g_free(uri);
    return FALSE;
  }

  *size = (gint) info.size;
  *buffer = (gchar *) g_malloc (*size);
  result = gnome_vfs_read (read_handle, *buffer, *size, &bytes_read);
  if (result != GNOME_VFS_OK) {
    vfs_print_error(&result,myfilename);
    g_free(uri);
    return FALSE;
  }
  gnome_vfs_close(read_handle);
  g_free(uri);
  return TRUE;
}
#endif

gboolean unix_vfs_open_file (const char* myfilename, gchar** buffer, unsigned long long* size)
{
  GtkWidget *error_dialog;

  struct stat file_stat;
  FILE *fp;
  int pos = 0;

  stat (myfilename, &file_stat);
  fp = fopen (myfilename, "r");

  if (fp==NULL || S_ISDIR(file_stat.st_mode)) {
    error_dialog = gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK, "Could not open the file \"%s\".\n", myfilename);
    gtk_dialog_set_default_response(GTK_DIALOG (error_dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(error_dialog), FALSE);
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
    return FALSE;
  }

  *size=file_stat.st_size;
  *buffer = (gchar *) g_malloc (*size);
  fread (*buffer, *size, 1, fp);
  fclose (fp);
  return TRUE;
}


extern "C" bool
save_file_as (const char *myfilename, bool append)
{
  unsigned long long length;
  gchar *inbuffer,*outbuffer = NULL;
  gsize bytes_read, bytes_written;
  GError *error = NULL;
  GtkTextIter *start, *end;
  GIConv cd;

  start = new GtkTextIter;
  end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),start,0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),end,-1);

  inbuffer = gtk_text_iter_get_slice (start,end);

  length = gtk_text_iter_get_offset(end)-gtk_text_iter_get_offset(start);
  outbuffer = (char *)malloc((length+1)*sizeof(gchar));
  memcpy((void*)outbuffer,(void*)inbuffer,length*sizeof(gchar));
  outbuffer[length]=0;
  g_free(inbuffer);
  inbuffer=outbuffer;
  outbuffer=NULL;

  switch (fileencoding) {
  case Opts::UserDefault:
  case Opts::AlphabetDefault:
    //FIXME - need to call GetAlphabetType and do appropriate stuff regarding 
    //the character set. Arguably we should always be saving in either UTF-8 or
    //the user's locale (which may, of course, be UTF-8) because otherwise 
    //we're going to read in rubbish, and we shouldn't be encouraging weird 
    //codepage madness any further

    //FIXME - error handling
    outbuffer=g_locale_from_utf8(inbuffer,-1,&bytes_read,&bytes_written,&error);
    if (outbuffer==NULL) {
      // We can't represent the text in the current locale, so fall back to
      // UTF-8
      outbuffer=inbuffer;
      bytes_written=length;
    }
  case Opts::UTF8:
    outbuffer=inbuffer;
    bytes_written=length;
    break;
  // Does /anyone/ want to save text files in UTF16?
  // (in any case, my opinions regarding encouragement of data formats with
  // endianness damage are almost certainly unprintable)

  case Opts::UTF16LE:
    cd=g_iconv_open("UTF16LE","UTF8");
    outbuffer=g_convert_with_iconv(inbuffer,-1,cd,&bytes_read,&bytes_written,&error);
    break;
  case Opts::UTF16BE:
    cd=g_iconv_open("UTF16BE","UTF8");
    outbuffer=g_convert_with_iconv(inbuffer,-1,cd,&bytes_read,&bytes_written,&error);
    break;
  default:
    outbuffer=inbuffer;
    bytes_written=length;
  }

#ifdef GNOME_LIBS
  if (gnome_vfs_save_file(myfilename,outbuffer, bytes_written, append)==FALSE) {
    return false;
  }
#else
  if (unix_vfs_save_file(myfilename,outbuffer, bytes_written, append)==FALSE) {
    return false;
  }
#endif

  file_modified = FALSE;
  gtk_window_set_title(GTK_WINDOW(window), myfilename);

  if (filename!=myfilename) {
    g_free((void *)filename);
    filename = g_strdup(myfilename);
  }

  return true;
}

gboolean unix_vfs_save_file(const char* myfilename, gchar* buffer, unsigned long long length, bool append) {
  int opened=1;
  GtkWidget *error_dialog;

  FILE *fp;

  if (append == true) {
    fp = fopen (myfilename, "a");

    if (fp == NULL) {
      opened = 0;
    }
  } else {
    fp = fopen (myfilename, "w");
    if (fp == NULL) {
      opened = 0;
    }
  }

  if (!opened) {
    error_dialog = gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK, "Could not save the file \"%s\".\n", myfilename);
    gtk_dialog_set_default_response(GTK_DIALOG (error_dialog), GTK_RESPONSE_OK);
    gtk_window_set_resizable(GTK_WINDOW(error_dialog), FALSE);
    gtk_dialog_run(GTK_DIALOG(error_dialog));
    gtk_widget_destroy(error_dialog);
    return false;
  }

  fwrite(buffer,1,length,fp);
  fclose (fp);
}

#ifdef GNOME_LIBS
gboolean gnome_vfs_save_file(const char* myfilename, gchar* buffer, unsigned long long length, bool append) {
  GnomeVFSHandle *write_handle;
  GnomeVFSResult result;
  GnomeVFSFileInfo info;
  GnomeVFSFileSize bytes_written;
  GnomeVFSURI *uri;

  uri=gnome_vfs_uri_new(myfilename);

  if (uri==NULL) { // It's not a URI we can cope with - assume it's a filename
    char *tmpfilename=gnome_vfs_get_uri_from_local_path(myfilename);
    if (myfilename!=filename) {
      g_free((void *)myfilename);
    }
    myfilename=tmpfilename;
    uri=gnome_vfs_uri_new(myfilename);
    if (uri==NULL) {
      return FALSE;
    }
  }

  result = gnome_vfs_create_uri (&write_handle, uri, GnomeVFSOpenMode(GNOME_VFS_OPEN_WRITE|GNOME_VFS_OPEN_RANDOM), TRUE, 0666);

  if (result==GNOME_VFS_ERROR_FILE_EXISTS) {
    if (append) {
      result = gnome_vfs_open_uri (&write_handle, uri, GnomeVFSOpenMode(GNOME_VFS_OPEN_WRITE|GNOME_VFS_OPEN_RANDOM));
    } else {
      result = gnome_vfs_create_uri (&write_handle, uri, GnomeVFSOpenMode(GNOME_VFS_OPEN_WRITE|GNOME_VFS_OPEN_RANDOM), FALSE, 0666);
    }
  }

  if (result != GNOME_VFS_OK) {
    vfs_print_error(&result,myfilename);
    g_free(uri);
    return FALSE;
  }

  if (append) {
    result=gnome_vfs_seek(write_handle,GNOME_VFS_SEEK_END,0);
    if (result != GNOME_VFS_OK) {
      vfs_print_error(&result,myfilename);
      g_free(uri);
      return FALSE;
    }
  }

  result=gnome_vfs_write(write_handle,buffer,length, &bytes_written);
  if (result != GNOME_VFS_OK) {
    vfs_print_error(&result,myfilename);
    g_free(uri);
    return FALSE;
  }

  gnome_vfs_close(write_handle);
  g_free(uri);
  return TRUE;
}

void
vfs_print_error(GnomeVFSResult *result, const char *myfilename)
{
  // Turns a Gnome VFS error into English
  GtkWidget *error_dialog;
  error_dialog = gtk_message_dialog_new(GTK_WINDOW(window),GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK, "Could not open the file \"%s\"\n%s\n", myfilename,gnome_vfs_result_to_string (*result));
  gtk_dialog_set_default_response(GTK_DIALOG (error_dialog), GTK_RESPONSE_OK);
  gtk_window_set_resizable(GTK_WINDOW(error_dialog), FALSE);
  gtk_dialog_run(GTK_DIALOG(error_dialog));
  gtk_widget_destroy(error_dialog);
  return;
}
#endif
