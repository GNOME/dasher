#include "ControlManagerFactory.h"

using namespace Dasher;

CControlManagerFactory::CControlManagerFactory( CDasherModel *pModel, CLanguageModel *pLanguageModel ) {
  m_pControlManager = new CControlManager(pModel, pLanguageModel);
}

CControlManagerFactory::~CControlManagerFactory() {
  delete m_pControlManager;
}

CDasherNode *CControlManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper) {
  return m_pControlManager->GetRoot(pParent, iLower, iUpper);
}
