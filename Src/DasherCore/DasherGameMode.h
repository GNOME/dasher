#ifndef __DasherGameMode_h__
#define __DasherGameMode_h__

#include "../Common/NoClones.h"
#include "LanguageModelling/LanguageModel.h"
#include "DasherComponent.h"
#include "Alphabet/Alphabet.h"
#include "AlphabetManagerFactory.h"
#include "ControlManagerFactory.h"
#include <math.h>
#include "DasherTypes.h"
#include "FrameRate.h"
#include <vector>
#include <deque>

namespace Dasher {
  class CDasherModel;
  class CDasherInterfaceBase;
  class CEventHandler;
  class CDasherNode;
  class CDasherGameMode;
  class CDasherComponent;
}

class Dasher::CDasherGameMode:public CDasherComponent, private NoClones {
public:
  CDasherGameMode(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase * pDashIface, CDasherModel * model);

  // Returns Dasher Coordinate of the target string
  // myint.Max() reserved for "off the screen to the bottom"
  // myint.Min()+1 reserved for "off the screen to the top"
  // myint.Min() reserved for "nothing to draw" (string finished or error)
  myint GetDasherCoordOfTarget();

  void HandleEvent(Dasher::CEvent *) {
    return;
  }
private:
  // loads .txt file with strings for current alphabet
  myint InitializeTargetFile();

  // sets CurrentTarget to a new string from the vector of choices
  myint GetNextTargetString();

  // clears vector of choices and sets currenttarget to an empty string
  void Reset();

  CDasherModel *m_model;
  std::vector < std::string > TargetStrings;
  CDasherInterfaceBase *m_DasherInterface;
  std::string CurrentTarget;
};

#endif
