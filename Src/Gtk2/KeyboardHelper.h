#ifndef __KEYBOARD_HELPER_H__
#define __KEYBOARD_HELPER_H__

//#include "../DasherCore/DasherInterfaceBase.h"

#include "DasherAppSettings.h"

#include <map>

class CKeyboardHelper {
 public:
  CKeyboardHelper(DasherAppSettings *pAppSettings);
  void SetKey(int iCode, int iId);
  void UnsetKey(int iCode);

  int ConvertKeycode(int iCode);
  // unused: void Grab(bool bGrab);

 private:
  void SetupDefaults();
  void LoadSettings();
  void SaveSettings();

  std::map<int, int> m_mTable;

  //  CDasherInterfaceBase *m_pInterface;
  DasherAppSettings *m_pAppSettings;
};

#endif
