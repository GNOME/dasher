
#include "../Common/Common.h"

#include "View/DelayedDraw.h"
#include "DasherScreen.h"
#include "DasherTypes.h"

#include <iostream>

using namespace Dasher;

void CDelayedDraw::DelayDrawText(const std::string & str, screenint x1, screenint y1, int Size) {
  m_DrawTextString.push_back(CTextString(str, x1, y1, Size));
}

void CDelayedDraw::Draw(CDasherScreen * screen) {
  int iSize = m_DrawTextString.size();

  for(int i = 0; i < iSize; i++) {
    CTextString & draw = m_DrawTextString[i];
    screen->DrawString(draw.m_String, draw.m_x, draw.m_y, draw.m_iSize);
  }
  m_DrawTextString.clear();

}
