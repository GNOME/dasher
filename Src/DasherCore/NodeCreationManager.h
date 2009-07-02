#ifndef __NodeCreationManager_h__
#define __NodeCreationManager_h__

#include "Alphabet/Alphabet.h"
#include "Alphabet/AlphIO.h"
#include "AlphabetManagerFactory.h"
#include "ConversionManagerFactory.h"
#include "DasherComponent.h"

#include "LanguageModelling/LanguageModel.h"

#include <string>
#include <vector>

namespace Dasher {
  class CDasherNode;
  class CDasherInterfaceBase;
  class CControlManager;
};
//TODO why is CNodeCreationManager _not_ in namespace Dasher?!?!
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

  Dasher::CDasherNode *GetRoot(int iType, Dasher::CDasherNode *pParent, int iLower, int iUpper, void *pUserData);

  ///
  /// Resgister a control node 
  ///

  void RegisterNode( int iID, const std::string &strLabel, int iColour );

  ///
  /// Connect control nodes in the tree
  ///
  
  void ConnectNode(int iChild, int iParent, int iAfter);

  ///
  /// Disconnect control nodes
  ///

  void DisconnectNode(int iChild, int iParent);

  ///
  /// Temporary methods: TODO: remove
  ///

  Dasher::CLanguageModel *GetLanguageModel() {
    return m_pLanguageModel;
  };

  Dasher::CLanguageModel::Context GetLearnContext() {
    return m_pAlphabetManagerFactory->GetLearnContext();
  }

  void GetProbs(Dasher::CLanguageModel::Context context, std::vector <Dasher::symbol >&NewSymbols, std::vector <unsigned int >&Probs, int iNorm) const;
  void LearnText(Dasher::CLanguageModel::Context context, std::string *TheText, bool IsMore);
  void EnterText(Dasher::CLanguageModel::Context context, std::string TheText) const;

  inline int GetColour(Dasher::symbol s, int iPhase) const {
    return m_pAlphabet->GetColour(s, iPhase); 
  };

  ///
  /// Get the symbol ID representing space
  ///

  Dasher::symbol GetSpaceSymbol() const {
    return m_pAlphabet->GetSpaceSymbol();
  }

  ///
  /// Get the symbol ID representing the control node 
  ///

  Dasher::symbol GetControlSymbol() const {
    return m_pAlphabet->GetControlSymbol();
  }

  ///
  /// Get the symbol ID representing the conversion pseudo-character
  ///

  Dasher::symbol GetStartConversionSymbol() const {
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

  Dasher::CAlphabet *GetAlphabet() {
    return m_pAlphabet;
  }

  void ImportTrainingText(const std::string &strPath);

 private:
  Dasher::CLanguageModel *m_pLanguageModel;     // pointer to the language model
  Dasher::CAlphabet *m_pAlphabet;        // pointer to the alphabet

  Dasher::CAlphabetManagerFactory *m_pAlphabetManagerFactory;
  Dasher::CControlManager *m_pControlManager;
  Dasher::CConversionManagerFactory *m_pConversionManagerFactory;
};
/// @}

#endif
