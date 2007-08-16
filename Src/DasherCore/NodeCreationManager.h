#ifndef __NodeCreationManager_h__
#define __NodeCreationManager_h__

#include "Alphabet/Alphabet.h"
#include "Alphabet/AlphIO.h"
#include "AlphabetManagerFactory.h"
#include "ControlManagerFactory.h"
#include "ConversionManagerFactory.h"
#include "DasherComponent.h"

#include "LanguageModelling/LanguageModel.h"

#include <string>
#include <vector>

namespace Dasher {
  class CDasherNode;
  class CDasherInterfaceBase;
};
/// \ingroup Model
/// @{
class CNodeCreationManager : public Dasher::CDasherComponent {
 public:
  CNodeCreationManager(Dasher::CDasherInterfaceBase *pInterface,
		       Dasher::CEventHandler * pEventHandler,
		       CSettingsStore * pSettingsStore,
		       Dasher::CAlphIO *pAlphIO);
  ~CNodeCreationManager();
  
  ///
  /// Get a root node of a given type
  ///

  CDasherNode *GetRoot(int iType, Dasher::CDasherNode *pParent, int iLower, int iUpper, void *pUserData);

  ///
  /// Resgister a control node 
  ///

  void RegisterNode( int iID, const std::string &strLabel, int iColour ) {
    if(m_pControlManagerFactory)
      m_pControlManagerFactory->RegisterNode(iID, strLabel, iColour);
  }

  ///
  /// Connect control nodes in the tree
  ///
  
  void ConnectNode(int iChild, int iParent, int iAfter) {
    if(m_pControlManagerFactory)
      m_pControlManagerFactory->ConnectNode(iChild, iParent, iAfter);
  }

  ///
  /// Disconnect control nodes
  ///

  void DisconnectNode(int iChild, int iParent) {
    if(m_pControlManagerFactory)
      m_pControlManagerFactory->DisconnectNode(iChild, iParent);
  }

  ///
  /// Temporary methods: TODO: remove
  ///

  CLanguageModel *GetLanguageModel() {
    return m_pLanguageModel;
  };

  CLanguageModel::Context GetLearnContext() {
    return m_pAlphabetManagerFactory->GetLearnContext();
  }

  void GetProbs(CLanguageModel::Context context, std::vector <symbol >&NewSymbols, std::vector <unsigned int >&Probs, int iNorm) const;
  void LearnText(CLanguageModel::Context context, std::string *TheText, bool IsMore);
  void EnterText(CLanguageModel::Context context, std::string TheText) const;

  inline int GetColour(symbol s) const { 
    return m_pAlphabet->GetColour(s); 
  };

  ///
  /// Get the symbol ID representing space
  ///

  symbol GetSpaceSymbol() const {
    return m_pAlphabet->GetSpaceSymbol();
  }

  ///
  /// Get the symbol ID representing the control node 
  ///

  symbol GetControlSymbol() const {
    return m_pAlphabet->GetControlSymbol();
  }

  ///
  /// Get the symbol ID representing the conversion pseudo-character
  ///

  symbol GetStartConversionSymbol() const {
    return m_pAlphabet->GetStartConversionSymbol();
  }

  ///
  /// Convert a given symbol ID to display text
  ///

  const std::string & GetDisplayText(int iSymbol) const {
    return m_pAlphabet->GetDisplayText(iSymbol);
  }

  ///
  /// Get a reference to the alphabet
  ///

  CAlphabet *GetAlphabet() {
    return m_pAlphabet;
  }

  ///
  /// Get a pointer to the alphabet - TODO: Remove duplicate
  /// functionality with the above function
  ///

/*   CAlphabet *GetAlphabet() const { */
/*     return m_pAlphabet; */
/*   } */

  CAlphabetManagerFactory::CTrainer *GetTrainer();

 private:
  CLanguageModel *m_pLanguageModel;     // pointer to the language model
  CAlphabet *m_pAlphabet;        // pointer to the alphabet

  CAlphabetManagerFactory *m_pAlphabetManagerFactory;
  CControlManagerFactory *m_pControlManagerFactory;
  CConversionManagerFactory *m_pConversionManagerFactory;
};
/// @}

#endif
