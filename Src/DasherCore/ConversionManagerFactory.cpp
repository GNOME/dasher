#ifndef WIN32
#include "config.h"
#endif

#ifdef JAPANESE

#include "ConversionManager.h"
#include "ConversionManagerFactory.h"
#include "CannaConversionHelper.h"

using namespace Dasher;

CConversionManagerFactory::CConversionManagerFactory(CDasherModel *pModel, CLanguageModel *pLanguageModel) {
  m_pModel = pModel;
  m_pLanguageModel = pLanguageModel;
  m_pHelper = new CCannaConversionHelper;
}

CDasherNode *CConversionManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CConversionManager *pConversionManager(new CConversionManager(m_pModel, m_pLanguageModel, m_pHelper));
  return pConversionManager->GetRoot(pParent, iLower, iUpper, pUserData);
}

#endif
