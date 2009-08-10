
#include "../Common/Common.h"

#include "AlphabetManagerFactory.h"
#include "MandarinAlphMgr.h"
#include "DasherInterfaceBase.h"
#include "LanguageModelling/PPMLanguageModel.h"
#include "LanguageModelling/WordLanguageModel.h"
#include "LanguageModelling/DictLanguageModel.h"
#include "LanguageModelling/MixtureLanguageModel.h"
#include "LanguageModelling/CTWLanguageModel.h"
#include "LanguageModelling/PPMPYLanguageModel.h"

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CAlphabetManagerFactory::CAlphabetManagerFactory(CDasherInterfaceBase *pInterface,
						 CEventHandler *pEventHandler,
						 CSettingsStore *pSettingsStore,
						 Dasher::CAlphIO *pAlphIO,
						 CNodeCreationManager *pNCManager)
{
  // -- put all this in a separate method
  // TODO: Think about having 'prefered' values here, which get
  // retrieved by DasherInterfaceBase and used to set parameters

  // TODO: We might get a different alphabet to the one we asked for -
  // if this is the case then the parameter value should be updated,
  // but not in such a way that it causes everything to be rebuilt.

  const Dasher::CAlphIO::AlphInfo &oAlphInfo(pAlphIO->GetInfo(pSettingsStore->GetStringParameter(SP_ALPHABET_ID)));
  m_pAlphabet = new CAlphabet(oAlphInfo);
  m_pCHAlphabet = NULL;

  pSettingsStore->SetStringParameter(SP_TRAIN_FILE, m_pAlphabet->GetTrainingFile());
  pSettingsStore->SetStringParameter(SP_GAME_TEXT_FILE, m_pAlphabet->GetGameModeFile());
  
  pSettingsStore->SetStringParameter(SP_DEFAULT_COLOUR_ID, m_pAlphabet->GetPalette());

  if(pSettingsStore->GetLongParameter(LP_ORIENTATION) == Dasher::Opts::AlphabetDefault)
    pSettingsStore->SetLongParameter(LP_REAL_ORIENTATION, m_pAlphabet->GetOrientation());
  // --

  CSymbolAlphabet alphabet(m_pAlphabet->GetNumberTextSymbols());
  alphabet.SetSpaceSymbol(m_pAlphabet->GetSpaceSymbol());      // FIXME - is this right, or do we have to do some kind of translation?
  alphabet.SetAlphabetPointer(m_pAlphabet);    // Horrible hack, but ignore for now.

  // Create an appropriate language model;

  m_iConversionID = oAlphInfo.m_iConversionID;

  //WZ: Mandarin Dasher Change
  //If statement checks for the specific Super PinYin alphabet, and sets language model to PPMPY
  if((m_iConversionID==2)&&(pSettingsStore->GetStringParameter(SP_ALPHABET_ID)=="Chinese Super Pin Yin, grouped by Dictionary")){

    std::string CHAlphabet = "Chinese / 简体中文 (simplified chinese, in pin yin groups)";
    Dasher::CAlphIO::AlphInfo oCHAlphInfo = pAlphIO->GetInfo(CHAlphabet);
    m_pCHAlphabet = new CAlphabet(oCHAlphInfo);

    CSymbolAlphabet chalphabet(m_pCHAlphabet->GetNumberTextSymbols());
    chalphabet.SetSpaceSymbol(m_pCHAlphabet->GetSpaceSymbol());
    chalphabet.SetAlphabetPointer(m_pCHAlphabet);
    //std::cout<<"CHALphabet size "<<chalphabet.GetSize(); [7603]
    m_pLanguageModel = new CPPMPYLanguageModel(pEventHandler, pSettingsStore, chalphabet, alphabet);
	m_pTrainer = new CMandarinTrainer(m_pLanguageModel, m_pAlphabet, m_pCHAlphabet);
    std::cout<<"Setting PPMPY model"<<std::endl;
  }
  else{
  //End Mandarin Dasher Change

  // FIXME - return to using enum here
    switch (pSettingsStore->GetLongParameter(LP_LANGUAGE_MODEL_ID)) {
    case 0:
      m_pLanguageModel = new CPPMLanguageModel(pEventHandler, pSettingsStore, alphabet);
      break;
    case 2:
      m_pLanguageModel = new CWordLanguageModel(pEventHandler, pSettingsStore, alphabet);
      break;
    case 3:
      m_pLanguageModel = new CMixtureLanguageModel(pEventHandler, pSettingsStore, alphabet);
      break;  
    case 4:
      m_pLanguageModel = new CCTWLanguageModel(pEventHandler, pSettingsStore, alphabet);
      break;
      
    default:
      // If there is a bogus value for the language model ID, we'll default
      // to our trusty old PPM language model.
      m_pLanguageModel = new CPPMLanguageModel(pEventHandler, pSettingsStore, alphabet);    
      break;
    }
    m_pTrainer = new CTrainer(m_pLanguageModel, m_pAlphabet);
  }

  m_iLearnContext = m_pLanguageModel->CreateEmptyContext();
  
  // TODO: Tell the alphabet manager about the alphabet here, so we
  // don't end up having to duck out to the NCM all the time
  
  //(ACL) Modify AlphabetManager for Mandarin Dasher
  if (m_iConversionID == 2)
	m_pAlphabetManager = new CMandarinAlphMgr(pInterface, pNCManager, m_pLanguageModel, m_iLearnContext);
  else
    m_pAlphabetManager = new CAlphabetManager(pInterface, pNCManager, m_pLanguageModel, m_iLearnContext);
}

CAlphabetManagerFactory::~CAlphabetManagerFactory() {
  m_pLanguageModel->ReleaseContext(m_iLearnContext);
  delete m_pLanguageModel;
  delete m_pAlphabetManager;
}

CDasherNode *CAlphabetManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  return m_pAlphabetManager->GetRoot(pParent, iLower, iUpper, pUserData);
}
