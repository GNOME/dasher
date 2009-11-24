#ifndef __NodeCreationManager_h__
#define __NodeCreationManager_h__

#include "Alphabet/Alphabet.h"
#include "Alphabet/AlphIO.h"
#include "AlphabetManager.h"
#include "ConversionManager.h"
#include "DasherComponent.h"
#include "LanguageModelling/LanguageModel.h"
#include "Trainer.h"

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
  /// Get a root node of a particular type
  ///

  Dasher::CDasherNode *GetAlphRoot(Dasher::CDasherNode *pParent, int iLower, int iUpper, bool bEnteredLast, int iOffset);
  Dasher::CDasherNode *GetCtrlRoot(Dasher::CDasherNode *pParent, int iLower, int iUpper, int iOffset);
  Dasher::CDasherNode *GetConvRoot(Dasher::CDasherNode *pParent, int iLower, int iUpper, int iOffset);

  ///
  /// Register a control node
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

  void GetProbs(Dasher::CLanguageModel::Context context, std::vector <unsigned int >&Probs, int iNorm) const;

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
  Dasher::CTrainer *m_pTrainer;
  
  int m_iConversionID;
  
  Dasher::CAlphabetManager *m_pAlphabetManager;
  Dasher::CControlManager *m_pControlManager;
  Dasher::CConversionManager *m_pConversionManager;
};
/// @}

#endif
