#include "edit.h"
#include "dasher.h"
#include "accessibility.h"
#include "canvas.h"
#include <iostream>
#include <libwnck/libwnck.h>

extern int paused;
extern bool keyboardmodeon;
extern bool mouseposstart;
extern bool onedmode;
extern gboolean stdoutpipe;

#include <gdk/gdkx.h>

#ifdef GNOME_SPEECH
#include "speech.h"
#endif

GtkWidget *the_text_view;  
GtkTextBuffer *the_text_buffer;
GtkClipboard *the_text_clipboard;
GtkClipboard *the_primary_selection;
std::string last_said;
std::string say;
std::string pipetext;
std::string outputtext;
KeySym *origkeymap;
int modifiedkey=0;
int numcodes;

#ifdef GNOME_A11Y
AccessibleText* textbox=NULL;
AccessibleEditableText* edittextbox=NULL;
#endif

gunichar* wideoutput;

extern gint outputcharacters;
extern gboolean file_modified;
void initialise_edit()
{
  int min, max;
  Display *dpy = gdk_x11_get_default_xdisplay();
  the_text_clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  the_primary_selection = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  the_text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (the_text_view));
#ifdef X_HAVE_UTF8_STRING
  XDisplayKeycodes(dpy,&min,&max);
  origkeymap = XGetKeyboardMapping(dpy,min,max-min+1,&numcodes);
#endif
}

void cleanup_edit() {
#ifdef X_HAVE_UTF8_STRING
  // We want to set the keymap back to whatever it was before,
  // if that's possible
  int min, max;
  Display *dpy = gdk_x11_get_default_xdisplay();
  XDisplayKeycodes(dpy,&min,&max);
  XChangeKeyboardMapping(dpy,min,numcodes,origkeymap,(max-min));
#endif
}

void handle_cursor_move(GtkTextView *textview, GtkMovementStep arg1, gint arg2, gboolean arg3, gpointer data)
{
  // Let the core get new context and redraw it if the cursor is moved
  dasher_start();
  dasher_redraw();
}

void gtk2_edit_output_callback(symbol Symbol)
{
  std::string label;
  label = dasher_get_edit_text( Symbol );

#ifdef GNOME_SPEECH
  say+=label;
#endif

  if (stdoutpipe==TRUE) {
    pipetext+=label;
  }

  outputtext+=label;
  file_modified=TRUE;

  gtk_text_buffer_insert_at_cursor(the_text_buffer, label.c_str(), -1);
  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));

#ifdef GNOME_A11Y
  if (textbox!=NULL) {
    int position=AccessibleText_getCaretOffset(textbox);
    AccessibleEditableText_insertText(edittextbox,position,label.c_str(),label.size());
    position=position+g_utf8_strlen(label.c_str(),-1);
    AccessibleText_setCaretOffset(textbox,position);
    return;
  }
#endif

  if (keyboardmodeon==true) {
#ifdef X_HAVE_UTF8_STRING
    // FIXME
    // We should really check this at runtime rather than compile time
    // Ought to be possible by doing keysym_to_string on a Unicode keysym
    // and seeing if we get anything back
    Display *dpy = gdk_x11_get_default_xdisplay();
    int min, max;
    KeySym *keysym;
    KeyCode code;
    glong numoutput;
    
    if (label[0]=='\n') {
      // If it's a nreline, we want to mimic an enter press rather than a raw newline
      code = XKeysymToKeycode(dpy,XK_Return);
      if (code!=0) {
	XTestFakeKeyEvent(dpy, code, True, CurrentTime);
	XSync(dpy,true);
	XTestFakeKeyEvent(dpy, code, False, CurrentTime);
	XSync(dpy,true);
      }
    } else {
      wideoutput=g_utf8_to_ucs4(label.c_str(),-1,NULL,&numoutput,NULL);    
      for (int i=0; i<numoutput; i++) {
	modifiedkey=(modifiedkey+1)%10;
	// This gives us the magic X keysym
	wideoutput[i]=wideoutput[i] | 0x01000000;
	
	XDisplayKeycodes(dpy,&min,&max);
	keysym = XGetKeyboardMapping(dpy,min,max-min+1,&numcodes);
	keysym[(max-min-modifiedkey-1)*numcodes]=wideoutput[i];
	XChangeKeyboardMapping(dpy,min,numcodes,keysym,(max-min));
	XSync(dpy,true);
	XFree(keysym);
	// There's no way whatsoever that this could ever possibly
	// be guaranteed to work (ever), but it does.
	code=(max-modifiedkey-1);
	if (code!=0) {
	  XTestFakeKeyEvent(dpy, code, True, CurrentTime);
	  XSync(dpy,true);
	  XTestFakeKeyEvent(dpy, code, False, CurrentTime);
	  XSync(dpy,true);
	}
      }
      XSync(dpy,true);
      g_free(wideoutput);
    }
#else
#ifdef GNOME_A11Y
    // This would be the preferred way of doing it, but there's currently
    // only a small set of character sets that it works for, and you need
    // the keysym in your keymap anyway
    SPI_generateKeyboardEvent(0,(char*)label.c_str(),SPI_KEY_STRING);
#endif
#endif
  }
  outputcharacters++;
}

void write_to_file()
{
  // Add the text from the edit box to the user training file so we
  // can learn from them
  std::string filename=dasher_get_training_file();
  int fd=open(filename.c_str(),O_CREAT|O_WRONLY|O_APPEND,S_IRUSR|S_IWUSR);
  write(fd,outputtext.c_str(),outputtext.length());
  close(fd);
  outputtext="";
}

void gtk2_edit_outputcontrol_callback(void* pointer, int data)
{
  switch(data) {
#ifdef GNOME_A11Y
  case 1:
    if (pointer!=NULL) {
      Accessible *myfoo;
      myfoo=(Accessible *)pointer;
      AccessibleAction_doAction(Accessible_getAction(myfoo),0);
    }
    break;
  case 30:
    if (pointer!=NULL) {
      set_textbox((Accessible *)pointer);
    }
    break;
  case 31:
	  if (pointer!=NULL) {
	    //		  wnck_window_activate((WnckWindow *)pointer, gtk_get_current_event_time());
	  }
	  break;
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
    gtk2_edit_delete_callback(-1);
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

#ifdef GNOME_A11Y
    int position=AccessibleText_getCaretOffset(textbox);
    position++;
    AccessibleText_setCaretOffset(textbox,position);
#endif

}

void edit_move_back()
{
  GtkTextIter *pos = new GtkTextIter;

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,pos,gtk_text_buffer_get_insert(the_text_buffer));

  gtk_text_iter_backward_cursor_position(pos);

  gtk_text_buffer_place_cursor(the_text_buffer,pos);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));

#ifdef GNOME_A11Y
    int position=AccessibleText_getCaretOffset(textbox);
    position--;
    if (position<0) {
      position=0;
    }	     
    AccessibleText_setCaretOffset(textbox,position);
#endif
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

void gtk2_edit_delete_callback(symbol Symbol)
{

  if (Symbol==0) {
    return;
  }
  
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;
  int length,displaylength;
  if (Symbol!=-1) {
    length = dasher_get_edit_text(Symbol).length();
    displaylength = g_utf8_strlen(dasher_get_edit_text(Symbol).c_str(),-1);
  } else {
    displaylength=length=1;
  }

  gtk_text_buffer_get_iter_at_mark(the_text_buffer,end,gtk_text_buffer_get_insert(the_text_buffer));

  *start=*end;  

  gtk_text_iter_backward_chars(start, displaylength);

  gtk_text_buffer_delete(the_text_buffer,start,end);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));

#ifdef GNOME_SPEECH
  if(((signed int)say.length()-length)>=0) {
    say.resize(say.length()-length);
  }
#endif

  if (stdoutpipe==true) {
    if (((signed int)pipetext.length()-length)>=0) {
      pipetext.resize(pipetext.length()-length);
    }
  }

  if(((signed int)outputtext.length()-length)>=0) {
    outputtext.resize(outputtext.length()-length);
  }

#ifdef GNOME_A11Y
  if (textbox!=NULL) {
    int endpos=AccessibleText_getCaretOffset(textbox);
    if (endpos!=0) {
      int startpos=endpos-length;      
      AccessibleEditableText_deleteText(edittextbox,startpos,endpos);
    }
    outputcharacters--;

    delete start;
    delete end;
    return;
  }
#endif

  if (keyboardmodeon==true) {
#ifdef X_HAVE_UTF8_STRING
    Display *dpy;
    dpy = gdk_x11_get_default_xdisplay();
    KeyCode code;
    code = XKeysymToKeycode(dpy,XK_BackSpace);
    for (int i=0; i<displaylength; i++) {
      XTestFakeKeyEvent(dpy, code, True, 0);
      XTestFakeKeyEvent(dpy, code, False, 0);
    }
    XFlush(dpy);
#else
#ifdef GNOME_A11Y
    for (int i=0; i<displaylength; i++) {
      SPI_generateKeyboardEvent(XK_BackSpace,NULL,SPI_KEY_SYM);
    }
#endif
#endif
  }
  outputcharacters--;

  delete start;
  delete end;
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

#ifdef GNOME_A11Y
  if (textbox!=NULL) {
    int startpos=AccessibleText_getCaretOffset(textbox);
    int endpos=startpos+1;      
    AccessibleEditableText_deleteText(edittextbox,startpos,endpos);
  }
#endif

  delete start;
  delete end;
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

  gtk_text_iter_backward_line(start);

  gtk_text_buffer_delete(the_text_buffer,start,end);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(the_text_view),gtk_text_buffer_get_insert(the_text_buffer));
}

void gtk2_clipboard_callback( clipboard_action act )
{
  GtkTextIter *start = new GtkTextIter;
  GtkTextIter *end = new GtkTextIter;
  
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),start,0);
  gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(the_text_buffer),end,-1);
  
  gchar *the_text=gtk_text_buffer_get_text(the_text_buffer,start,end,TRUE);

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
      gtk_clipboard_set_text(the_text_clipboard,the_text,strlen(the_text));
      gtk_clipboard_set_text(the_primary_selection,the_text,strlen(the_text));

      break;
    case CLIPBOARD_SELECTALL:
      select_all();
      break;
    case CLIPBOARD_CLEAR:
      gtk_text_buffer_set_text (the_text_buffer,"",0);
      break;
    }
  g_free(the_text);
  
  delete start;
  delete end;  
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

  delete start;
  delete end;
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

void gtk2_get_new_context_callback( std::string &str, int max )
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

  delete start;
  delete end;  
}

void outputpipe()
{
  printf("%s",pipetext.c_str());
  fflush(stdout);
  pipetext="";
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
  // Repeat whatever it was that we last spoke
  if (last_said.length()>0) {
    // Festival seems unhappy about quotes
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

#ifdef GNOME_A11Y
void set_textbox(Accessible* newtextbox) {
  AccessibleText_unref(textbox);
  AccessibleEditableText_unref(textbox);
  if (newtextbox==NULL) {
    textbox=NULL;
    edittextbox=NULL;
  } else {
    AccessibleComponent *component;
    component=Accessible_getComponent(newtextbox);
    //    AccessibleComponent_grabFocus(component);
    textbox=Accessible_getText(newtextbox);
    edittextbox=Accessible_getEditableText(newtextbox);
  }
}
#endif

gboolean a11y_text_entry() {
#ifdef GNOME_A11Y
  if (textbox!=NULL) {
    return TRUE;
  }
#endif
  return FALSE;
}
