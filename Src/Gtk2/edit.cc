#include "edit.h"
#include "dasher.h"
#include "accessibility.h"
#include "canvas.h"

extern int paused;
extern bool keyboardmodeon;
extern bool mouseposstart;
extern bool onedmode;

#include <gdk/gdkx.h>

#ifdef GNOME_SPEECH
#include "speech.h"
#endif

GtkWidget *the_text_view;  
GtkTextBuffer *the_text_buffer;
GtkClipboard *the_text_clipboard;
std::string last_said;
std::string say;
std::string outputtext;

gunichar* wideoutput;

extern gint outputcharacters;
extern bool file_modified;
void initialise_edit()
{
  the_text_clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  the_text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (the_text_view));
}

void handle_cursor_move(GtkTextView *textview, GtkMovementStep arg1, gint arg2, gboolean arg3, gpointer data)
{
  dasher_start();
  dasher_redraw();
}

void edit_output_callback(symbol Symbol)
{
  std::string label;
  label = dasher_get_edit_text( Symbol );

#ifdef GNOME_SPEECH
  say+=label;
#endif

  outputtext+=label;
  file_modified=true;

  gtk_text_buffer_insert_at_cursor(the_text_buffer, label.c_str(), -1);
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));

  if (keyboardmodeon==true) {
#ifdef GNOME_A11Y
#ifdef X_HAVE_UTF8_STRING
    Display *dpy = gdk_x11_get_default_xdisplay();
    int min, max, numcodes;
    KeySym *keysym;
    KeyCode code;
    glong numoutput;
    
    wideoutput=g_utf8_to_ucs4(label.c_str(),-1,NULL,&numoutput,NULL);
    for (size_t i=0; i<numoutput; i++) {
      // This gives us the magic X keysym
      wideoutput[i]=wideoutput[i] | 0x01000000;
      
      XDisplayKeycodes(dpy,&min,&max);
      keysym = XGetKeyboardMapping(dpy,min,max-min+1,&numcodes);
      keysym[(max-min-1)*numcodes]=wideoutput[i];
      XChangeKeyboardMapping(dpy,min,numcodes,keysym,(max-min));
      XSync(dpy,true);
      XFree(keysym);
      code = XKeysymToKeycode(dpy,wideoutput[i]);    
      if (code!=0) {
	XTestFakeKeyEvent(dpy, code, True, CurrentTime);
	XSync(dpy,true);
	XTestFakeKeyEvent(dpy, code, False, CurrentTime);
	XSync(dpy,true);
      }
    }
    XSync(dpy,true);
    g_free(wideoutput);
#else
    SPI_generateKeyboardEvent(0,(char*)label.c_str(),SPI_KEY_STRING);
#endif
#endif
  }
  outputcharacters++;
}

void write_to_file()
{
  std::string filename=dasher_get_training_file();
  int fd=open(filename.c_str(),O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
  write(fd,outputtext.c_str(),outputtext.length());
  close(fd);
  outputtext="";
}

void edit_outputcontrol_callback(void* pointer, int data)
{
  switch(data) {
#ifdef GNOME_A11Y
  case 1:
    if (pointer!=NULL) {
      Accessible *myfoo;
      myfoo=(Accessible *)pointer;
      AccessibleAction_doAction(Accessible_getAction(myfoo),0);
      break;
    }
#endif
  case 2:
    // stop
    stop();
    break;
  case 3:
    //	pause
    dasher_pause(0,0);
    if (onedmode==true) {
      dasher_halt();
    }
    paused=true;
    if (mouseposstart==true)
      draw_mouseposbox(0);
    break;
  case 4:
#ifdef GNOME_SPEECH
    speak_buffer();
#endif
    break;
  case 5:
#ifdef GNOME_SPEECH
    speak();
#endif
    break;
  case 6:
#ifdef GNOME_SPEECH
    speak_last();
#endif
    break;
  case 11:
    // move left
    edit_move_back();
    break;
  case 12:
    // move right
    edit_move_forward();
    break;
  case 13:
    edit_move_start();
    break;
  case 14:
    edit_move_end();
    break;
  case 21:
    edit_delete_forward_character();
    break;
  case 22:
    edit_delete_forward_word();
    break;
  case 23:
    edit_delete_forward_line();
    break;
  case 24:
    edit_delete_callback();
    break;
  case 25:
    edit_delete_backward_word();
    break;
  case 26:
    edit_delete_backward_line();
    break;
  }
}

void edit_move_forward()
{
  GtkTextIter *pos = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,pos,gtk_text_buffer_get_insert(the_text_buffer));

  gtk_text_iter_forward_cursor_position(pos);

  gtk_text_buffer_place_cursor(the_text_buffer,pos);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void edit_move_back()
{
  GtkTextIter *pos = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,pos,gtk_text_buffer_get_insert(the_text_buffer));

  gtk_text_iter_backward_cursor_position(pos);

  gtk_text_buffer_place_cursor(the_text_buffer,pos);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void edit_move_start()
{
  GtkTextIter *pos = new GtkTextIter;

  gtk_text_buffer_get_start_iter(the_text_buffer,pos);

  gtk_text_buffer_place_cursor(the_text_buffer,pos);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void edit_move_end()
{
  GtkTextIter *pos = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,pos,gtk_text_buffer_get_insert(the_text_buffer));

  gtk_text_iter_forward_to_end(pos);

  gtk_text_buffer_place_cursor(the_text_buffer,pos);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void edit_delete_callback()
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,end,gtk_text_buffer_get_insert(the_text_buffer));

  *start=*end;  

  gtk_text_iter_backward_chars(start, 1);

  gtk_text_buffer_delete(the_text_buffer,start,end);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));

#ifdef GNOME_SPEECH
  if(say.length()>0) {
    say.resize(say.length()-1);
  }
#endif

  if(outputtext.length()>0) {
    outputtext.resize(outputtext.length()-1);
  }

  if (keyboardmodeon==true) {
#ifdef GNOME_A11Y
#ifdef X_HAVE_UTF8_STRING
    Display *dpy;
    dpy = gdk_x11_get_default_xdisplay();
    KeyCode code;
    code = XKeysymToKeycode(dpy,XK_BackSpace);
    XTestFakeKeyEvent(dpy, code, True, 0);
    XTestFakeKeyEvent(dpy, code, False, 0);
    XFlush(dpy);
#else
    SPI_generateKeyboardEvent(XK_BackSpace,NULL,SPI_KEY_SYM);
#endif
#endif
  }
  outputcharacters--;
}

void edit_delete_forward_character()
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,end,gtk_text_buffer_get_insert(the_text_buffer));

  *start=*end;  

  gtk_text_iter_forward_chars(end, 1);

  gtk_text_buffer_delete(the_text_buffer,start,end);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void edit_delete_forward_word()
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,end,gtk_text_buffer_get_insert(the_text_buffer));

  *start=*end;  

  gtk_text_iter_forward_word_ends(end, 1);

  gtk_text_buffer_delete(the_text_buffer,start,end);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void edit_delete_forward_line()
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,end,gtk_text_buffer_get_insert(the_text_buffer));

  *start=*end;  

  gtk_text_iter_forward_lines(end,1);

  gtk_text_buffer_delete(the_text_buffer,start,end);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void edit_delete_backward_word()
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,end,gtk_text_buffer_get_insert(the_text_buffer));

  *start=*end;  

  gtk_text_iter_backward_word_starts(start, 1);

  gtk_text_buffer_delete(the_text_buffer,start,end);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void edit_delete_backward_line()
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,end,gtk_text_buffer_get_insert(the_text_buffer));

  *start=*end;  

  gtk_text_iter_backward_lines(start,1);

  gtk_text_buffer_delete(the_text_buffer,start,end);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void clipboard_callback( clipboard_action act )
{
  switch( act )
    {
    case CLIPBOARD_CUT: 
      gtk_text_buffer_cut_clipboard(the_text_buffer, the_text_clipboard, TRUE);
      break;
    case CLIPBOARD_COPY:
      gtk_text_buffer_copy_clipboard(the_text_buffer, the_text_clipboard);
      break;
    case CLIPBOARD_PASTE:
      gtk_text_buffer_paste_clipboard(the_text_buffer, the_text_clipboard, NULL, TRUE);
      break;
    case CLIPBOARD_COPYALL:
      select_all();
      gtk_text_buffer_copy_clipboard(the_text_buffer, the_text_clipboard);
      break;
    case CLIPBOARD_SELECTALL:
      select_all();
      break;
    case CLIPBOARD_CLEAR:
      gtk_text_buffer_set_text (the_text_buffer,"",0);
      break;
    }
}

void select_all()
{
  GtkTextIter *start, *end;

  start = new GtkTextIter;
  end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),start,0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),end,-1);

  GtkTextMark *selection = gtk_text_buffer_get_mark (the_text_buffer,"selection_bound");
  GtkTextMark *cursor = gtk_text_buffer_get_mark(the_text_buffer,"insert");

  gtk_text_buffer_move_mark(the_text_buffer,selection,start);
  gtk_text_buffer_move_mark(the_text_buffer,cursor,end);
}

void clear_edit()
{
  GtkTextIter *start, *end;

  start = new GtkTextIter;
  end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),start,0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),end,-1); 

  gtk_text_buffer_delete(the_text_buffer,start,end);

}

void reset_edit_font()
{
  gtk_widget_modify_font (the_text_view,pango_font_description_from_string("Sans 10"));
}

void get_new_context_callback( std::string &str, int max )
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;

  gtk_text_buffer_get_selection_bounds(the_text_buffer,start,end);

  if (gtk_text_iter_get_offset(start)==gtk_text_iter_get_offset(end)) {
    // if there's no slection, just get the context from the cursor
    gtk_text_buffer_get_iter_at_mark(the_text_buffer,end,gtk_text_buffer_get_insert(the_text_buffer));
    *start=*end;  
  } else {
    // otherwise, we want to get the context from the left hand edge of
    // the selection rather than the right hand edge (which is where the 
    // cursor is)
    *end=*start;
  }

  gtk_text_iter_backward_chars(start, max);

  str = std::string( gtk_text_buffer_get_text( the_text_buffer, start, end, FALSE ) );
}

#ifdef GNOME_SPEECH

void speak()
{
  if (say.length()>0) {
    while (say.find("\"") != std::string::npos) {
      say.replace(say.find("\""), 1, "");
    }
    SPEAK_DAMN_YOU(&say);
    last_said = say;
  }
  say="";
}

void speak_last()
{
  if (last_said.length()>0) {
    while (last_said.find("\"") != std::string::npos) {
      last_said.replace(last_said.find("\""), 1, "");
    }
    SPEAK_DAMN_YOU(&last_said);
  }
}

void speak_buffer()
{
  std::string buffer;
  GtkTextIter *speak_start, *speak_end;

  speak_start = new GtkTextIter;
  speak_end = new GtkTextIter;

  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),speak_start,0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),speak_end,-1);
  
  buffer = gtk_text_iter_get_slice (speak_start,speak_end);

  if (buffer.length()>0) {
    while (buffer.find("\"") != std::string::npos) {
      buffer.replace(buffer.find("\""), 1, "");
    }
    SPEAK_DAMN_YOU(&buffer);
    last_said = buffer;
  }
}
#endif

void set_editbox_font(std::string FontName)
{
  if (FontName!="") {
    gtk_widget_modify_font (the_text_view,pango_font_description_from_string(FontName.c_str()));
  }
}
