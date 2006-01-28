
#include "../Common/Common.h"

#include "AlphabetManagerFactory.h"

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

CAlphabetManagerFactory::CAlphabetManagerFactory( CDasherModel *pModel, CLanguageModel *pLanguageModel ) {
  m_pAlphabetManager = new CAlphabetManager( pModel, pLanguageModel );
}

CAlphabetManagerFactory::~CAlphabetManagerFactory() {
  delete m_pAlphabetManager;
}

CDasherNode *CAlphabetManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  return m_pAlphabetManager->GetRoot(pParent, iLower, iUpper, pUserData);
}
