#ifndef __pangocache_h__
#define __pangocache_h__

#include <pango/pango.h>
#include <gtk/gtk.h>

#include <string>
#include <map>

class CPangoCache {
 public:
  CPangoCache();
  void ChangeFont( const std::string &strFontName );
  PangoLayout *GetLayout( GtkWidget *pCanvas, std::string sDisplayText, int iSize );

 private:
  std::map< std::string, PangoLayout * > oPangoCache;
  PangoFontDescription *font;
};

#endif
