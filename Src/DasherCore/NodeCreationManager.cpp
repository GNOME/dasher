#include "DasherNode.h"
#include "DasherInterfaceBase.h"
#include "LanguageModelling/PPMLanguageModel.h"
#include "LanguageModelling/WordLanguageModel.h"
#include "LanguageModelling/DictLanguageModel.h"
#include "LanguageModelling/MixtureLanguageModel.h"
#include "LanguageModelling/PPMPYLanguageModel.h"
#include "LanguageModelling/CTWLanguageModel.h"
#include "NodeCreationManager.h"
#include "MandarinAlphMgr.h"
#include "ConvertingAlphMgr.h"
#include "ControlManager.h"
#include "EventHandler.h"

using namespace Dasher;

CNodeCreationManager::CNodeCreationManager(Dasher::CDasherInterfaceBase *pInterface,
					   Dasher::CEventHandler *pEventHandler, 
					   CSettingsStore *pSettingsStore,
					   Dasher::CAlphIO *pAlphIO) : CDasherComponent(pEventHandler, pSettingsStore),
  m_pInterface(pInterface), m_pControlManager(NULL) {

  const Dasher::CAlphIO::AlphInfo &oAlphInfo(pAlphIO->GetInfo(pSettingsStore->GetStringParameter(SP_ALPHABET_ID)));
  m_pAlphabet = new CAlphabet(oAlphInfo);
  
  pSettingsStore->SetStringParameter(SP_TRAIN_FILE, m_pAlphabet->GetTrainingFile());
  pSettingsStore->SetStringParameter(SP_GAME_TEXT_FILE, m_pAlphabet->GetGameModeFile());
  
  pSettingsStore->SetStringParameter(SP_DEFAULT_COLOUR_ID, m_pAlphabet->GetPalette());
  
  if(pSettingsStore->GetLongParameter(LP_ORIENTATION) == Dasher::Opts::AlphabetDefault)
    pSettingsStore->SetLongParameter(LP_REAL_ORIENTATION, m_pAlphabet->GetOrientation());
  // --
  
  // Create an appropriate language model;
  
  //WZ: Mandarin Dasher Change
  //If statement checks for the specific Super PinYin alphabet, and sets language model to PPMPY
  if((oAlphInfo.m_iConversionID==2)&&(pSettingsStore->GetStringParameter(SP_ALPHABET_ID)=="Chinese Super Pin Yin, grouped by Dictionary")){
    
    std::string CHAlphabet = "Chinese / 简体中文 (simplified chinese, in pin yin groups)";
    Dasher::CAlphIO::AlphInfo oCHAlphInfo = pAlphIO->GetInfo(CHAlphabet);
    CAlphabet *pCHAlphabet = new CAlphabet(oCHAlphInfo);
    
    //std::cout<<"CHALphabet size "<< pCHAlphabet->GetNumberTextSymbols(); [7603]
    m_pLanguageModel = new CPPMPYLanguageModel(pEventHandler, pSettingsStore, pCHAlphabet, m_pAlphabet);
    m_pTrainer = new CMandarinTrainer(m_pLanguageModel, m_pAlphabet, pCHAlphabet);
    std::cout<<"Setting PPMPY model"<<std::endl;
  }
  else{
    //End Mandarin Dasher Change
    
    // FIXME - return to using enum here
    switch (pSettingsStore->GetLongParameter(LP_LANGUAGE_MODEL_ID)) {
      case 0:
        m_pLanguageModel = new CPPMLanguageModel(pEventHandler, pSettingsStore, m_pAlphabet);
        break;
      case 2:
        m_pLanguageModel = new CWordLanguageModel(pEventHandler, pSettingsStore, m_pAlphabet);
        break;
      case 3:
        m_pLanguageModel = new CMixtureLanguageModel(pEventHandler, pSettingsStore, m_pAlphabet);
        break;  
      case 4:
        m_pLanguageModel = new CCTWLanguageModel(pEventHandler, pSettingsStore, m_pAlphabet);
        break;
        
      default:
        // If there is a bogus value for the language model ID, we'll default
        // to our trusty old PPM language model.
        m_pLanguageModel = new CPPMLanguageModel(pEventHandler, pSettingsStore, m_pAlphabet);    
        break;
    }
    m_pTrainer = new CTrainer(m_pLanguageModel, m_pAlphabet);
  }
    
  // TODO: Tell the alphabet manager about the alphabet here, so we
  // don't end up having to duck out to the NCM all the time

    switch(oAlphInfo.m_iConversionID) {
      default:
        //TODO: Error reporting here
        //fall through to
      case 0: // No conversion required
        m_pAlphabetManager = new CAlphabetManager(pInterface, this, m_pAlphabet, m_pLanguageModel);
        break;
#ifdef JAPANESE
      case 1: // Japanese
        CConversionManager *pConversionManager =
#ifdef WIN32
        new CIMEConversionHelper;
#else
        new CCannaConversionHelper(this, m_pAlphabet, GetLongParameter(LP_CONVERSION_TYPE), GetLongParameter(LP_CONVERSION_ORDER));
#endif
        //TODO ownership/deletion
        m_pAlphabetManager = new CConvertingAlphMgr(pInterface, this, pConversionManager, m_pAlphabet, m_pLanguageModel);
        break;
#endif
      case 2:   //(ACL) Modify AlphabetManager for Mandarin Dasher
        m_pAlphabetManager = new CMandarinAlphMgr(pInterface, this, m_pAlphabet, m_pLanguageModel);
    }

  if (!m_pAlphabet->GetTrainingFile().empty()) {
    //1. Look for system training text...
    CLockEvent oEvent("Training on System Text", true, 0);
    pEventHandler->InsertEvent(&oEvent);
    m_pTrainer->LoadFile(GetStringParameter(SP_SYSTEM_LOC) + m_pAlphabet->GetTrainingFile());
    //Now add in any user-provided individual training text...
    oEvent.m_strMessage = "Training on User Text"; oEvent.m_bLock=true; oEvent.m_iPercent = 0;
    pEventHandler->InsertEvent(&oEvent);
    m_pTrainer->LoadFile(GetStringParameter(SP_USER_LOC) + m_pAlphabet->GetTrainingFile());
    oEvent.m_bLock = false;
    pEventHandler->InsertEvent(&oEvent);
  }
#ifdef DEBUG
  else {
    std::cout << "Alphabet does not specify training file" << std::endl;
  }
#endif
#ifdef DEBUG_LM_READWRITE
  {
    //test...
    m_pLanguageModel->WriteToFile("test.model");
    CPPMLanguageModel *pLan = (CPPMLanguageModel *)m_pLanguageModel;
    CPPMLanguageModel *pLM2 = new CPPMLanguageModel(pEventHandler, pSettingsStore, m_pAlphabet);
    pLM2->ReadFromFile("test.model");
    if (!pLan->eq(pLM2)) {
      std::cout << "Not equal!" << std::endl;
      pLM2->WriteToFile("test2.model");
    }
    delete pLM2;
  }
#endif

  HandleEvent(&CParameterNotificationEvent(BP_CONTROL_MODE));

}

CNodeCreationManager::~CNodeCreationManager() {
  delete m_pAlphabetManager;
  delete m_pTrainer;
  
  delete m_pControlManager;
}

void CNodeCreationManager::HandleEvent(CEvent *pEvent) {
  if (pEvent->m_iEventType == EV_PARAM_NOTIFY) {
    switch (static_cast<CParameterNotificationEvent *>(pEvent)->m_iParameter) {
      case BP_CONTROL_MODE:
        delete m_pControlManager;
        m_pControlManager = (GetBoolParameter(BP_CONTROL_MODE))
          ? new CControlManager(m_pEventHandler, m_pSettingsStore, this, m_pInterface)
          : NULL;        
        break;
    }
  }
}

void 
CNodeCreationManager::ImportTrainingText(const std::string &strPath) {
	m_pTrainer->LoadFile(strPath);
}
