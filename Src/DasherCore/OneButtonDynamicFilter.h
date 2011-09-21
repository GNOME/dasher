// OneButtonDynamicFilter.h
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __ONE_BUTTON_DYNAMIC_FILTER_H__
#define __ONE_BUTTON_DYNAMIC_FILTER_H__

#include "ButtonMultiPress.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
class COneButtonDynamicFilter : public CButtonMultiPress {
 public:
  COneButtonDynamicFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate);
  ~COneButtonDynamicFilter();

  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput);

  virtual bool GetSettings(SModuleSettings **pSettings, int *iCount);

  //override to get mouse clicks / taps back again...
  virtual void KeyDown(unsigned long Time, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  virtual void KeyUp(unsigned long Time, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);

 private:
  unsigned int maxClickCount() {return 2;} //double-click to reverse
  virtual void TimerImpl(unsigned long Time, CDasherView *pView, CDasherModel *m_pDasherModel, CExpansionPolicy **pol);
  virtual void ActionButton(unsigned long iTime, int iButton, int iType, CDasherModel *pModel);
  
  int m_iTarget;

  int m_iTargetX[2];
  int m_iTargetY[2];
};
}
/// @}

#endif
