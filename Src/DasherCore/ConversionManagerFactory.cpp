#include "ConversionManager.h"
#include "ConversionManagerFactory.h"
#include "PinYinConversionHelper.h"

using namespace Dasher;

CConversionManagerFactory::CConversionManagerFactory(CDasherModel *pModel, CLanguageModel *pLanguageModel) {
  m_pModel = pModel;
  m_pLanguageModel = pLanguageModel;
  m_pHelper = new CPinYinConversionHelper;
}

CDasherNode *CConversionManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CConversionManager *pConversionManager(new CConversionManager(m_pModel, m_pLanguageModel, m_pHelper));
  return pConversionManager->GetRoot(pParent, iLower, iUpper, pUserData);
}
