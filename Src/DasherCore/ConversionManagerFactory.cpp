#ifndef WIN32
#include "config.h"
#endif

#ifdef JAPANESE

#include "ConversionManager.h"
#include "ConversionManagerFactory.h"
#ifdef WIN32
#include "IMEConversionHelper.h"
#else
#include "CannaConversionHelper.h"
#endif

using namespace Dasher;

CConversionManagerFactory::CConversionManagerFactory(CDasherModel *pModel, CLanguageModel *pLanguageModel) {
  m_pModel = pModel;
  m_pLanguageModel = pLanguageModel;
#ifdef WIN32
  m_pHelper = new CIMEConversionHelper;
#else
  m_pHelper = new CCannaConversionHelper;
#endif
}

CDasherNode *CConversionManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CConversionManager *pConversionManager(new CConversionManager(m_pModel, m_pLanguageModel, m_pHelper));
  return pConversionManager->GetRoot(pParent, iLower, iUpper, pUserData);
}

#endif
