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

using namespace Dasher;

@class NSUserDefaultsObserver;

class IPhonePrefsObserver {
public:
  virtual void iPhonePrefsChanged(NSString *key)=0;
protected:
  void ObserveKeys(NSString *key,...);
  ~IPhonePrefsObserver();
private:
  NSUserDefaultsObserver *obsvr;
};
#ifndef __IPHONE_FILTERS_MM__
extern NSString *HOLD_TO_GO;
extern NSString *TILT_1D;
extern NSString *TILT_USE_TOUCH_X;
extern NSString *TOUCH_USE_TILT_X;
#endif

#define TILT_FILTER "IPhone Tilt Filter"
#define TOUCH_FILTER "IPhone Touch Filter"
class CIPhoneTiltFilter : public COneDimensionalFilter, private IPhonePrefsObserver {
public:
	CIPhoneTiltFilter(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, CDasherInput *pTouch);
  ///override to enable hold-to-go
	virtual void KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CUserLogBase *pUserLog);
  ///override to enable hold-to-go
	virtual void KeyUp(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);

  ///respond to BP_DASHER_PAUSED by engaging wakelock (if !hold-to-go)
  virtual void HandleEvent(int iParameter);
  void iPhonePrefsChanged(NSString *key);
  bool supportsPause();
protected:
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

/// @}