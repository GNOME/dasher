GtkWidget *vbox, *toolbar;
GdkPixbuf *p;
GtkWidget *pw;
//Gtk2DasherEdit *dasher_text_view;
GtkWidget *text_view;
GtkWidget *speed_frame;
GtkObject *speed_slider;
GtkWidget *speed_hscale;
GtkWidget *text_scrolled_window;
GtkWidget *canvas_frame;
GtkWidget *ofilesel;
GtkWidget *ifilesel;
GtkWidget *afilesel;
GtkStyle *style;
//Gtk2DasherCanvas *dasher_canvas;
//Gtk2DasherPane *dasher_pane;
//CDasherInterface *interface;
GtkItemFactory *dasher_menu;
GtkAccelGroup *dasher_accel;
GtkWidget *dasher_menu_bar;
PangoLayout *the_pangolayout;
GtkWidget *the_canvas;

//Gtk2DoubleBuffer *the_buffer;

GdkPixmap *offscreen_buffer;
GdkPixmap *onscreen_buffer;

GtkWidget *the_text_view;  
GtkTextBuffer *the_text_buffer;
GtkClipboard *the_text_clipboard;

int flush_count;
