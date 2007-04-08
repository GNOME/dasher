// DelayedDraw.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __View_DelayedDraw_h_
#define __View_DelayedDraw_h_

#include "../DasherTypes.h"

namespace Dasher {
  class CDasherScreen;
}

class Dasher::CDasherScreen;

namespace Dasher {
  /// \ingroup View
  /// @{

  /// Class for delayed draw events
  /// Useful for rendering the text on top of the nodes without using 
  /// 2 buffers or 2 passes of the model
  class CDelayedDraw {
  public:
    //! DelayDraw string String of size Size positioned at x1 and y1
    void DelayDrawText(const std::string & String, screenint x1, screenint y1, int Size);

    //! Draw all DelayDraw events
    void Draw(CDasherScreen * screen);

  private:

      class CTextSymbol {
    public:
      CTextSymbol(symbol Character, screenint x, screenint y, int iSize)
      : m_Character(Character), m_x(x), m_y(y), m_iSize(iSize) {
      } symbol m_Character;
      screenint m_x;
      screenint m_y;
      int m_iSize;
    };

    class CTextString {
    public:
      CTextString(const std::string & str, screenint x, screenint y, int iSize)
      : m_String(str), m_x(x), m_y(y), m_iSize(iSize) {
      } std::string m_String;
      screenint m_x;
      screenint m_y;
      int m_iSize;
    };

    std::vector < CTextSymbol > m_DrawTextSymbol;
    std::vector < CTextString > m_DrawTextString;
  };
  /// @}
};

#endif
