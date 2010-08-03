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

  m_vTargetY.push_back(m_iTargetY = (m_y1+m_y2)/2);
  
  //draw a line along the y axis
  myint x[2], y[2];
  x[0] = x[1] = -100;
  
  bool bDrawHelper=true;
  const int lineWidth(GetLongParameter(LP_LINE_WIDTH));
  
  if (m_y1 > CDasherModel::MAX_Y) {
    //off the top! draw an arrow pointing up...
    y[1] = CDasherModel::MAX_Y;
    y[0] = y[1] - 400;
  } else if (m_y2 < 0) {
    //off the bottom! draw arrow pointing down...
    y[1] = 0;
    y[0] = 400;
  } else {
    //draw line parallel to that region of y-axis
    y[0] = m_y1; y[1] = m_y2;
    pView->DasherPolyline(x, y, 2, lineWidth, m_iCrosshairColor);
    //and a horizontal arrow pointing to the midpoint
    x[0] = -400;
    y[0] = y[1] = m_iTargetY;
    bDrawHelper=false;
  }
  pView->DasherPolyarrow(x, y, 2, 3*lineWidth, m_iCrosshairColor, 0.2);
  
  if (bDrawHelper) DrawBrachistochrone(pView);
  
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

void CGameModule::DrawBrachistochrone(CDasherView *pView) {
  // Plot a brachistochrone - the optimal path from the crosshair to the target
  // this is a circle, passing through both crosshair and target, centered on the y-axis
  const myint CenterY = ComputeBrachCenter();
  pView->DasherSpaceArc(CenterY, abs(CenterY - m_iTargetY), CDasherModel::ORIGIN_X, CDasherModel::ORIGIN_Y, 0, m_iTargetY, m_iCrosshairColor, 2*(int)GetLongParameter(LP_LINE_WIDTH));
}

void CGameModule::DrawHelperArrow(Dasher::CDasherView* pView)
{
  // This plots a helpful pointer to the best direction to take to get to the target.
  // Probably too much floating point maths here, sort later.
  // Start of line is the crosshair location
  const int gameColour = 135; //Neon green. (!)
  const int noOfPoints = 10; // The curve will be made up of 9 straight segments...
  const myint m_iCrossX(CDasherModel::ORIGIN_X),m_iCrossY(CDasherModel::ORIGIN_Y);
  
  struct {
    myint iTargetY;
    myint iCenterY;
  } m_Target;
  m_Target.iTargetY = m_iTargetY;
  m_Target.iCenterY = ComputeBrachCenter();
  myint iX[noOfPoints];
  myint iY[noOfPoints];
  myint iLength;
  
  // Arrow starts at the cross hairs
  iX[0] = m_iCrossX;
  iY[0] = m_iCrossY;
  
  myint a = m_iCrossX/5;
  myint defaultlength = m_iCrossX - a ; 
  
  // ... then decide the length of the arrow...
  myint r = m_Target.iTargetY-m_Target.iCenterY; // radius of our circle (+ or -)
  
  if(m_Target.iTargetY < a && m_Target.iCenterY < m_iCrossY-defaultlength/2)
  {
    myint x = (myint) sqrt((double)(r*r-pow((double)(m_Target.iCenterY-a),2)));
    iLength = (myint) sqrt((double)(pow((double)(x-m_iCrossX),2)+pow((double)(a-m_iCrossY),2)));
  }
  else if(m_Target.iTargetY > 2*m_iCrossY-a && m_Target.iCenterY > m_iCrossY+defaultlength/2)
  {
    myint x = (myint) sqrt((double)(r*r-pow((double)(m_Target.iCenterY+a-2*m_iCrossY),2)));
    iLength = (myint) sqrt((double)(pow((double)(x-m_iCrossX),2)+pow((double)(a-m_iCrossY),2)));
  }
  else
    iLength = defaultlength;
  
  //...then calculate the points required...
  double angle = ((double)iLength/(double)r)/(double)noOfPoints;
  
  for(int n = 1; n < noOfPoints; ++n)
  {
    iX[n] = (myint) (cos(angle)*(iX[n-1]) - sin(angle)*(iY[n-1]-m_Target.iCenterY));
    iY[n] = (myint) (m_Target.iCenterY + sin(angle)*(iX[n-1]) + cos(angle)*(iY[n-1]-m_Target.iCenterY));
  }
  //...then plot it.
  pView->DasherPolyarrow(iX, iY, noOfPoints, GetLongParameter(LP_LINE_WIDTH)*4, gameColour, 1.414);
  
}

myint CGameModule::ComputeBrachCenter() {
  const myint iCrossX(CDasherModel::ORIGIN_X), iCrossY(CDasherModel::ORIGIN_Y);
  // This formula computes the Dasher Y Coordinate of the center of the circle on which
  // the dasher brachistochrone lies : iCenterY
  
  // It comes from the pythagorean relation: iCrossX^2 + (iCenterY - iCrossY)^2 = r^2
  // where r is the radius of the circle, r = abs(iTargetY-iCenterY)
  return 0.5*(double(iCrossX*iCrossX)/double(iCrossY-m_iTargetY)+iCrossY+m_iTargetY);
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
