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
  
  //(ACL) Modify AlphabetManager for Mandarin Dasher
  if (oAlphInfo.m_iConversionID == 2)
    m_pAlphabetManager = new CMandarinAlphMgr(pInterface, this, m_pLanguageModel);
  else
    m_pAlphabetManager = new CAlphabetManager(pInterface, this, m_pLanguageModel);

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
  
  switch(oAlphInfo.m_iConversionID) {
    default:
      //TODO: Error reporting here
      //fall through to
    case 0: // No conversion required
      m_pConversionManager = new CConversionManager(this, m_pAlphabet);
      //ACL no, not quite - ConvMgrFac would always be created, so (ConvMgrFac==NULL) would always fail; but then segfault on ConvMgr->GetRoot() ?
      break;
#ifdef JAPANESE
    case 1: // Japanese
#ifdef WIN32
      m_pConversionManager = new CIMEConversionHelper;
#else
      m_pConversionManager = new CCannaConversionHelper(this, m_pAlphabet, GetLongParameter(LP_CONVERSION_TYPE), GetLongParameter(LP_CONVERSION_ORDER));
#endif
      break;
#endif
  }
}

CNodeCreationManager::~CNodeCreationManager() {
  delete m_pAlphabetManager;
  delete m_pTrainer;
  
  delete m_pControlManager;

  if (m_pConversionManager) m_pConversionManager->Unref();
}

CDasherNode *CNodeCreationManager::GetConvRoot(Dasher::CDasherNode *pParent, unsigned int iLower, unsigned int iUpper, int iOffset) { 
 if(m_pConversionManager)
   return m_pConversionManager->GetRoot(pParent, iLower, iUpper, iOffset);
 return NULL;
}

void CNodeCreationManager::GetProbs(CLanguageModel::Context context, std::vector <unsigned int >&Probs, int iNorm) const {
  // Total number of symbols
  int iSymbols = m_pAlphabet->GetNumberSymbols();      // note that this includes the control node and the root node
  
  // Number of text symbols, for which the language model gives the distribution
  // int iTextSymbols = m_pAlphabet->GetNumberTextSymbols();
  
  // TODO - sort out size of control node - for the timebeing I'll fix the control node at 5%
  // TODO: New method (see commented code) has been removed as it wasn' working.

  int uniform_add;
  int nonuniform_norm;
  int control_space = 0;
  int uniform = GetLongParameter(LP_UNIFORM);

   if(!GetBoolParameter(BP_CONTROL_MODE)) {
     control_space = 0;
     uniform_add = ((iNorm * uniform) / 1000) / (iSymbols - 2);  // Subtract 2 from no symbols to lose control/root nodes
     nonuniform_norm = iNorm - (iSymbols - 2) * uniform_add;
   }
   else {
     control_space = int (iNorm * 0.05);
     uniform_add = (((iNorm - control_space) * uniform / 1000) / (iSymbols - 2));        // Subtract 2 from no symbols to lose control/root nodes
     nonuniform_norm = iNorm - control_space - (iSymbols - 2) * uniform_add;
   }
   
   //  m_pLanguageModel->GetProbs(context, Probs, iNorm, ((iNorm * uniform) / 1000));
   
   //ACL used to test explicitly for MandarinDasher and if so called GetPYProbs instead
   // (by statically casting to PPMPYLanguageModel). However, have renamed PPMPYLanguageModel::GetPYProbs
   // to GetProbs as per ordinary language model, so no need to test....
   m_pLanguageModel->GetProbs(context, Probs, iNorm, 0);

   // #if _DEBUG
   //int iTotal = 0;
   //for(int k = 0; k < Probs.size(); ++k)
     //     iTotal += Probs[k];
   //   DASHER_ASSERT(iTotal == nonuniform_norm);
// #endif

//   //  Probs.insert(Probs.begin(), 0);

   for(unsigned int k(1); k < Probs.size(); ++k)
     Probs[k] += uniform_add;

  Probs.push_back(control_space);

#if _DEBUG
  int iTotal = 0;
  for(int k = 0; k < Probs.size(); ++k)
    iTotal += Probs[k];
  DASHER_ASSERT(iTotal == iNorm);
#endif

}

void CNodeCreationManager::HandleEvent(CEvent *pEvent) {
  if (pEvent->m_iEventType == EV_PARAM_NOTIFY) {
    if (static_cast<CParameterNotificationEvent *>(pEvent)->m_iParameter == BP_CONTROL_MODE) {
      delete m_pControlManager;
      m_pControlManager = (GetBoolParameter(BP_CONTROL_MODE))
        ? new CControlManager(m_pEventHandler, m_pSettingsStore, this, m_pInterface)
        : NULL;
    } 
  }
}

void 
CNodeCreationManager::ImportTrainingText(const std::string &strPath) {
	m_pTrainer->LoadFile(strPath);
}
