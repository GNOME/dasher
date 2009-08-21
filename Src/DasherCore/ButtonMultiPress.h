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

#include "DynamicFilter.h"

namespace Dasher {
/// \ingroup InputFilter
/// @{
///DynamicFilter which detects multiple presses of the same button occurring in a short space of time
/// (such multi-presses are then passed onto the standard 'ActionButton' method, with iType equal
/// to the number of presses, for subclasses to handle/decide how to respond.)
class CButtonMultiPress : public CDynamicFilter {
 public:
  CButtonMultiPress(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, int iType, const char *szName);
  
  virtual void KeyDown(int iTime, int iId, CDasherView *pView, CDasherModel *pModel, CUserLogBase *pUserLog);

 protected:
  virtual unsigned int maxClickCount()=0;
  virtual void reverse();
  virtual void pause();
  virtual void run();

 private:
  virtual void RevertPresses(int iCount) {};

  int m_iQueueId;
  std::deque<int> m_deQueueTimes;
 };
}

#endif
