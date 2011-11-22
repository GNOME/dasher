/*
 *  IPhoneFilters.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 29/04/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#include "OneDimensionalFilter.h"
#include "StylusFilter.h"
#include "IPhoneInputs.h"

#ifndef __IPHONE_FILTERS_MM__
extern NSString *HOLD_TO_GO;
extern NSString *TILT_1D;
extern NSString *TILT_USE_TOUCH_X;
extern NSString *TOUCH_USE_TILT_X;
extern NSString *DOUBLE_TOUCH_X;
#endif

#define TILT_FILTER "IPhone Tilt Filter"
#define TOUCH_FILTER "IPhone Touch Filter"
#define TWO_FINGER_FILTER "Two-finger filter"

namespace Dasher {

class CIPhoneTiltFilter : public COneDimensionalFilter, private IPhonePrefsObserver {
public:
	CIPhoneTiltFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, CDasherInput *pTouch);
  ///override to enable hold-to-go
	virtual void KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  ///override to enable hold-to-go
	virtual void KeyUp(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);

  virtual void pause(); ///Override to re-enable idle timer
  void iPhonePrefsChanged(NSString *key);
  bool supportsPause();
protected:
  virtual void run(unsigned long iTime); ///Override to disable idle timer
  ///Override to choose whether to apply 1D transform or not, and to get X coord from touch if appropriate
	virtual void ApplyTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView);
  ///Never apply offset (just eyetracker remapping!) - otherwise would be done when operating in 2d mode
  virtual void ApplyOffset(myint &iDasherX, myint &iDasherY);
  ///Override to choose whether to use 1D start handler or not
  virtual CStartHandler *MakeStartHandler();
private:
  CDasherInput *m_pTouch;
  bool bHoldToGo, bUseTouchX, bTilt1D;
};

class CIPhoneTouchFilter : public CStylusFilter, private IPhonePrefsObserver {
public:
	CIPhoneTouchFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, UndoubledTouch *pUndoubledTouch, CIPhoneTiltInput *pTilt);
	
	virtual void KeyUp(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  
  void ApplyTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView);
  void Activate();
  void Deactivate();
  void iPhonePrefsChanged(NSString *key);
private:
  UndoubledTouch *m_pUndoubledTouch;
  CIPhoneTiltInput *m_pTilt;
  bool bUseTiltX;
};

class CIPhoneTwoFingerFilter : public CDefaultFilter {
public:
  CIPhoneTwoFingerFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID);
  virtual void KeyUp(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  virtual void KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  virtual bool DecorateView(CDasherView *pView, CDasherInput *pInput);
};

}
/// @}
