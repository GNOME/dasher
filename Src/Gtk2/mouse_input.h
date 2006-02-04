#ifndef __mouse_input_h__
#define __mouse_input_h__

#include "../DasherCore/DasherInput.h"
#include "../DasherCore/DasherTypes.h"

#include <iostream>

using namespace Dasher;

class CDasherMouseInput : public CDasherInput {
public:
  CDasherMouseInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore) 
    : CDasherInput(pEventHandler, pSettingsStore, 0, 0) {
  };

  // Fill pCoordinates with iN coordinate values, return 0 if the
  // values were in screen coordinates or 1 if the values were in
  // Dasher coordinates.

  virtual int GetCoordinates(int iN, myint * pCoordinates) {

    pCoordinates[0] = m_iX;
    pCoordinates[1] = m_iY;

    return 0;
  };

  // Get the number of co-ordinates that this device supplies

  virtual int GetCoordinateCount() {
    return 2;
  };

  void SetCoordinates(myint _iX, myint _iY) {
    m_iX = _iX;
    m_iY = _iY;
  };

private:
  myint m_iX;
  myint m_iY;

};

class CDasher1DMouseInput:public CDasherInput {
public:
  CDasher1DMouseInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore) 
    : CDasherInput(pEventHandler, pSettingsStore, 2, 0) {
  };

  // Fill pCoordinates with iN coordinate values, return 0 if the
  // values were in screen coordinates or 1 if the values were in
  // Dasher coordinates.

  virtual int GetCoordinates(int iN, myint * pCoordinates) {

    pCoordinates[0] = m_iY * m_iDasherMaxY / 1024;      // FIXME - hard coded screen resolution!!!!!!!!!!

    std::cout << m_iY << " " << pCoordinates[0] << std::endl;

    return 0;
  };

  // Get the number of co-ordinates that this device supplies

  virtual int GetCoordinateCount() {
    return 1;
  };

  virtual void SetMaxCoordinates(int iN, myint * iDasherMax) {

    // FIXME - need to cope with the more general case here

    m_iDasherMaxX = iDasherMax[0];
    m_iDasherMaxY = iDasherMax[1];
  };

  void SetCoordinates(myint _iX, myint _iY) {
    m_iX = _iX;
    m_iY = _iY;
  };

private:

  myint m_iDasherMaxX;
  myint m_iDasherMaxY;

  myint m_iX;
  myint m_iY;

};

#endif
