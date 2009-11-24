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
#include "PinYinConversionHelper.h"
#include "ControlManager.h"
#include "EventHandler.h"

using namespace Dasher;

CNodeCreationManager::CNodeCreationManager(Dasher::CDasherInterfaceBase *pInterface,
					   Dasher::CEventHandler *pEventHandler, 
					   CSettingsStore *pSettingsStore,
					   Dasher::CAlphIO *pAlphIO) : CDasherComponent(pEventHandler, pSettingsStore) {

  const Dasher::CAlphIO::AlphInfo &oAlphInfo(pAlphIO->GetInfo(pSettingsStore->GetStringParameter(SP_ALPHABET_ID)));
  m_pAlphabet = new CAlphabet(oAlphInfo);
  
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
    CAlphabet *pCHAlphabet = new CAlphabet(oCHAlphInfo);
    
    CSymbolAlphabet chalphabet(pCHAlphabet->GetNumberTextSymbols());
    chalphabet.SetSpaceSymbol(pCHAlphabet->GetSpaceSymbol());
    chalphabet.SetAlphabetPointer(pCHAlphabet);
    //std::cout<<"CHALphabet size "<<chalphabet.GetSize(); [7603]
    m_pLanguageModel = new CPPMPYLanguageModel(pEventHandler, pSettingsStore, chalphabet, alphabet);
    m_pTrainer = new CMandarinTrainer(m_pLanguageModel, m_pAlphabet, pCHAlphabet);
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
    
  // TODO: Tell the alphabet manager about the alphabet here, so we
  // don't end up having to duck out to the NCM all the time
  
  //(ACL) Modify AlphabetManager for Mandarin Dasher
  if (m_iConversionID == 2)
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
    CPPMLanguageModel *pLM2 = new CPPMLanguageModel(pEventHandler, pSettingsStore, pLan->SymbolAlphabet());
    pLM2->ReadFromFile("test.model");
    if (!pLan->eq(pLM2)) {
      std::cout << "Not equal!" << std::endl;
      pLM2->WriteToFile("test2.model");
    }
    delete pLM2;
  }
#endif
#ifndef _WIN32_WCE
  m_pControlManager = new CControlManager(this);
#else
  m_pControlManager = 0;
#endif
  
  switch(m_iConversionID) {
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
#ifdef CHINESE
    case 2: // Chinese
      m_pConversionManager = new CPinYinConversionHelper(this, m_pEventHandler, m_pSettingsStore, pAlphIO,
                                         GetStringParameter(SP_SYSTEM_LOC) +"/alphabet.chineseRuby.xml",
                                         m_pAlphabet, static_cast<CPPMPYLanguageModel *>(m_pLanguageModel));
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

void CNodeCreationManager::RegisterNode( int iID, const std::string &strLabel, int iColour ) {
  if(m_pControlManager)
    m_pControlManager->RegisterNode(iID, strLabel, iColour);
}

void CNodeCreationManager::ConnectNode(int iChild, int iParent, int iAfter) {
  if(m_pControlManager)
    m_pControlManager->ConnectNode(iChild, iParent, iAfter);
}

void CNodeCreationManager::DisconnectNode(int iChild, int iParent) {
  if(m_pControlManager)
    m_pControlManager->DisconnectNode(iChild, iParent);
}

CDasherNode *CNodeCreationManager::GetAlphRoot(Dasher::CDasherNode *pParent, int iLower, int iUpper, bool bEnteredLast, int iOffset) { 
 return m_pAlphabetManager->GetRoot(pParent, iLower, iUpper, bEnteredLast, iOffset);
}

CDasherNode *CNodeCreationManager::GetCtrlRoot(Dasher::CDasherNode *pParent, int iLower, int iUpper, int iOffset) { 
 if(m_pControlManager)
 return m_pControlManager->GetRoot(pParent, iLower, iUpper, iOffset);
 else
 return NULL;
}

CDasherNode *CNodeCreationManager::GetConvRoot(Dasher::CDasherNode *pParent, int iLower, int iUpper, int iOffset) { 
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
   
   //WZ: Mandarin Dasher Change
   if(m_iConversionID==2){
     //Mark: static cast ok?
     static_cast<CPPMPYLanguageModel *>(m_pLanguageModel)->GetPYProbs(context, Probs, iNorm, 0);
   }
   else
     //End Mandarin Dasher Change
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

void 
CNodeCreationManager::ImportTrainingText(const std::string &strPath) {
	m_pTrainer->LoadFile(strPath);
}
