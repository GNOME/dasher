// DelayedDraw.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __View_DelayedDraw_h_
#define __View_DelayedDraw_h_

#include "../DasherScreen.h"
#include "../DasherTypes.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////

// Class for delayed draw events
// Useful for rendering the text on top of the nodes without using 
// 2 buffers or 2 passes of the model

namespace Dasher {

  class CDelayedDraw {
  public:

    //! DelayDraw character Character of size Size positioned at x1 and y1
    //void DelayDrawText(symbol Character, screenint x1, screenint y1, int Size);

    //! DelayDraw string String of size Size positioned at x1 and y1
    void DelayDrawText(const std::string & String, screenint x1, screenint y1, int Size);

    //! Draw all DelayDraw events
    void Draw(CDasherScreen & screen);

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

/////////////////////////////////////////////////////////////////////////////
/*
inline void CDelayedDraw::DelayDrawText(symbol Character, screenint x1, screenint y1, int Size)
{
	m_DrawTextSymbol.push_back( CTextSymbol( Character,x1,y1,Size) );
}
*/

/////////////////////////////////////////////////////////////////////////////

  inline void CDelayedDraw::DelayDrawText(const std::string & str, screenint x1, screenint y1, int Size) {
    m_DrawTextString.push_back(CTextString(str, x1, y1, Size));
  }

/////////////////////////////////////////////////////////////////////////////

//! Draw all DelayDraw events
  inline void CDelayedDraw::Draw(CDasherScreen & screen) {
/*	int iSize=m_DrawTextSymbol.size();

	for (int i=0; i<iSize ; i++)
	{
		CTextSymbol& draw = m_DrawTextSymbol[i];
		screen.DrawText( draw.m_Character, draw.m_x, draw.m_y, draw.m_iSize);
	}

	m_DrawTextSymbol.clear();*/

    int iSize = m_DrawTextString.size();

    std::cout << "Delay draw: " << iSize << " objects" << std::endl;

    for(int i = 0; i < iSize; i++) {
      CTextString & draw = m_DrawTextString[i];
      screen.DrawString(draw.m_String, draw.m_x, draw.m_y, draw.m_iSize);
    }
    m_DrawTextString.clear();

  }

/////////////////////////////////////////////////////////////////////////////

};

/////////////////////////////////////////////////////////////////////////////
#endif
