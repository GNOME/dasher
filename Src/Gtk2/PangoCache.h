#ifndef __pangocache_h__
#define __pangocache_h__

#include <pango/pango.h>
#include <gtk/gtk.h>

#include <string>
#include <map>

class CPangoCache {
public:
  CPangoCache(const std::string & strFontName);
  void ChangeFont(const std::string & strFontName);

#if WITH_CAIRO
  PangoLayout *GetLayout(cairo_t *cr, std::string sDisplayText, int iSize);
#else
  PangoLayout *GetLayout(GtkWidget *pCanvas, std::string sDisplayText, int iSize);
#endif

private:
  std::map<std::string, PangoLayout *> oPangoCache;
  PangoFontDescription *font;
};

#endif
