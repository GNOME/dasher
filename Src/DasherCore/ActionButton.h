#ifndef __ActionButton_h__
#define __ActionButton_h__

#include "DasherScreen.h"

#include <string>

namespace Dasher {
  class CDasherInterfaceBase;
};

/// \defgroup ActionButtons Action Buttons
/// @{
class CActionButton {
 public:
  CActionButton(Dasher::CDasherInterfaceBase *pInterface, const std::string &strCommand, bool bAlwaysVisible);

  void SetPosition(int iX, int iY, int iWidth, int iHeight);
  void Draw(Dasher::CDasherScreen *pScreen, bool bVisible);
  bool HandleClickDown(int iTime, int iX, int iY, bool bVisible);
  bool HandleClickUp(int iTime, int iX, int iY, bool bVisible);
  void Execute(int iTime);
  
 private:
  std::string m_strCommand;

  bool m_bAlwaysVisible;
  
  int m_iX;
  int m_iY;
  int m_iWidth;
  int m_iHeight;

  Dasher::CDasherInterfaceBase *m_pInterface;
};
/// @}
#endif
