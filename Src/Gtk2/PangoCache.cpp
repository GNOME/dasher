#include "PangoCache.h"

CPangoCache::CPangoCache() {
  font = pango_font_description_new();
  pango_font_description_set_family( font,"Serif");
}

PangoLayout *CPangoCache::GetLayout( GtkWidget *pCanvas, std::string sDisplayText, int iSize ) {
  
  // Calculate the name of the pango layout in the cache - this
  // includes the display text and the size.

  char buffer[128]; // FIXME - what if we exceed this?

  snprintf( buffer, 128, "%d_%s", iSize, sDisplayText.c_str() );

  //  std::stringstream sCacheName;
  //sCacheName << iSize << "_" << sDisplayText;

  std::string sCacheName( buffer );

  // If we haven't got a cached pango layout for this string/size yet,
  // create a new one

  std::map< std::string, PangoLayout * >::iterator it( oPangoCache.find( sCacheName ) );

  if( it != oPangoCache.end() )
    return it->second;
  else {
    PangoLayout *pNewPangoLayout( gtk_widget_create_pango_layout (pCanvas, "") );

    pango_font_description_set_size( font, iSize*PANGO_SCALE );
    pango_layout_set_font_description( pNewPangoLayout,font );
    pango_layout_set_text( pNewPangoLayout,sDisplayText.c_str(),-1 );

    oPangoCache[ sCacheName ] = pNewPangoLayout;

    return pNewPangoLayout;
  }
}
