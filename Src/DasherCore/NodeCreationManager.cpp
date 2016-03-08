#include "DasherNode.h"
#include "DasherInterfaceBase.h"
#include "NodeCreationManager.h"
#include "MandarinAlphMgr.h"
#include "RoutingAlphMgr.h"
#include "ConvertingAlphMgr.h"
#include "ControlManager.h"
#include "Observable.h"

#include <string.h>

using namespace Dasher;

//Wraps the ParseFile of a provided Trainer, to setup progress notification
// - and then passes self, as a ProgressIndicator, to the Trainer's ParseFile method.
class ProgressNotifier : public AbstractParser, private CTrainer::ProgressIndicator {
public:
  ProgressNotifier(CDasherInterfaceBase *pInterface, CTrainer *pTrainer)
  : AbstractParser(pInterface), m_bSystem(false), m_bUser(false), m_pInterface(pInterface), m_pTrainer(pTrainer) { }
  void bytesRead(off_t n) {
    int iNewPercent = ((m_iStart + n)*100)/m_iStop;
    if (iNewPercent != m_iPercent) {
      m_pInterface->SetLockStatus(m_strDisplay, m_iPercent = iNewPercent);
    }
  }
  bool ParseFile(const string &strFilename, bool bUser) {
    m_iStart = 0;
    m_iStop = m_pInterface->GetFileSize(strFilename);
    if (m_iStop==0) return false;
    return AbstractParser::ParseFile(strFilename, bUser);
  }
  bool Parse(const string &strUrl, istream &in, bool bUser) {
    m_strDisplay = bUser ? _("Training on User Text") : _("Training on System Text");
    m_pInterface->SetLockStatus(m_strDisplay, m_iPercent=0);
    m_pTrainer->SetProgressIndicator(this);
    if (!m_pTrainer->Parse(strUrl, in, bUser)) return false;
    if (bUser) m_bUser=true; else m_bSystem=true;
    return true;
  }
  bool m_bSystem, m_bUser;
private:
  CDasherInterfaceBase *m_pInterface;
  CTrainer *m_pTrainer;
  off_t m_iStart, m_iStop;
  int m_iPercent;
  string m_strDisplay;
};

CNodeCreationManager::CNodeCreationManager(
  CSettingsUser *pCreateFrom,
  Dasher::CDasherInterfaceBase *pInterface,
  const Dasher::CAlphIO *pAlphIO,
  const Dasher::CControlBoxIO *pControlBoxIO
  ) : CSettingsUserObserver(pCreateFrom),
  m_pInterface(pInterface), m_pControlManager(NULL), m_pScreen(NULL) {

  const Dasher::CAlphInfo *pAlphInfo(pAlphIO->GetInfo(GetStringParameter(SP_ALPHABET_ID)));

  switch (pAlphInfo->m_iConversionID) {
    default:
      //TODO: Error reporting here
      //fall through to
    case 0: // No conversion required
      m_pAlphabetManager = new CAlphabetManager(this, pInterface, this, pAlphInfo);
      break;      
    case 2:
      //Mandarin Dasher!
      //(ACL) Modify AlphabetManager for Mandarin Dasher
      m_pAlphabetManager = new CMandarinAlphMgr(this, pInterface, this, pAlphInfo);
      break;
    case 3: //these differ only in that conversion id 3 assumes the route by which
    case 4: //the user writes a symbol, is not dependent on context (e.g. just user preference),
            //whereas 4 assumes it does depend on context (e.g. phonetic chinese)
      m_pAlphabetManager = new CRoutingAlphMgr(this, pInterface, this, pAlphInfo);
      break;
      //TODO: we could even just switch from standard alphmgr, to case 3, automatically
      // if the alphabet has repeated symbols; and thus do away with much of the "conversionid"
      // tag (just a flag for context-sensitivity, and maybe the start/stop delimiters?)
  }
  //all other configuration changes, etc., that might be necessary for a particular conversion mode,
  // are implemented by AlphabetManager subclasses overriding the following two methods:
  m_pAlphabetManager->Setup();
  m_pTrainer = m_pAlphabetManager->GetTrainer();
    
  if (!pAlphInfo->GetTrainingFile().empty()) {
    ProgressNotifier pn(pInterface, m_pTrainer);
    pInterface->ScanFiles(&pn,pAlphInfo->GetTrainingFile());
    if (!pn.m_bUser) {
      ///TRANSLATORS: These 3 messages will be displayed when the user has just chosen a new alphabet. The %s parameter will be the name of the alphabet.
      const char *msg = pn.m_bSystem ? _("No user training text found - if you have written in \"%s\" before, this means Dasher may not be learning from previous sessions")
      : _("No training text (user or system) found for \"%s\". Dasher will still work but entry will be slower. We suggest downloading a training text file from the Dasher website, or constructing your own.");
      pInterface->FormatMessageWithString(msg, pAlphInfo->GetID().c_str());
    }
    //3. Finished, so unlock.
    m_pInterface->SetLockStatus("", -1);
  }  else {
    pInterface->FormatMessageWithString(_("\"%s\" does not specify training file. Dasher will work but entry will be slower. Check you have the latest version of the alphabet definition."), pAlphInfo->GetID().c_str());
  }
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

  HandleEvent(LP_ORIENTATION);
  CreateControlBox(pControlBoxIO);
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
  if (m_pControlManager) m_pControlManager->ChangeScreen(pScreen);
}

void CNodeCreationManager::CreateControlBox(const CControlBoxIO* pControlIO) {
  delete m_pControlManager;
  unsigned long iControlSpace;
  //don't allow a control manager during Game Mode 
  if (GetBoolParameter(BP_CONTROL_MODE) && !m_pInterface->GetGameModule()) {
    auto id = GetStringParameter(SP_CONTROL_BOX_ID);
    m_pControlManager = pControlIO->CreateControlManager(id, this, this, m_pInterface);
    if (m_pScreen) m_pControlManager->ChangeScreen(m_pScreen);
    iControlSpace = CDasherModel::NORMALIZATION / 20;
  } else {
    m_pControlManager = NULL;
    iControlSpace = 0;
  }
  m_iAlphNorm = CDasherModel::NORMALIZATION-iControlSpace;
}

void CNodeCreationManager::AddExtras(CDasherNode *pParent) {
  //control mode:
  DASHER_ASSERT(pParent->GetChildren().back()->Hbnd() == m_iAlphNorm);
  if (m_pControlManager) {
    //ACL leave offset as is - like its groupnode parent, but unlike its alphnode siblings,
    //the control node does not enter a symbol....
    CDasherNode *ctl = m_pControlManager->GetRoot(pParent, pParent->offset());
    ctl->Reparent(pParent, pParent->GetChildren().back()->Hbnd(), CDasherModel::NORMALIZATION);
  }
}

void 
CNodeCreationManager::ImportTrainingText(const std::string &strPath) {
  ProgressNotifier pn(m_pInterface, m_pTrainer);
	pn.ParseFile(strPath, true);
}
