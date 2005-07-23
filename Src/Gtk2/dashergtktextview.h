/* GTK - The GIMP Toolkit
 * gtktextview.h Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __DASHER_GTK_TEXT_VIEW_H__
#define __DASHER_GTK_TEXT_VIEW_H__

#include <gtk/gtkcontainer.h>
#include <gtk/gtkimcontext.h>
#include <gtk/gtktextbuffer.h>
#include <gtk/gtkmenu.h>

G_BEGIN_DECLS
#define DASHER_GTK_TYPE_TEXT_VIEW             (dasher_gtk_text_view_get_type ())
#define DASHER_GTK_TEXT_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), DASHER_GTK_TYPE_TEXT_VIEW, DasherGtkTextView))
#define DASHER_GTK_TEXT_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), DASHER_GTK_TYPE_TEXT_VIEW, DasherGtkTextViewClass))
#define DASHER_GTK_IS_TEXT_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), DASHER_GTK_TYPE_TEXT_VIEW))
#define DASHER_GTK_IS_TEXT_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), DASHER_GTK_TYPE_TEXT_VIEW))
#define DASHER_GTK_TEXT_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), DASHER_GTK_TYPE_TEXT_VIEW, DasherGtkTextViewClass))
  typedef enum {
  DASHER_GTK_TEXT_WINDOW_PRIVATE,
  DASHER_GTK_TEXT_WINDOW_WIDGET,
  DASHER_GTK_TEXT_WINDOW_TEXT,
  DASHER_GTK_TEXT_WINDOW_LEFT,
  DASHER_GTK_TEXT_WINDOW_RIGHT,
  DASHER_GTK_TEXT_WINDOW_TOP,
  DASHER_GTK_TEXT_WINDOW_BOTTOM
} DasherGtkTextWindowType;

#define DASHER_GTK_TEXT_VIEW_PRIORITY_VALIDATE (GDK_PRIORITY_REDRAW + 5)

typedef struct _DasherGtkTextView DasherGtkTextView;
typedef struct _DasherGtkTextViewClass DasherGtkTextViewClass;

/* Internal private types. */
typedef struct _GtkTextWindow DasherGtkTextWindow;
typedef struct _GtkTextPendingScroll GtkTextPendingScroll;

struct _DasherGtkTextView {
  GtkContainer parent_instance;

  struct _GtkTextLayout *layout;
  GtkTextBuffer *buffer;

  guint selection_drag_handler;
  guint scroll_timeout;

  /* Default style settings */
  gint pixels_above_lines;
  gint pixels_below_lines;
  gint pixels_inside_wrap;
  GtkWrapMode wrap_mode;
  GtkJustification justify;
  gint left_margin;
  gint right_margin;
  gint indent;
  PangoTabArray *tabs;
  guint editable:1;

  guint overwrite_mode:1;
  guint cursor_visible:1;

  /* if we have reset the IM since the last character entered */
  guint need_im_reset:1;

  guint accepts_tab:1;

  /* this flag is no longer used */
  guint reserved:1;

  /* debug flag - means that we've validated onscreen since the
   * last "invalidate" signal from the layout
   */
  guint onscreen_validated:1;

  guint mouse_cursor_obscured:1;

  DasherGtkTextWindow *text_window;
  DasherGtkTextWindow *left_window;
  DasherGtkTextWindow *right_window;
  DasherGtkTextWindow *top_window;
  DasherGtkTextWindow *bottom_window;

  GtkAdjustment *hadjustment;
  GtkAdjustment *vadjustment;

  gint xoffset;                 /* Offsets between widget coordinates and buffer coordinates */
  gint yoffset;
  gint width;                   /* Width and height of the buffer */
  gint height;

  /* The virtual cursor position is normally the same as the
   * actual (strong) cursor position, except in two circumstances:
   *
   * a) When the cursor is moved vertically with the keyboard
   * b) When the text view is scrolled with the keyboard
   *
   * In case a), virtual_cursor_x is preserved, but not virtual_cursor_y
   * In case b), both virtual_cursor_x and virtual_cursor_y are preserved.
   */
  gint virtual_cursor_x;        /* -1 means use actual cursor position */
  gint virtual_cursor_y;        /* -1 means use actual cursor position */

  GtkTextMark *first_para_mark; /* Mark at the beginning of the first onscreen paragraph */
  gint first_para_pixels;       /* Offset of top of screen in the first onscreen paragraph */

  GtkTextMark *dnd_mark;
  guint blink_timeout;

  guint first_validate_idle;    /* Idle to revalidate onscreen portion, runs before resize */
  guint incremental_validate_idle;      /* Idle to revalidate offscreen portions, runs after redraw */

  GtkIMContext *im_context;
  GtkWidget *popup_menu;

  gint drag_start_x;
  gint drag_start_y;

  GSList *children;

  GtkTextPendingScroll *pending_scroll;

  gint pending_place_cursor_button;
};

struct _DasherGtkTextViewClass {
  GtkContainerClass parent_class;

  void (*set_scroll_adjustments) (DasherGtkTextView * text_view, GtkAdjustment * hadjustment, GtkAdjustment * vadjustment);

  void (*populate_popup) (DasherGtkTextView * text_view, GtkMenu * menu);

  /* These are all RUN_ACTION signals for keybindings */

  /* move insertion point */
  void (*move_cursor) (DasherGtkTextView * text_view, GtkMovementStep step, gint count, gboolean extend_selection);

  /* FIXME should be deprecated in favor of adding GTK_MOVEMENT_HORIZONTAL_PAGES
   * or something in GTK 2.2, was put in to avoid adding enum values during
   * the freeze.
   */
  void (*page_horizontally) (DasherGtkTextView * text_view, gint count, gboolean extend_selection);

  /* move the "anchor" (what Emacs calls the mark) to the cursor position */
  void (*set_anchor) (DasherGtkTextView * text_view);

  /* Edits */
  void (*insert_at_cursor) (DasherGtkTextView * text_view, const gchar * str);
  void (*delete_from_cursor) (DasherGtkTextView * text_view, GtkDeleteType type, gint count);
  void (*backspace) (DasherGtkTextView * text_view);

  /* cut copy paste */
  void (*cut_clipboard) (DasherGtkTextView * text_view);
  void (*copy_clipboard) (DasherGtkTextView * text_view);
  void (*paste_clipboard) (DasherGtkTextView * text_view);
  /* overwrite */
  void (*toggle_overwrite) (DasherGtkTextView * text_view);

  /* propagates to GtkWindow move_focus */
  void (*move_focus) (DasherGtkTextView * text_view, GtkDirectionType direction);

  /* Padding for future expansion */
  void (*_gtk_reserved1) (void);
  void (*_gtk_reserved2) (void);
  void (*_gtk_reserved3) (void);
  void (*_gtk_reserved4) (void);
  void (*_gtk_reserved5) (void);
  void (*_gtk_reserved6) (void);
  void (*_gtk_reserved7) (void);
};

GType dasher_gtk_text_view_get_type(void) G_GNUC_CONST;
     GtkWidget *dasher_gtk_text_view_new(void);
     GtkWidget *dasher_gtk_text_view_new_with_buffer(GtkTextBuffer *buffer);
     void dasher_gtk_text_view_set_buffer(DasherGtkTextView *text_view, GtkTextBuffer *buffer);
     GtkTextBuffer *dasher_gtk_text_view_get_buffer(DasherGtkTextView *text_view);
     gboolean dasher_gtk_text_view_scroll_to_iter(DasherGtkTextView *text_view, GtkTextIter *iter, gdouble within_margin, gboolean use_align, gdouble xalign, gdouble yalign);
     void dasher_gtk_text_view_scroll_to_mark(DasherGtkTextView *text_view, GtkTextMark *mark, gdouble within_margin, gboolean use_align, gdouble xalign, gdouble yalign);
     void dasher_gtk_text_view_scroll_mark_onscreen(DasherGtkTextView *text_view, GtkTextMark *mark);
     gboolean dasher_gtk_text_view_move_mark_onscreen(DasherGtkTextView *text_view, GtkTextMark *mark);
     gboolean dasher_gtk_text_view_place_cursor_onscreen(DasherGtkTextView *text_view);

     void dasher_gtk_text_view_get_visible_rect(DasherGtkTextView *text_view, GdkRectangle *visible_rect);
     void dasher_gtk_text_view_set_cursor_visible(DasherGtkTextView *text_view, gboolean setting);
     gboolean dasher_gtk_text_view_get_cursor_visible(DasherGtkTextView *text_view);

     void dasher_gtk_text_view_get_iter_location(DasherGtkTextView *text_view, const GtkTextIter *iter, GdkRectangle *location);
     void dasher_gtk_text_view_get_iter_at_location(DasherGtkTextView *text_view, GtkTextIter *iter, gint x, gint y);
     void dasher_gtk_text_view_get_iter_at_position(DasherGtkTextView *text_view, GtkTextIter *iter, gint *trailing, gint x, gint y);
     void dasher_gtk_text_view_get_line_yrange(DasherGtkTextView *text_view, const GtkTextIter *iter, gint *y, gint *height);

     void dasher_gtk_text_view_get_line_at_y(DasherGtkTextView *text_view, GtkTextIter *target_iter, gint y, gint *line_top);

     void dasher_gtk_text_view_buffer_to_window_coords(DasherGtkTextView *text_view, DasherGtkTextWindowType win, gint buffer_x, gint buffer_y, gint *window_x, gint *window_y);
     void dasher_gtk_text_view_window_to_buffer_coords(DasherGtkTextView *text_view, DasherGtkTextWindowType win, gint window_x, gint window_y, gint *buffer_x, gint *buffer_y);

     GdkWindow *dasher_gtk_text_view_get_window(DasherGtkTextView *text_view, DasherGtkTextWindowType win);
     DasherGtkTextWindowType dasher_gtk_text_view_get_window_type(DasherGtkTextView *text_view, GdkWindow *window);

     void dasher_gtk_text_view_set_border_window_size(DasherGtkTextView *text_view, DasherGtkTextWindowType type, gint size);
     gint dasher_gtk_text_view_get_border_window_size(DasherGtkTextView *text_view, DasherGtkTextWindowType type);

     gboolean dasher_gtk_text_view_forward_display_line(DasherGtkTextView *text_view, GtkTextIter *iter);
     gboolean dasher_gtk_text_view_backward_display_line(DasherGtkTextView *text_view, GtkTextIter *iter);
     gboolean dasher_gtk_text_view_forward_display_line_end(DasherGtkTextView *text_view, GtkTextIter *iter);
     gboolean dasher_gtk_text_view_backward_display_line_start(DasherGtkTextView *text_view, GtkTextIter *iter);
     gboolean dasher_gtk_text_view_starts_display_line(DasherGtkTextView *text_view, const GtkTextIter *iter);
     gboolean dasher_gtk_text_view_move_visually(DasherGtkTextView *text_view, GtkTextIter *iter, gint count);

/* Adding child widgets */
     void dasher_gtk_text_view_add_child_at_anchor(DasherGtkTextView *text_view, GtkWidget *child, GtkTextChildAnchor *anchor);

     void dasher_gtk_text_view_add_child_in_window(DasherGtkTextView *text_view, GtkWidget *child, DasherGtkTextWindowType which_window,
                                                   /* window coordinates */
                                                   gint xpos, gint ypos);

     void dasher_gtk_text_view_move_child(DasherGtkTextView *text_view, GtkWidget *child,
                                          /* window coordinates */
                                          gint xpos, gint ypos);

/* Default style settings (fallbacks if no tag affects the property) */

     void dasher_gtk_text_view_set_wrap_mode(DasherGtkTextView *text_view, GtkWrapMode wrap_mode);
     GtkWrapMode dasher_gtk_text_view_get_wrap_mode(DasherGtkTextView *text_view);
     void dasher_gtk_text_view_set_editable(DasherGtkTextView *text_view, gboolean setting);
     gboolean dasher_gtk_text_view_get_editable(DasherGtkTextView *text_view);
     void dasher_gtk_text_view_set_overwrite(DasherGtkTextView *text_view, gboolean overwrite);
     gboolean dasher_gtk_text_view_get_overwrite(DasherGtkTextView *text_view);
     void dasher_gtk_text_view_set_accepts_tab(DasherGtkTextView *text_view, gboolean accepts_tab);
     gboolean dasher_gtk_text_view_get_accepts_tab(DasherGtkTextView *text_view);
     void dasher_gtk_text_view_set_pixels_above_lines(DasherGtkTextView *text_view, gint pixels_above_lines);
     gint dasher_gtk_text_view_get_pixels_above_lines(DasherGtkTextView *text_view);
     void dasher_gtk_text_view_set_pixels_below_lines(DasherGtkTextView *text_view, gint pixels_below_lines);
     gint dasher_gtk_text_view_get_pixels_below_lines(DasherGtkTextView *text_view);
     void dasher_gtk_text_view_set_pixels_inside_wrap(DasherGtkTextView *text_view, gint pixels_inside_wrap);
     gint dasher_gtk_text_view_get_pixels_inside_wrap(DasherGtkTextView *text_view);
     void dasher_gtk_text_view_set_justification(DasherGtkTextView *text_view, GtkJustification justification);
     GtkJustification dasher_gtk_text_view_get_justification(DasherGtkTextView *text_view);
     void dasher_gtk_text_view_set_left_margin(DasherGtkTextView *text_view, gint left_margin);
     gint dasher_gtk_text_view_get_left_margin(DasherGtkTextView *text_view);
     void dasher_gtk_text_view_set_right_margin(DasherGtkTextView *text_view, gint right_margin);
     gint dasher_gtk_text_view_get_right_margin(DasherGtkTextView *text_view);
     void dasher_gtk_text_view_set_indent(DasherGtkTextView *text_view, gint indent);
     gint dasher_gtk_text_view_get_indent(DasherGtkTextView *text_view);
     void dasher_gtk_text_view_set_tabs(DasherGtkTextView *text_view, PangoTabArray *tabs);
     PangoTabArray *dasher_gtk_text_view_get_tabs(DasherGtkTextView *text_view);

/* note that the return value of this changes with the theme */
     GtkTextAttributes *dasher_gtk_text_view_get_default_attributes(DasherGtkTextView *text_view);

G_END_DECLS
#endif /* __DASHER_GTK_TEXT_VIEW_H__ */
