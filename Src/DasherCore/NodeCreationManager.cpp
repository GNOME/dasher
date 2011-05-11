#include "DasherNode.h"
#include "DasherInterfaceBase.h"
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
  m_pInterface(pInterface), m_pControlManager(NULL), m_pScreen(NULL) {

  const Dasher::CAlphInfo *pAlphInfo(pAlphIO->GetInfo(pSettingsStore->GetStringParameter(SP_ALPHABET_ID)));
  const CAlphabetMap *pAlphMap = pAlphInfo->MakeMap();
  
  pSettingsStore->SetStringParameter(SP_GAME_TEXT_FILE, pAlphInfo->GetGameModeFile());
  
  pSettingsStore->SetStringParameter(SP_DEFAULT_COLOUR_ID, pAlphInfo->GetPalette());
  
  // --
  
  switch (pAlphInfo->m_iConversionID) {
    default:
      //TODO: Error reporting here
      //fall through to
    case 0: // No conversion required
      m_pAlphabetManager = new CAlphabetManager(pInterface, this, pAlphInfo, pAlphMap);
      break;      
#ifdef JAPANESE
    case 1: {
      // Japanese
      CConversionManager *pConversionManager =
#ifdef WIN32
      new CIMEConversionHelper;
#else
      new CCannaConversionHelper(this, pAlphInfo, GetLongParameter(LP_CONVERSION_TYPE), GetLongParameter(LP_CONVERSION_ORDER));
#endif
      //TODO ownership/deletion
      m_pAlphabetManager = new CConvertingAlphMgr(pInterface, this, pConversionManager, pAlphInfo, pLanguageModel);
      break;
    }
#endif
    case 2:
      //Mandarin Dasher!
      //(ACL) Modify AlphabetManager for Mandarin Dasher
      m_pAlphabetManager = new CMandarinAlphMgr(pInterface, this, pAlphInfo, pAlphMap);
      break;
  }
  //all other configuration changes, etc., that might be necessary for a particular conversion mode,
  // are implemented by AlphabetManager subclasses overriding the following two methods:
  m_pAlphabetManager->CreateLanguageModel(pEventHandler, pSettingsStore);
  m_pTrainer = m_pAlphabetManager->GetTrainer();
    
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

void CNodeCreationManager::ChangeScreen(CDasherScreen *pScreen) {
  if (m_pScreen == pScreen) return;
  m_pScreen = pScreen;
  m_pAlphabetManager->MakeLabels(pScreen);
  if (m_pControlManager) m_pControlManager->MakeLabels(pScreen);
}

void CNodeCreationManager::HandleEvent(CEvent *pEvent) {
  if (pEvent->m_iEventType == EV_PARAM_NOTIFY) {
    switch (static_cast<CParameterNotificationEvent *>(pEvent)->m_iParameter) {
      case BP_CONTROL_MODE: {
        delete m_pControlManager;
        const unsigned long iNorm(GetLongParameter(LP_NORMALIZATION));
        unsigned long iControlSpace;
        if (GetBoolParameter(BP_CONTROL_MODE)) {
          m_pControlManager = new CControlManager(m_pEventHandler, m_pSettingsStore, this, m_pInterface);
          if (m_pScreen) m_pControlManager->MakeLabels(m_pScreen);
          iControlSpace = iNorm / 20;
        } else {
          m_pControlManager = NULL;
          iControlSpace = 0;
        }
        m_iAlphNorm = iNorm-iControlSpace;
        break;
      }
      case LP_ORIENTATION: {
        const long iOverride(GetLongParameter(LP_ORIENTATION));
        SetLongParameter(LP_REAL_ORIENTATION,
                         iOverride == Dasher::Opts::AlphabetDefault ? GetAlphabet()->GetOrientation() : iOverride);
      }
    }
  }
}


void CNodeCreationManager::AddExtras(CDasherNode *pParent) {
  //control mode:
  DASHER_ASSERT(pParent->GetChildren().back()->Hbnd() == m_iAlphNorm);
  if (GetBoolParameter(BP_CONTROL_MODE)) {
#ifdef _WIN32_WCE
    DASHER_ASSERT(false);
#endif
    //ACL leave offset as is - like its groupnode parent, but unlike its alphnode siblings,
    //the control node does not enter a symbol....
    m_pControlManager->GetRoot(pParent, pParent->GetChildren().back()->Hbnd(), GetLongParameter(LP_NORMALIZATION), pParent->offset());
  }
}

void 
CNodeCreationManager::ImportTrainingText(const std::string &strPath) {
	m_pTrainer->LoadFile(strPath);
}
