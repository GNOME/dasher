#ifndef __NodeCreationManager_h__
#define __NodeCreationManager_h__

#include "Alphabet/Alphabet.h"
#include "Alphabet/AlphIO.h"
#include "AlphabetManager.h"
#include "ConversionManager.h"
#include "ControlManager.h"
#include "DasherComponent.h"
#include "LanguageModelling/LanguageModel.h"
#include "Trainer.h"
#include "Event.h"

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
  
  //we watch for changes to BP_CONTROL_MODE and create the Control Manager lazily
  void HandleEvent(Dasher::CEvent *pEvent);
  ///
  /// Get a root node of a particular type
  ///

  Dasher::CAlphabetManager *GetAlphabetManager() {return m_pAlphabetManager;}

  Dasher::CControlManager *GetControlManager() {return m_pControlManager;}
  
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
  
  Dasher::CDasherInterfaceBase *m_pInterface;
  
  Dasher::CAlphabetManager *m_pAlphabetManager;
  Dasher::CControlManager *m_pControlManager;
  
  ///Probability to assign to control node (0 if control mode off)
  unsigned long m_iControlSpace;
  ///Amount of probability space remaining, i.e. for language model to assign to letters:
  unsigned long m_iNonUniformNorm;
  ///Amount of probability space we will add to _each_ letter (on top of fraction of
  /// m_iNonUniformNorm) to effect smoothing/uniformity:
  unsigned long m_iUniformAdd;
};
/// @}

#endif
