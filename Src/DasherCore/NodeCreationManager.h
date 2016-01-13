#ifndef __NodeCreationManager_h__
#define __NodeCreationManager_h__

#include "Alphabet/AlphabetMap.h"
#include "Alphabet/AlphIO.h"
#include "AlphabetManager.h"
#include "ConversionManager.h"
#include "ControlManager.h"
#include "LanguageModelling/LanguageModel.h"
#include "Trainer.h"
#include "Event.h"
#include "SettingsStore.h"

#include <string>
#include <vector>

namespace Dasher {
  class CDasherNode;
  class CDasherInterfaceBase;
  class CControlManager;
  class CDasherScreen;
  class CControlBoxIO;
}
//TODO why is CNodeCreationManager _not_ in namespace Dasher?!?!
/// \ingroup Model
/// @{
class CNodeCreationManager : public Dasher::CSettingsUserObserver {
 public:
  CNodeCreationManager(Dasher::CSettingsUser *pCreateFrom,
                       Dasher::CDasherInterfaceBase *pInterface,
                       const Dasher::CAlphIO *pAlphIO,
                       const Dasher::CControlBoxIO *pControlBoxIO);
  ~CNodeCreationManager();
  
  ///Tells us the screen on which all created node labels must be rendered
  void ChangeScreen(Dasher::CDasherScreen *pScreen);
  
  ///Create/ or not Control Manager, as appropriate (according to
  /// BP_CONTROL_MODE and game mode status)
  void CreateControlBox(const Dasher::CControlBoxIO* pControlIO);

  void HandleEvent(int iParameter) {}
  ///
  /// Get a root node of a particular type
  ///

  Dasher::CAlphabetManager *GetAlphabetManager() {return m_pAlphabetManager;}

  Dasher::CControlManager *GetControlManager() {return m_pControlManager;}
  
  ///
  /// Get a reference to the current alphabet
  ///

  const Dasher::CAlphInfo *GetAlphabet() const {
    return m_pAlphabetManager->GetAlphabet();
  }

  void ImportTrainingText(const std::string &strPath);

  unsigned long GetAlphNodeNormalization() {return m_iAlphNorm;}
  
  ///Called to add any non-alphabet (non-symbol) children to a top-level node (root or symbol).
  /// Default is just to add the control node, if appropriate.
  void AddExtras(Dasher::CDasherNode *pParent);
 private:
  Dasher::CTrainer *m_pTrainer;
  
  Dasher::CDasherInterfaceBase *m_pInterface;
  
  Dasher::CAlphabetManager *m_pAlphabetManager;
  Dasher::CControlManager *m_pControlManager;
  
  ///Amount of probability space to assign to letters (language model + smoothing),
  /// i.e. remaining after taking away whatever we need for control mode (perhaps 0)
  unsigned long m_iAlphNorm;
  
  ///Screen to use to create node labels
  Dasher::CDasherScreen *m_pScreen;
};
/// @}

#endif
