// ScreenGameModule.cpp
// Dasher

#include "ScreenGameModule.h"

using namespace Dasher;

CScreenGameModule::CScreenGameModule(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pIntf, CDasherView *pView, CDasherModel *pModel)
: CGameModule(pCreateFrom, pIntf, pView, pModel), m_pLabEntered(NULL), m_pLabTarget(NULL), m_pLabWrong(NULL) {
}

void CScreenGameModule::ChunkGenerated() {
  delete m_pLabEntered; m_pLabEntered = NULL;
  delete m_pLabTarget; m_pLabTarget = NULL;
  delete m_pLabWrong; m_pLabWrong = NULL;
  m_iFirstSym = m_iLastSym = 0;
}

void CScreenGameModule::HandleEvent(const CEditEvent *pEvt) {
  const int iPrev(lastCorrectSym());
  CGameModule::HandleEvent(pEvt);
  if (iPrev==lastCorrectSym()) {
    if (m_pLabWrong) DASHER_ASSERT(m_pLabWrong->m_strText != m_strWrong);
    delete m_pLabWrong;
    m_pLabWrong = NULL;
  } else {
    DASHER_ASSERT(!m_pLabWrong);
    delete m_pLabEntered; m_pLabEntered = NULL;
    delete m_pLabTarget; m_pLabTarget = NULL;
    if (iPrev>lastCorrectSym()) {
      //erased correct sym!
      const string erased(m_pAlph->GetText(targetSyms()[iPrev]));
      DASHER_ASSERT(m_strEntered.substr(m_strEntered.length()-erased.length())==erased);
      m_strTarget = erased + m_strTarget;
      m_strEntered = m_strEntered.substr(0, m_strEntered.length()-erased.length());
    } else {
      const string written(m_pAlph->GetText(targetSyms()[lastCorrectSym()]));
      DASHER_ASSERT(m_strTarget.substr(0,written.length())==written);
      m_strTarget = m_strTarget.substr(written.length());
      m_strEntered += written;
    }
  }
}

void CScreenGameModule::DrawText(CDasherView *pView) {
  const unsigned int uFontSize(GetLongParameter(LP_MESSAGE_FONTSIZE));
  //Assume left-to-right orientation...too many issues for other orientations!
  CDasherScreen *pScreen(pView->Screen());
  screenint maxX,tempy; //maxX = target width of game display...
  //i.e. make <margin width> left of y axis (itself <margin width> to left of screen edge)
  pView->Dasher2Screen(GetLongParameter(LP_MARGIN_WIDTH), 0, maxX, tempy);

  if (!m_pLabTarget || !m_pLabEntered) {
    DASHER_ASSERT(!m_pLabTarget && !m_pLabEntered);
    //Make new labels, aiming for 1/3 and 2/3 of the available width.
    //Note we ask for wrapped labels, because on Mac+iPhone OpenGL screens this gives better
    // quality text rendering; none should ever reach be wide enough for wrapping to actually occur.
    //Yes, this is a slight hack ;), and indeed, the whole business here raises the question of
    // whether the screen Label API is right yet. Possibilities
    //  (a) Allow mutating Labels, i.e. change the string. On Linux, might be more efficient to
    //      just change the text of a pangolayout, rather than recreate all the properties. On
    //      MacOS, would probably have to lazily (re)generate the texture only when actually rendered.
    //  (b) Add back in TextSize(string), with the understanding that Screen's would _not_ cache
    //      the result. On linux this would be no more efficient at all (wd still have to create
    //      pangolayout), but would be simple+performant on MacOS/iPhone...
    //Of course, this may be a bit academic, if platforms have their own game displays...
    for (bool bIncreased=false;;) {
      m_pLabEntered = pScreen->MakeLabel(m_strEntered, uFontSize);
      screenint w = pScreen->TextSize(m_pLabEntered, uFontSize).first;
      if (w < maxX/3 && m_iFirstSym) {
        m_strEntered = m_pAlph->GetText(targetSyms()[--m_iFirstSym]) + m_strEntered;
        bIncreased = true;
      } else if (w>2*maxX/3 && m_iFirstSym < targetSyms().size()-1 && !bIncreased)
        m_strEntered = m_strEntered.substr(m_pAlph->GetText(targetSyms()[m_iFirstSym++]).length());
      else break;
      //m_strEntered changed
      delete m_pLabEntered;
    }
    for (bool bIncreased=false;;) {
      m_pLabTarget = pScreen->MakeLabel(m_strTarget, uFontSize);
      screenint w = pScreen->TextSize(m_pLabTarget, uFontSize).first;
      if (w < 2*maxX/3 && m_iLastSym < targetSyms().size()-1) {
        m_strTarget += m_pAlph->GetText(targetSyms()[m_iLastSym++]);
        bIncreased = true;
      } else if (w > maxX && m_iLastSym && !bIncreased)
        m_strTarget = m_strTarget.substr(0, m_strTarget.length() - m_pAlph->GetText(targetSyms()[--m_iLastSym]).length());
      else break;
      //m_strTarget changed
      delete m_pLabTarget;
    }
  }
  if (!m_strWrong.empty() && !m_pLabWrong) m_pLabWrong = pScreen->MakeLabel(m_strWrong, uFontSize);
  const screenint x(maxX/3); //RHS of entered label
  const pair<screenint,screenint> entDims(pScreen->TextSize(m_pLabEntered, uFontSize));

  screenint targetStart(x); //x coord at which to render target label
  //total area we will render text over, i.e. need to blank out:
  pair<screenint,screenint> rectDims(pScreen->TextSize(m_pLabTarget, uFontSize));

  rectDims.second=max(rectDims.second,entDims.second);

  if (m_pLabWrong) {
    const pair<screenint,screenint> wrongDims(pScreen->TextSize(m_pLabWrong, uFontSize));
    targetStart+=wrongDims.first;
    rectDims.second=max(rectDims.second, wrongDims.second);
  }
  rectDims.first+=targetStart;

  //ok. that's the layout done
  pScreen->DrawRectangle(0, 0, rectDims.first, rectDims.second, 0, 4, 1);
  pScreen->DrawString(m_pLabEntered, x-entDims.first, 0, uFontSize, 7);//root node green...?
  if (m_pLabWrong) {
    pScreen->DrawString(m_pLabWrong, x, 0, uFontSize, 1); //mouse-line red
    CDasherScreen::point p[2];
    p[0].x=x; p[1].x=targetStart;
    p[0].y = p[1].y = rectDims.second/2;
    pScreen->Polyline(p, 2, 2, 1); //mouse-line red
  }
  pScreen->DrawString(m_pLabTarget, targetStart, 0, uFontSize, 4); //text black

}
