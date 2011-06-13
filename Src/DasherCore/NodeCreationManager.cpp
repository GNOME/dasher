#include "DasherNode.h"
#include "DasherInterfaceBase.h"
#include "NodeCreationManager.h"
#include "MandarinAlphMgr.h"
#include "ConvertingAlphMgr.h"
#include "ControlManager.h"
#include "EventHandler.h"

using namespace Dasher;

class ProgressNotifier : public CTrainer::ProgressIndicator {
public:
  ProgressNotifier(CDasherInterfaceBase *pInterface, CTrainer *pTrainer)
  : m_pInterface(pInterface), m_pTrainer(pTrainer) { }
  void bytesRead(off_t n) {
    int iNewPercent = ((m_iStart + n)*100)/m_iStop;
    if (iNewPercent != m_iPercent) {
      m_pInterface->SetLockStatus(m_strDisplay, m_iPercent = iNewPercent);
    }
  }
  void run(const string &strDisplay, string strFile) {
    m_pInterface->SetLockStatus(m_strDisplay=strDisplay, m_iPercent=0);
    m_iStart = 0;
    m_iStop = m_pInterface->GetFileSize(strFile);
    m_pTrainer->LoadFile(strFile,this);
  }
private:
  CDasherInterfaceBase *m_pInterface;
  CTrainer *m_pTrainer;
  off_t m_iStart, m_iStop;
  int m_iPercent;
  string m_strDisplay;
};

CNodeCreationManager::CNodeCreationManager(Dasher::CDasherInterfaceBase *pInterface,
                                           Dasher::CEventHandler *pEventHandler, 
                                           CSettingsStore *pSettingsStore,
                                           const Dasher::CAlphIO *pAlphIO) : CDasherComponent(pEventHandler, pSettingsStore),
  m_pInterface(pInterface), m_pControlManager(NULL), m_pScreen(NULL) {

  const Dasher::CAlphInfo *pAlphInfo(pAlphIO->GetInfo(pSettingsStore->GetStringParameter(SP_ALPHABET_ID)));
  
  pSettingsStore->SetStringParameter(SP_GAME_TEXT_FILE, pAlphInfo->GetGameModeFile());
  
  pSettingsStore->SetStringParameter(SP_DEFAULT_COLOUR_ID, pAlphInfo->GetPalette());
  
  // --
  
  switch (pAlphInfo->m_iConversionID) {
    default:
      //TODO: Error reporting here
      //fall through to
    case 0: // No conversion required
      m_pAlphabetManager = new CAlphabetManager(pInterface, this, pAlphInfo);
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
      m_pAlphabetManager = new CMandarinAlphMgr(pInterface, this, pAlphInfo, pAlphIO);
      break;
  }
  //all other configuration changes, etc., that might be necessary for a particular conversion mode,
  // are implemented by AlphabetManager subclasses overriding the following two methods:
  m_pAlphabetManager->CreateLanguageModel(pEventHandler, pSettingsStore);
  m_pTrainer = m_pAlphabetManager->GetTrainer();
    
  if (!pAlphInfo->GetTrainingFile().empty()) {
    ProgressNotifier pn(pInterface, m_pTrainer);

    //1. Look for system training text...
    pn.run("Training on System Text", GetStringParameter(SP_SYSTEM_LOC) + pAlphInfo->GetTrainingFile());
    //2. Now add in any user-provided individual training text...
    pn.run("Training on User Text", GetStringParameter(SP_USER_LOC) + pAlphInfo->GetTrainingFile());
    //3. Finished, so unlock.
    m_pInterface->SetLockStatus("", -1);
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
  ProgressNotifier pn(m_pInterface, m_pTrainer);
	pn.run("Training on New Text", strPath);
}
