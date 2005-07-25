#include "AlphabetManagerFactory.h"

using namespace Dasher;

CAlphabetManagerFactory::CAlphabetManagerFactory( CDasherModel *pModel, CLanguageModel *pLanguageModel ) {
  m_pAlphabetManager = new CAlphabetManager( pModel, pLanguageModel );
}

CAlphabetManagerFactory::~CAlphabetManagerFactory() {
  delete m_pAlphabetManager;
}

CDasherNode *CAlphabetManagerFactory::GetRoot() {
  return m_pAlphabetManager->GetRoot();
}
