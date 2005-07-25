#include "ControlManagerFactory.h"

using namespace Dasher;

CControlManagerFactory::CControlManagerFactory( CDasherModel *pModel ) {
  m_pControlManager = new CControlManager(pModel);
}

CControlManagerFactory::~CControlManagerFactory() {
  delete m_pControlManager;
}

CDasherNode *CControlManagerFactory::GetRoot() {
  return m_pControlManager->GetRoot();
}
