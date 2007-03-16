#ifndef WIN32
#include "config.h"
#endif

#include "ConversionManager.h"
#include "ConversionManagerFactory.h"
#include "DasherModel.h"

#ifdef WIN32
#ifdef JAPANESE
#include "IMEConversionHelper.h"
#endif
#else

#ifdef CHINESE

#include "PinYinConversionHelper.h"
#endif

#ifdef JAPANESE
#include "CannaConversionHelper.h"
#endif

#endif

using namespace Dasher;

CConversionManagerFactory::CConversionManagerFactory(Dasher::CEventHandler *pEventHandler,  CSettingsStore *pSettingsStore, CNodeCreationManager *pNCManager, int iID, Dasher::CAlphIO *pCAlphIO, CAlphabet *pAlphabet) {
  m_pNCManager = pNCManager;
  m_pAlphabet = pAlphabet;

  // TODO: Need to deal with the case of GetHelper returning NULL
  m_pHelper = GetHelper(pEventHandler, pSettingsStore, iID, pCAlphIO);

  // TODO: These shouldn't be here - need to figure out exactly how it all works
  pagecount = 0; // TODO: Doesn't actually appear to do anything
  m_iCMCount = 0; // Unique identifier passed to conversion managers
}

CDasherNode *CConversionManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  CConversionManager *pConversionManager(new CConversionManager(m_pNCManager, m_pHelper, m_pAlphabet, m_iCMCount));

  if(m_iCMCount >= MAX_CM_NUM-1){
    pagecount ++;
    m_iCMCount =0;
  }
  else
    m_iCMCount++;

  CDasherNode *pNewRoot = pConversionManager->GetRoot(pParent, iLower, iUpper, pUserData);
  pConversionManager->Unref();

  return pNewRoot;
}

// TODO: Japanese/Chinese are currently disabled in Win32 - see 'exclude from build' on individual files' property pages, plus preprocessor defines

CConversionHelper *CConversionManagerFactory::GetHelper(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, int iID, Dasher::CAlphIO *pCAlphIO) {
  switch(iID) {
  case 0: // No conversion required (shouldn't really be called)
    return NULL;
  case 1: // Japanese
#ifdef JAPANESE
#ifdef WIN32
    return new CIMEConversionHelper;
#else
    return new CCannaConversionHelper;
#endif
#else
    return NULL;
#endif
  case 2: // Chinese
#ifdef WIN32
    return NULL;
#else
#ifdef CHINESE
    return new CPinYinConversionHelper(pEventHandler,pSettingsStore, pCAlphIO);
#else
    return NULL;
#endif
#endif
  default:
    // TODO: Error reporting here
    return NULL;
  }
}
