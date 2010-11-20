// ModuleManager.cpp
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

#include <iostream>
#include <stdexcept>

#include "ModuleManager.h"
#include "DasherInput.h"
#include "InputFilter.h"

using namespace Dasher;

CDasherModule *CModuleManager::RegisterModule(CDasherModule *pModule) {
    m_vModules.push_back(pModule);
    ModuleID_t id = m_vModules.size() - 1;
    pModule->SetID(id);

    // This does
    //     m_mapNameToID[pModule->GetName()] = id;
    // but with error checking.
    std::pair<std::map<std::string, ModuleID_t>::iterator, bool> res;
    std::pair<const std::string, ModuleID_t> keyvalue(pModule->GetName(), id);
    res = m_mapNameToID.insert(keyvalue);
    if (!res.second)
        std::cerr << "Module \"" << pModule->GetName()
                  << "\" registered twice" << std::endl;

    return pModule;
}

CDasherModule *CModuleManager::GetModule(ModuleID_t iID) {
    // This does
    //     return m_vModules[iID];
    // but with error checking.
    try {
        return m_vModules.at(iID);
    }
    catch (std::out_of_range) {
        std::cerr << "Module with ID " << iID << " not found" << std::endl;
        return NULL;
    }
}

CDasherModule *CModuleManager::GetModuleByName(const std::string strName) {
    // This does
    //    return m_vModules[m_mapNameToID[strName]];
    // but with error checking.
    std::map<std::string, ModuleID_t>::iterator res;
    res = m_mapNameToID.find(strName);
    if (res == m_mapNameToID.end()) {
        std::cerr << "Module \"" << strName << "\" not registered" <<std::endl;
        return NULL;
    } else {
        return m_vModules[res->second];
    }
}

void CModuleManager::ListModules(int iType, std::vector<std::string> &vList) {
    for (ModuleID_t i = 0; i < m_vModules.size(); ++i) {
        if(m_vModules[i]->GetType() == iType)
            vList.push_back(m_vModules[i]->GetName());
    }
}

CModuleManager::~CModuleManager() {
/* XXX PRLW: Modules are Components. Components are registered
 * with an event handler on creation. When they are unregistered,
 * they are deleted, so this part is not necessary. One question
 * then is why are effectively maintaining two lists.
    for (ModuleID_t i = 0; i < m_vModules.size(); ++i) {
        delete m_vModules[i];
    }
 */
}

CDasherInput *CModuleManager::GetDefaultInputDevice() {
    return m_pDefaultInputDevice;
}

CInputFilter *CModuleManager::GetDefaultInputMethod() {
    return m_pDefaultInputMethod;
}

void CModuleManager::SetDefaultInputDevice(CDasherInput *p) {
  DASHER_ASSERT(p->GetType() == InputDevice);
  m_pDefaultInputDevice = p;
}

void CModuleManager::SetDefaultInputMethod(CInputFilter *p) {
  DASHER_ASSERT(p->GetType() == InputMethod);
  m_pDefaultInputMethod = p;
}
