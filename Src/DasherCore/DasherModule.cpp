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

CDasherModule::CDasherModule(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, long long int iID, int iType) 
  : CDasherComponent(pEventHandler, pSettingsStore) {
  m_iID = iID;
  m_iType = iType;
  m_iRefCount = 0;
}

long long int CDasherModule::GetID() {
  return m_iID;
}

int CDasherModule::GetType() {
  return m_iType;
}

void CDasherModule::Ref() {
  ++m_iRefCount;
}

void CDasherModule::Unref() {
  --m_iRefCount;

  if(m_iRefCount == 0)
    delete this;
}
