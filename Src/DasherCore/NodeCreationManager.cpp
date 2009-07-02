#include "DasherNode.h"
#include "DasherInterfaceBase.h"
#include "LanguageModelling/PPMLanguageModel.h"
#include "LanguageModelling/WordLanguageModel.h"
#include "LanguageModelling/DictLanguageModel.h"
#include "LanguageModelling/MixtureLanguageModel.h"
#include "LanguageModelling/PPMPYLanguageModel.h"
#include "NodeCreationManager.h"
#include "ControlManager.h"

CNodeCreationManager::CNodeCreationManager(Dasher::CDasherInterfaceBase *pInterface,
					   Dasher::CEventHandler *pEventHandler, 
					   CSettingsStore *pSettingsStore,
					   Dasher::CAlphIO *pAlphIO) : CDasherComponent(pEventHandler, pSettingsStore) {

  m_pAlphabetManagerFactory = new CAlphabetManagerFactory(pInterface,
							  pEventHandler,
							  pSettingsStore,
							  pAlphIO,
							  this);
  
  m_pLanguageModel = m_pAlphabetManagerFactory->GetLanguageModel();
  m_pAlphabet = m_pAlphabetManagerFactory->GetAlphabet();

  int iConversionID(m_pAlphabetManagerFactory->GetConversionID());

  // Train the language model
  CTrainer *pTrainer =  m_pAlphabetManagerFactory->GetTrainer();

  //WZ: Mandarin Dasher Change
  if((iConversionID==2)&&(pSettingsStore->GetStringParameter(SP_ALPHABET_ID)=="Chinese Super Pin Yin, grouped by Dictionary"))
    pTrainer->TrainMandarin(GetStringParameter(SP_USER_LOC), GetStringParameter(SP_SYSTEM_LOC));
  else{
    //End Mandarin Dasher Change  
    m_pAlphabet->Train(GetStringParameter(SP_USER_LOC), GetStringParameter(SP_SYSTEM_LOC), pTrainer);
  }
  delete pTrainer;



#ifndef _WIN32_WCE
  m_pControlManager = new CControlManager(this);
#else
  m_pControlManager = 0;
#endif
  m_pConversionManagerFactory = new CConversionManagerFactory(pEventHandler,
							      pSettingsStore,
							      this,
							      iConversionID,
							      pAlphIO,
							      m_pAlphabet);
}

CNodeCreationManager::~CNodeCreationManager() {

  // C++ standard dictates that
  // delete NULL;
  // is totally safe, and does nothing. Do we need all these if statements?

  if(m_pAlphabetManagerFactory)
    delete m_pAlphabetManagerFactory;
  
  if(m_pControlManager)
    delete m_pControlManager;

  if(m_pConversionManagerFactory)
    delete m_pConversionManagerFactory;
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

CDasherNode *CNodeCreationManager::GetRoot(int iType, Dasher::CDasherNode *pParent, int iLower, int iUpper, void *pUserData ) {

  switch(iType) {
  case 0:
    return m_pAlphabetManagerFactory->GetRoot(pParent, iLower, iUpper, pUserData);
  case 1:
    if(m_pControlManager)
      return m_pControlManager->GetRoot(pParent, iLower, iUpper, pUserData);
    else
      return NULL;
  case 2:
    if(m_pConversionManagerFactory)
      return m_pConversionManagerFactory->GetRoot(pParent, iLower, iUpper, pUserData);
    else
      return NULL;
  default:
    return NULL;
  }
}


void CNodeCreationManager::GetProbs(CLanguageModel::Context context, std::vector <symbol >&NewSymbols, std::vector <unsigned int >&Probs, int iNorm) const {
  // Total number of symbols
  int iSymbols = m_pAlphabet->GetNumberSymbols();      // note that this includes the control node and the root node
  
  // Number of text symbols, for which the language model gives the distribution
  // int iTextSymbols = m_pAlphabet->GetNumberTextSymbols();
  
  NewSymbols.resize(iSymbols);
//      Groups.resize(iSymbols);
  for(int i = 0; i < iSymbols; i++) {
    NewSymbols[i] = i;          // This will be replaced by something that works out valid nodes for this context
    //      Groups[i]=m_pAlphabet->get_group(i);
  }

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
   if(m_pAlphabetManagerFactory->GetConversionID()==2){
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

void CNodeCreationManager::LearnText(CLanguageModel::Context context, std::string *TheText, bool IsMore) {
  std::vector < symbol > Symbols;

  m_pAlphabet->GetSymbols(&Symbols, TheText, IsMore);

  for(unsigned int i = 0; i < Symbols.size(); i++)
    m_pLanguageModel->LearnSymbol(context, Symbols[i]); // FIXME - conversion to symbol alphabet
}

void CNodeCreationManager::EnterText(CLanguageModel::Context context, std::string TheText) const {
  std::vector < symbol > Symbols;
  m_pAlphabet->GetSymbols(&Symbols, &TheText, false);
  for(unsigned int i = 0; i < Symbols.size(); i++)
    m_pLanguageModel->EnterSymbol(context, Symbols[i]); // FIXME - conversion to symbol alphabet
}

void 
CNodeCreationManager::ImportTrainingText(const std::string &strPath) {
  CTrainer *pTrainer = NULL;

  if(m_pAlphabetManagerFactory)
    pTrainer = m_pAlphabetManagerFactory->GetTrainer();

  if(m_pAlphabet && pTrainer)
    m_pAlphabet->Train(strPath, pTrainer);

  delete pTrainer;
}
