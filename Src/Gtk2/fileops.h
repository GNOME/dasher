extern "C" void open_file(const char *myfilename);
extern "C" bool save_file_as(const char *filename, bool append);
gboolean unix_vfs_open_file(const char* filename, gchar** buffer, unsigned long long* size);
gboolean unix_vfs_save_file(const char* filename, gchar* buffer, unsigned long long length, bool append);

#ifdef GNOME_LIBS
#include <libgnomevfs/gnome-vfs.h>
void vfs_print_error(GnomeVFSResult *result, const char* myfilename);
gboolean gnome_vfs_open_file(const char* filename, gchar** buffer, unsigned long long* size);
gboolean gnome_vfs_save_file(const char* filename, gchar* buffer, unsigned long long length, bool append);
#endif
