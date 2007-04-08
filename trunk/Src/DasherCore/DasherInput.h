// DasherInput.h
//
// Copyright (c) 2005 Phil Cowans

#ifndef __DasherInput_h__
#define __DasherInput_h__

#include "DasherTypes.h"
#include "DasherModule.h"

namespace Dasher {
  class CDasherInput;
  class CDasherInterfaceBase;
}
/// \defgroup Input Input devices
/// \{
class Dasher::CDasherInput : public CDasherModule {

public:

  CDasherInput(Dasher::CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, ModuleID_t iID, int iType, const char *szName) 
    : CDasherModule(pEventHandler, pSettingsStore, iID, iType, szName) {};

  /// Set the maximum values for each of the coordinates. Minimum
  /// values are assumed to be zero for now
  virtual void SetMaxCoordinates(int iN, myint * iDasherMax) {};

  /// Fill pCoordinates with iN coordinate values, return 0 if the
  /// values were in screen coordinates or 1 if the values were in
  /// Dasher coordinates.
  virtual int GetCoordinates(int iN, myint * pCoordinates) = 0;

  /// Get the number of co-ordinates that this device supplies
  ///
  virtual int GetCoordinateCount() = 0;
  
  /// Activate the device. If a helper thread needs to be started in
  /// order to listen for input then do it here.
  virtual void Activate() {};

  /// Deactivate the device. Please don't hold on to any significant
  /// resources (eg helper threads) after deactivation.
  virtual void Deactivate() {};

  /// Handle key down events
  ///
  virtual void KeyDown(int iTime, int iId) {};

  ///
  /// Handle key up events
  virtual void KeyUp(int iTime, int iId) {};
};
/// \}
#endif
