#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
  m_pMgr = GetHelper(pEventHandler, pSettingsStore, iID, pCAlphIO);
	
  //To clean up:
  // TODO: These shouldn't be here - need to figure out exactly how it all works
  pagecount = 0; // TODO: Doesn't actually appear to do anything
  m_iCMCount = 0; // Unique identifier passed to conversion managers
}

CDasherNode *CConversionManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  return m_pMgr->GetRoot(pParent, iLower, iUpper, pUserData);
}

CConversionManagerFactory::~CConversionManagerFactory() {
  m_pMgr->Unref();
}

// TODO: Japanese/Chinese are currently disabled in Win32 - see 'exclude from build' on individual files' property pages, plus preprocessor defines

CConversionManager *CConversionManagerFactory::GetHelper(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, int iID, Dasher::CAlphIO *pCAlphIO) {
  switch(iID) {
  case 0: // No conversion required (shouldn't really be called)
	return NULL; //or, new CConversionManager(m_pNCManager, pHelper, m_pAlphabet); ??
  case 1: // Japanese
#ifdef JAPANESE
#ifdef WIN32
    return new CIMEConversionHelper;
#else
    return new CCannaConversionHelper(m_pNCManager, m_pAlphabet, pSettingsStore->GetLongParameter(LP_CONVERSION_TYPE), pSettingsStore->GetLongParameter(LP_CONVERSION_ORDER));
#endif
#else
    return NULL;
#endif
  case 2: // Chinese
    return GetHelperChinese(pEventHandler, pSettingsStore, pCAlphIO);
  default:
    // TODO: Error reporting here
    return new CConversionManager(m_pNCManager, m_pAlphabet);
  }
}

CConversionManager *CConversionManagerFactory::GetHelperChinese(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, Dasher::CAlphIO *pCAlphIO) {
#ifdef CHINESE
  std::string strCHAlphabetPath = pSettingsStore->GetStringParameter(SP_SYSTEM_LOC);
  strCHAlphabetPath += "/alphabet.chineseRuby.xml";

  return new CPinYinConversionHelper(m_pNCManager, pEventHandler,pSettingsStore, pCAlphIO, strCHAlphabetPath, m_pAlphabet);
#else
  return NULL; //or, new ConversionManager(m_pNCManager, m_pAlphabet); ??
#endif
}
