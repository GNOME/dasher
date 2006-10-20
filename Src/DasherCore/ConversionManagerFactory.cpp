#ifndef WIN32
#include "config.h"
#endif

#include "ConversionManager.h"
#include "ConversionManagerFactory.h"
#include "DasherModel.h"

#ifdef WIN32
#include "IMEConversionHelper.h"
#else

#ifdef CHINESE
#include "PinYinConversionHelper.h"
#endif

#include "CannaConversionHelper.h"

#endif

using namespace Dasher;

CConversionManagerFactory::CConversionManagerFactory(CNodeCreationManager *pNCManager, int iID) {
  m_pNCManager = pNCManager;
  m_pHelper = GetHelper(iID);

  // TODO: These shouldn't be here - need to figure out exactly how it all works
  pagecount = 0; // TODO: Doesn't actually appear to do anything
  m_iCMCount = 0; // Unique identifier passed to conversion managers
}

CDasherNode *CConversionManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CConversionManager *pConversionManager(new CConversionManager(m_pNCManager, m_pHelper, m_iCMCount));

  if(m_iCMCount >= MAX_CM_NUM-1){
    pagecount ++;
    m_iCMCount =0;
  }
  else
    m_iCMCount++;
  return pConversionManager->GetRoot(pParent, iLower, iUpper, pUserData);
}

CConversionHelper *CConversionManagerFactory::GetHelper(int iID) {
  switch(iID) {
  case 0: // No conversion required (shouldn't really be called)
    return NULL;
  case 1: // Japanese
#ifdef WIN32
    return new CIMEConversionHelper;
#else
    return new CCannaConversionHelper;
#endif
  case 2: // Chinese
#ifdef WIN32
    return NULL;
#else
    return new CPinYinConversionHelper;
#endif
  default:
    // TODO: Error reporting here
    return NULL;
  }
}
