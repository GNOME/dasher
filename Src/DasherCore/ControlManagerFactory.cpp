
#include "../Common/Common.h"

#include "ControlManagerFactory.h"
#include "ControlManager.h"

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CControlManagerFactory::CControlManagerFactory( CDasherModel *pModel, CLanguageModel *pLanguageModel ) {
  m_pControlManager = new CControlManager(pModel, pLanguageModel);
}

CControlManagerFactory::~CControlManagerFactory() {
  delete m_pControlManager;
}

CDasherNode *CControlManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  return m_pControlManager->GetRoot(pParent, iLower, iUpper, pUserData);
}

void CControlManagerFactory::RegisterNode( int iID, const std::string &strLabel, int iColour ) {
  m_pControlManager->RegisterNode(iID, strLabel, iColour);
}

void CControlManagerFactory::ConnectNode(int iChild, int iParent, int iAfter) {
  m_pControlManager->ConnectNode(iChild, iParent, iAfter);
}
