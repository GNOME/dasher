// DynamicFilter.h
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

#ifndef __ButtonMultiPress_h__
#define __ButtonMultiPress_h__

#include "DynamicButtons.h"

namespace Dasher {
/// \ingroup InputFilter
/// @{
/// DynamicButtons filter which detects long and multiple presses - the latter of the
/// same button, up to maxClickCount() consecutive presses,
/// with a gap of up to LP_MULTIPRESS_TIME ms between the start of _each_pair_ of 
/// presses. Long- and multi-presses are passed onto the standard ButtonEvent method,
/// with iType 1 or to the number of presses, respectively, for subclasses to decide how to respond.
class CButtonMultiPress : public CDynamicButtons {
 public:
  CButtonMultiPress(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, const char *szName);

  void Timer(unsigned long iTime, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CExpansionPolicy **pol);
  void KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);

  void pause();
 protected:
  virtual unsigned int maxClickCount()=0;
  void reverse(unsigned long iTime);
  void run(unsigned long iTime);

 private:
  virtual void RevertPresses(int iCount) {};

  int m_iQueueId;
  std::deque<unsigned long> m_deQueueTimes;

  ///Whether a long-press has been handled (in Timer) - as the key
  /// may still be down (and the press becoming ever-longer)!
  bool m_bKeyHandled;
  unsigned long m_iKeyDownTime;
 };
}

#endif
