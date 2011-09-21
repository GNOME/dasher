#include "GameModule.h"
#include "GameStatistics.h"
#include <sstream>

using namespace Dasher;

CGameModule::CGameModule(CSettingsUser *pCreateFrom, Dasher::CDasherInterfaceBase *pInterface, CDasherView *pView, CDasherModel *pModel) 
: CSettingsUser(pCreateFrom), TransientObserver<const Dasher::CEditEvent *>(pInterface), TransientObserver<CGameNodeDrawEvent*>(pView),
TransientObserver<CDasherNode*>(pModel), TransientObserver<CDasherView*>(pView),
m_pInterface(pInterface), m_iLastSym(-1),
m_y1(std::numeric_limits<myint>::min()), m_y2(std::numeric_limits<myint>::max()),
m_ulTotalTime(0), m_dTotalNats(0.0), m_uiTotalSyms(0),
m_iCrosshairColor(135), m_iFontSize(36)
{}

CGameModule::~CGameModule()  {
  DASHER_ASSERT(!GetBoolParameter(BP_GAME_MODE));
  if (m_ulTotalTime) {
    //TODO make this a running commentary?
    ostringstream summary;
    summary << "Total time " << m_ulTotalTime; 
    summary << " nats " << m_dTotalNats << "=" << (m_dTotalNats*1000.0/m_ulTotalTime) << "/sec";
    summary << " chars " << m_uiTotalSyms << "=" << (m_uiTotalSyms/m_ulTotalTime) << "/sec";
    m_pInterface->Message(summary.str(),true);
  }
  m_pInterface->ClearAllContext();
}

//Node populated...
void CGameModule::HandleEvent(CDasherNode *pNode) {
  if (pNode->GetFlag(NF_GAME) //if on game path, look for next/child node on path...
      && pNode->offset()+1 < m_vTargetSymbols.size())
    pNode->GameSearchChildren(m_vTargetSymbols[pNode->offset()+1]);
}

void CGameModule::HandleEvent(const CEditEvent *evt) {
  if (!m_pAlph) return; //Game Mode currently not running
  const int iOffset(evt->m_pNode->offset());
  switch(evt->m_iEditType) {
      // Added a new character (Stepped one node forward)
    case CEditEvent::EDIT_OUTPUT:
      if (iOffset == m_iLastSym+1
          && iOffset < m_vTargetSymbols.size()) {
        DASHER_ASSERT(m_strWrong == "");
        if (evt->m_sText == m_pAlph->GetText(m_vTargetSymbols[iOffset])) {
          // User has entered correct text...
          ++m_iLastSym;
        } else m_strWrong = evt->m_sText;
      } else {
        DASHER_ASSERT(iOffset >= m_iLastSym+1);
        m_strWrong+=evt->m_sText;
      }
      break;
      // Removed a character (Stepped one node back)
    case CEditEvent::EDIT_DELETE:
      if (iOffset == m_iLastSym) {
        //seems they've just deleted the last _correct_ character they'd entered...
        DASHER_ASSERT(evt->m_sText == m_pAlph->GetText(m_vTargetSymbols[m_iLastSym]));
        --m_iLastSym;
      } else {
        //just deleted previously-entered wrong text - hopefully they're heading in the right direction!
        DASHER_ASSERT(m_strWrong.length() >= evt->m_sText.length());
        DASHER_ASSERT(m_strWrong.substr(m_strWrong.length() - evt->m_sText.length()) == evt->m_sText);
        m_strWrong = m_strWrong.substr(0,m_strWrong.length() - evt->m_sText.length());
      }
      break;
    default:
      break;
  }
}

void CGameModule::HandleEvent(CGameNodeDrawEvent *gmd) {
  //game nodes form a single chain, i.e. are strictly nested.
  // we want the coordinates of the smallest (innermost) one about which we are told
  m_y1 = max(m_y1, gmd->m_y1);
  m_y2 = min(m_y2, gmd->m_y2);
}

void CGameModule::HandleEvent(CDasherView *pView) {
  if (pView!=TransientObserver<CGameNodeDrawEvent*>::m_pEventHandler) {
    TransientObserver<CGameNodeDrawEvent*>::m_pEventHandler->Unregister(this);
    (TransientObserver<CGameNodeDrawEvent*>::m_pEventHandler = pView)->Register(this);
  }
}

void CGameModule::SetWordGenerator(const CAlphInfo *pAlph, CWordGeneratorBase *pWordGenerator) {
        m_pAlph = pAlph;
	m_pWordGenerator = pWordGenerator;
	if (!GenerateChunk()) {
    m_pInterface->Message("Game mode sentences file empty!",true);
    //this'll delete the 'this' pointer, so we'd better not do anything else afterwards!...
    SetBoolParameter(BP_GAME_MODE, false);
  }
}

void CGameModule::StartWriting(unsigned long lTime) {
  if (!m_ulSentenceStartTime) {
    m_ulSentenceStartTime = lTime;
    m_dSentenceStartNats = numeric_limits<double>::max();
  }
}

void CGameModule::DecorateView(unsigned long lTime, CDasherView *pView, CDasherModel *pModel) {

  if (m_dSentenceStartNats == numeric_limits<double>::max())
    m_dSentenceStartNats = pModel->GetNats();

  m_vTargetY.push_back((m_y1+m_y2)/2);
  
  //draw a line along the y axis
  myint x[2], y[2];
  x[0] = x[1] = -100;
  
  if (m_y1 > CDasherModel::MAX_Y) {
    //off the top! draw an arrow pointing up...
    y[1] = CDasherModel::MAX_Y;
    y[0] = y[1] - 400;
    pView->DasherPolyarrow(x, y, 2, 2, m_iCrosshairColor, 0.1);
  } else if (m_y2 < 0) {
    //off the bottom! draw arrow pointing down...
    y[1] = 0;
    y[0] = 400;
    pView->DasherPolyarrow(x, y, 2, 2, m_iCrosshairColor, 0.1);
  } else {
    //draw line parallel to that region of y-axis
    y[0] = m_y1; y[1] = m_y2;
    pView->DasherPolyline(x, y, 2, 2, m_iCrosshairColor);
    //and a horizontal arrow pointing to the midpoint
    x[0] = -400;
    y[0] = y[1] = (m_y1+m_y2)/2;
    pView->DasherPolyarrow(x, y, 2, 2, m_iCrosshairColor, 0.1);
  }
  //reset location accumulators ready for next frame
  m_y1 = std::numeric_limits<myint>::min();
  m_y2 = std::numeric_limits<myint>::max();
  
  // Check if we've reached the end of a chunk
  if(m_iLastSym  == m_vTargetSymbols.size() - 1) {
    m_pInterface->Message(ComputeStats(m_vTargetY),true);
    m_vTargetY.clear(); //could preserve if samples not excessive...but is it meaningful (given restart)?
    m_pInterface->GetActiveInputMethod()->pause();
    m_ulTotalTime += (lTime - m_ulSentenceStartTime);
    m_dTotalNats += (pModel->GetNats() - m_dSentenceStartNats);
    m_uiTotalSyms += m_vTargetSymbols.size();
    if (!GenerateChunk()) {
      m_pInterface->Message("Game mode sentence file finished!",true);
      //note this deletes the 'this' pointer...
      SetBoolParameter(BP_GAME_MODE, false);
      //so better get out of here, fast!
      return;
    }
  }

  DrawText(pView);
}

bool CGameModule::GenerateChunk() {
  m_iLastSym = -1;
  m_vTargetSymbols.clear();
  m_strWrong="";
  m_pWordGenerator->GetSymbols(m_vTargetSymbols);
  m_pInterface->ClearAllContext();
  m_ulSentenceStartTime = 0;
  if (m_vTargetSymbols.empty()) return false;
  ChunkGenerated();
  return true;
}
