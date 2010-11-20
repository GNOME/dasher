// ModuleManager.h
//
// Copyright (c) 2008 The Dasher Team
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

#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include "DasherModule.h"

#include <map>
#include <string>
#include <vector>

namespace Dasher {
  class CDasherInput;
  class CInputFilter;

enum ModuleType {InputDevice = 0, InputMethod = 1};

/// \ingroup Core
/// \{
class CModuleManager {
 public:
  ~CModuleManager();
  CDasherModule *RegisterModule(CDasherModule *pModule);
  CDasherModule *GetModule(ModuleID_t iID);
  CDasherModule *GetModuleByName(const std::string strName);
  CDasherInput *GetDefaultInputDevice();
  CInputFilter *GetDefaultInputMethod();
  void SetDefaultInputDevice(CDasherInput *);
  void SetDefaultInputMethod(CInputFilter *);

  void ListModules(int iType, std::vector<std::string> &vList);

 private:
  std::vector<CDasherModule *> m_vModules;
  std::map<std::string, ModuleID_t> m_mapNameToID;
  CDasherInput *m_pDefaultInputDevice;
  CInputFilter *m_pDefaultInputMethod;
};
/// \}

}

#endif
