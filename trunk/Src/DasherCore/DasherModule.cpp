#include "DasherModule.h"

#include <iostream>

// Types:
// 0 = Input method
// 1 = Input filter

// IDs:
// 0 = Mouse input
// 1 = Socket input
// 2 = 1D mouse input
// 3 = Default filter
// 4 = 1D mode
// 5 = Eyetracker mode
// 6 = Dynamic Filter
// 7 = Click Mode
// 8 = Button mode (menu)
// 9 = One button filter (obsolete?)
// 10 = Button mode (static 3)
// 11 = Button mode (static 4)
// 12 = Button mode (alternating)
// 13 = Button mode (compass)
// 14 = Two button dynamic filter (Radford mode)
// 15 = Stylus input filter
// 16 = Joystick input
// 17 = Discrete joystick
// 18 = 1D joystick

CDasherModule::CDasherModule(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, ModuleID_t iID, int iType, const char *szName) 
  : CDasherComponent(pEventHandler, pSettingsStore) {
  m_iID = iID;
  m_iType = iType;
  m_iRefCount = 0;
  m_szName = szName;
}

ModuleID_t CDasherModule::GetID() {
  return m_iID;
}

int CDasherModule::GetType() {
  return m_iType;
}

const char *CDasherModule::GetName() {
  return m_szName;
}

void CDasherModule::Ref() {
  ++m_iRefCount;
}

void CDasherModule::Unref() {
  --m_iRefCount;

  if(m_iRefCount == 0)
    delete this;
}
