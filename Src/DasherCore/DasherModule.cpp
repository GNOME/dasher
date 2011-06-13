// DasherModule.cpp
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

#include "DasherModule.h"

#include <iostream>

// Types:
// 0 = Input method
// 1 = Input filter

CDasherModule::CDasherModule(ModuleID_t iID, int iType, const char *szName) {
  m_iID = iID;
  m_iType = iType;
  m_szName = szName;
}

ModuleID_t CDasherModule::GetID() {
  return m_iID;
}

void CDasherModule::SetID(ModuleID_t id) {
  m_iID = id;
}

int CDasherModule::GetType() {
  return m_iType;
}

const char *CDasherModule::GetName() {
  return m_szName;
}
