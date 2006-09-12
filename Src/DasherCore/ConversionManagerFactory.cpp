#ifndef WIN32
#include "config.h"
#endif

#include "ConversionManager.h"
#include "ConversionManagerFactory.h"
#ifdef WIN32
#include "IMEConversionHelper.h"
#else

#ifdef CHINESE
#include "PinYinConversionHelper.h"
#endif

#endif

using namespace Dasher;

CConversionManagerFactory::CConversionManagerFactory(CDasherModel *pModel, CLanguageModel *pLanguageModel) {
  m_pModel = pModel;
  m_pLanguageModel = pLanguageModel;
#ifdef WIN32
  m_pHelper = new CIMEConversionHelper;
#else
#ifdef CHINESE
  m_pHelper = new CPinYinConversionHelper;
#else
  m_pHelper = NULL;
#endif
#endif

  pagecount = 0;
  m_iCMCount = 0;
}

CDasherNode *CConversionManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CConversionManager *pConversionManager(new CConversionManager(m_pModel, m_pLanguageModel, m_pHelper, m_iCMCount));
  if(m_iCMCount >= MAX_CM_NUM-1){
    pagecount ++;
    m_iCMCount =0;
  }
  else
    m_iCMCount++;
  return pConversionManager->GetRoot(pParent, iLower, iUpper, pUserData);
}
