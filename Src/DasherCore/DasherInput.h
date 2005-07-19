// DasherInput.h
//
// Copyright (c) 2005 Phil Cowans

#ifndef __DasherInput_h__
#define __DasherInput_h__

#include "DasherTypes.h"

namespace Dasher {
  class CDasherInput;
} class Dasher::CDasherInput {
public:

  virtual void SetMaxCoordinates(int iN, myint * iDasherMax) {
  };

  // Fill pCoordinates with iN coordinate values, return 0 if the
  // values were in screen coordinates or 1 if the values were in
  // Dasher coordinates.

  virtual int GetCoordinates(int iN, myint * pCoordinates) = 0;

  // Get the number of co-ordinates that this device supplies

  virtual int GetCoordinateCount() = 0;
};

#endif
