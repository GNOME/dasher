#include "../Common/Common.h"

#include "PangoCache.h"
#include <iostream>

CPangoCache::CPangoCache(const std::string &strFontName) {
  font = pango_font_description_from_string(strFontName.c_str());
}

void CPangoCache::ChangeFont(const std::string &strFontName) {
  pango_font_description_free(font);
  font = pango_font_description_from_string(strFontName.c_str());

  // FIXME - probably need to free the pango layouts, but I can't find a function to do this.

  oPangoCache.clear();
}

#if WITH_CAIRO
PangoLayout *CPangoCache::GetLayout(cairo_t *cr, std::string sDisplayText, int iSize) {
#else
PangoLayout *CPangoCache::GetLayout(GtkWidget *pCanvas, std::string sDisplayText, int iSize) {
#endif

  // Calculate the name of the pango layout in the cache - this
  // includes the display text and the size.

   char buffer[128];             // FIXME - what if we exceed this?

   snprintf(buffer, 128, "%d_%s", iSize, sDisplayText.c_str());
//   //  std::stringstream sCacheName;
//   //sCacheName << iSize << "_" << sDisplayText;

   std::string sCacheName(buffer);

  // TODO: This is blatantly not going to work for unicode
   //  int iCacheIndex(iSize + 256 * sDisplayText[0]);

  // If we haven't got a cached pango layout for this string/size yet,
  // create a new one

  std::map<std::string, PangoLayout *>::iterator it(oPangoCache.find(sCacheName));

  if(it != oPangoCache.end())
    return it->second;
  else {

#if WITH_CAIRO
    PangoLayout *pNewPangoLayout(pango_cairo_create_layout(cr));
#else
    PangoLayout *pNewPangoLayout(gtk_widget_create_pango_layout(pCanvas, ""));
#endif

    pango_font_description_set_size(font, iSize * PANGO_SCALE);
    pango_layout_set_font_description(pNewPangoLayout, font);
    pango_layout_set_text(pNewPangoLayout, sDisplayText.c_str(), -1);

    oPangoCache[sCacheName] = pNewPangoLayout;

    return pNewPangoLayout;
  }
  return 0;
}
