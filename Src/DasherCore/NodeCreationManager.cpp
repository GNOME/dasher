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

  const Dasher::CAlphInfo *pAlphInfo(pAlphIO->GetInfo(pSettingsStore->GetStringParameter(SP_ALPHABET_ID)));
  const CAlphabetMap *pAlphMap = pAlphInfo->MakeMap();
  
  pSettingsStore->SetStringParameter(SP_TRAIN_FILE, pAlphInfo->GetTrainingFile());
  pSettingsStore->SetStringParameter(SP_GAME_TEXT_FILE, pAlphInfo->GetGameModeFile());
  
  pSettingsStore->SetStringParameter(SP_DEFAULT_COLOUR_ID, pAlphInfo->GetPalette());
  
  // --
  
  // Create an appropriate language model;
  CLanguageModel *pLanguageModel;
  //WZ: Mandarin Dasher Change
  //If statement checks for the specific Super PinYin alphabet, and sets language model to PPMPY
  if((pAlphInfo->m_iConversionID==2)&&(pSettingsStore->GetStringParameter(SP_ALPHABET_ID)=="Chinese Super Pin Yin, grouped by Dictionary")){
    
    std::string CHAlphabet = "Chinese 简体中文 (simplified chinese, in pin yin groups, and pinyin)";
    const Dasher::CAlphInfo *pCHAlphInfo(pAlphIO->GetInfo(CHAlphabet));
    const CAlphabetMap *pCHAlphMap = pCHAlphInfo->MakeMap();
    
    //std::cout<<"CHALphabet size "<< pCHAlphabet->GetNumberTextSymbols(); [7603]
    pLanguageModel = new CPPMPYLanguageModel(pEventHandler, pSettingsStore, pCHAlphInfo, pAlphInfo);
    m_pTrainer = new CMandarinTrainer(pLanguageModel, pAlphMap, pCHAlphMap);
    std::cout<<"Setting PPMPY model"<<std::endl;
  }
  else{
    //End Mandarin Dasher Change
    
    // FIXME - return to using enum here
    switch (pSettingsStore->GetLongParameter(LP_LANGUAGE_MODEL_ID)) {
      case 0:
        pLanguageModel = new CPPMLanguageModel(pEventHandler, pSettingsStore, pAlphInfo);
        break;
      case 2:
        pLanguageModel = new CWordLanguageModel(pEventHandler, pSettingsStore, pAlphInfo, pAlphMap);
        break;
      case 3:
        pLanguageModel = new CMixtureLanguageModel(pEventHandler, pSettingsStore, pAlphInfo, pAlphMap);
        break;  
      case 4:
        pLanguageModel = new CCTWLanguageModel(pEventHandler, pSettingsStore, pAlphInfo);
        break;
        
      default:
        // If there is a bogus value for the language model ID, we'll default
        // to our trusty old PPM language model.
        pLanguageModel = new CPPMLanguageModel(pEventHandler, pSettingsStore, pAlphInfo);    
        break;
    }
    m_pTrainer = new CTrainer(pLanguageModel, pAlphMap);
  }
    
    switch(pAlphInfo->m_iConversionID) {
      default:
        //TODO: Error reporting here
        //fall through to
      case 0: // No conversion required
        m_pAlphabetManager = new CAlphabetManager(pInterface, this, pAlphInfo, pAlphMap, pLanguageModel);
        break;
#ifdef JAPANESE
      case 1: // Japanese
        CConversionManager *pConversionManager =
#ifdef WIN32
        new CIMEConversionHelper;
#else
        new CCannaConversionHelper(this, pAlphInfo, GetLongParameter(LP_CONVERSION_TYPE), GetLongParameter(LP_CONVERSION_ORDER));
#endif
        //TODO ownership/deletion
        m_pAlphabetManager = new CConvertingAlphMgr(pInterface, this, pConversionManager, pAlphInfo, pLanguageModel);
        break;
#endif
      case 2:   //(ACL) Modify AlphabetManager for Mandarin Dasher
        m_pAlphabetManager = new CMandarinAlphMgr(pInterface, this, pAlphInfo, pAlphMap, pLanguageModel);
    }

  if (!pAlphInfo->GetTrainingFile().empty()) {
    //1. Look for system training text...
    CLockEvent oEvent("Training on System Text", true, 0);
    pEventHandler->InsertEvent(&oEvent);
    m_pTrainer->LoadFile(GetStringParameter(SP_SYSTEM_LOC) + pAlphInfo->GetTrainingFile());
    //Now add in any user-provided individual training text...
    oEvent.m_strMessage = "Training on User Text"; oEvent.m_bLock=true; oEvent.m_iPercent = 0;
    pEventHandler->InsertEvent(&oEvent);
    m_pTrainer->LoadFile(GetStringParameter(SP_USER_LOC) + pAlphInfo->GetTrainingFile());
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
    pLanguageModel->WriteToFile("test.model");
    CPPMLanguageModel *pLan = (CPPMLanguageModel *)pLanguageModel;
    CPPMLanguageModel *pLM2 = new CPPMLanguageModel(pEventHandler, pSettingsStore, pAlphInfo);
    pLM2->ReadFromFile("test.model");
    if (!pLan->eq(pLM2)) {
      std::cout << "Not equal!" << std::endl;
      pLM2->WriteToFile("test2.model");
    }
    delete pLM2;
  }
#endif

  HandleEvent(&CParameterNotificationEvent(LP_ORIENTATION));
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
      case LP_ORIENTATION: {
        const long iOverride(GetLongParameter(LP_ORIENTATION));
        SetLongParameter(LP_REAL_ORIENTATION,
                         iOverride == Dasher::Opts::AlphabetDefault ? GetAlphabet()->GetOrientation() : iOverride);
      }
    }
  }
}

void 
CNodeCreationManager::ImportTrainingText(const std::string &strPath) {
	m_pTrainer->LoadFile(strPath);
}
