/*
 *  IPhoneFilters.cpp
 *  Dasher
 *
 *  Created by Alan Lawrence on 29/04/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */
#define __IPHONE_FILTERS_MM__
#include "IPhoneFilters.h"

#include "../Common/Common.h"
#include "DasherInterfaceBase.h"
#include "Event.h"
#include "CDasherScreenBridge.h"
#include <iostream>

NSString *HOLD_TO_GO=@"HoldToGo";
NSString *TILT_1D=@"Tilt1D";
NSString * TILT_USE_TOUCH_X=@"UseTouchX";
NSString * TOUCH_USE_TILT_X=@"UseTiltX";

@interface NSUserDefaultsObserver : NSObject {
  IPhonePrefsObserver *po;
}
-(id)initForPrefsObserver:(IPhonePrefsObserver *)po;
-(void)stopObserving;
-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context;
@end

@implementation NSUserDefaultsObserver

-(id)initForPrefsObserver:(IPhonePrefsObserver *)_po {
  if (self = [super init]) {
    self->po = _po;
  }
  return self;
}

-(void)stopObserving {po=nil; [self autorelease];}

-(void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
  NSAssert(object == [NSUserDefaults standardUserDefaults],@"Only observing user defaults?");
  if (po) po->iPhonePrefsChanged(keyPath);
  else [[NSUserDefaults standardUserDefaults] removeObserver:self forKeyPath:keyPath];
}
@end

void IPhonePrefsObserver::ObserveKeys(NSString *key,...) {
  va_list args;
  va_start(args, key);
  
  obsvr = [[NSUserDefaultsObserver alloc] initForPrefsObserver:this];
  while (key) {
    [[NSUserDefaults standardUserDefaults] addObserver:obsvr forKeyPath:key options:0 context:nil];
    iPhonePrefsChanged(key);
    key=va_arg(args, NSString *);
  }
  va_end(args);
}

IPhonePrefsObserver::~IPhonePrefsObserver() {
  [obsvr stopObserving];
  [obsvr release];
}

CIPhoneTiltFilter::CIPhoneTiltFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, CDasherInput *pTouch)
: COneDimensionalFilter(pEventHandler, pSettingsStore, pInterface, iID, TILT_FILTER), m_pTouch(pTouch) {
  ObserveKeys(HOLD_TO_GO, TILT_USE_TOUCH_X, TILT_1D, nil);
};
			
void CIPhoneTiltFilter::ApplyTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView) {
  if (bHoldToGo && bUseTouchX) {
    myint temp;
    m_pTouch->GetDasherCoords(iDasherX,temp,pView);
  }
  if (bTilt1D) {
    COneDimensionalFilter::ApplyTransform(iDasherX, iDasherY, pView);
    //that skips CDefaultFilter::ApplyTransform => no LP_TARGET_OFFSET/BP_AUTO_CALIBRATE or BP_REMAP_XTREME
  } else {
    CDefaultFilter::ApplyTransform(iDasherX, iDasherY, pView);
  }
}

void CIPhoneTiltFilter::KeyDown(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel, CUserLogBase *pUserLog) {
	if(iId == 100 && bHoldToGo)
		m_pInterface->Unpause(iTime);
  else COneDimensionalFilter::KeyDown(iTime, iId, pView, pInput, pModel, pUserLog);
}

void CIPhoneTiltFilter::KeyUp(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
	if(iId == 100 && bHoldToGo)
		m_pInterface->Stop();
  else COneDimensionalFilter::KeyUp(iTime, iId, pView, pInput, pModel);
}

bool CIPhoneTiltFilter::supportsPause() {
  return !bHoldToGo;
}

void CIPhoneTiltFilter::ApplyOffset(myint &iDasherX, myint &iDasherY) {
  //Do not apply LP_TARGET_OFFSET, or BP_AUTO_CALIBRATE
}

void CIPhoneTiltFilter::HandleEvent(CEvent *pEvent) {
  if (pEvent->m_iEventType == EV_PARAM_NOTIFY
      && static_cast<CParameterNotificationEvent *>(pEvent)->m_iParameter==BP_DASHER_PAUSED
      && m_pInterface->GetActiveInputMethod()==this) {
    [UIApplication sharedApplication].idleTimerDisabled=(!GetBoolParameter(BP_DASHER_PAUSED) && !bHoldToGo);
  }
}

void CIPhoneTiltFilter::iPhonePrefsChanged(NSString *key) {
  bool val = [[NSUserDefaults standardUserDefaults] boolForKey:key];
  if ([key isEqualToString:HOLD_TO_GO])
    bHoldToGo = val;
  else if ([key isEqualToString:TILT_USE_TOUCH_X])
    bUseTouchX = val;
  else if ([key isEqualToString:TILT_1D])
    bTilt1D = val;
  //Hmmm, do we need to do anything _now_?
}

CIPhoneTouchFilter::CIPhoneTouchFilter(Dasher::CEventHandler * pEventHandler, CSettingsStore *pSettingsStore, CDasherInterfaceBase *pInterface, ModuleID_t iID, UndoubledTouch *pUndoubledTouch, CIPhoneTiltInput *pTilt)
: CStylusFilter(pEventHandler, pSettingsStore, pInterface, iID, TOUCH_FILTER), m_pUndoubledTouch(pUndoubledTouch), m_pTilt(pTilt) {
  ObserveKeys(TOUCH_USE_TILT_X,nil);
  
};

void CIPhoneTouchFilter::iPhonePrefsChanged(NSString *key) {
  if ([key isEqualToString:TOUCH_USE_TILT_X]) {
    if (m_pInterface->GetActiveInputMethod()==this) {
      //current=new value of preference, should be different from what we have atm...
      // (exception is at construction time - 
      DASHER_ASSERT([[NSUserDefaults standardUserDefaults] boolForKey:TOUCH_USE_TILT_X] ^ bUseTiltX);
      if (bUseTiltX)
        m_pTilt->Deactivate(); //setting was on, so is being turned off
      else 
        m_pTilt->Activate();
    }
    bUseTiltX = [[NSUserDefaults standardUserDefaults] boolForKey:TOUCH_USE_TILT_X];
  }
}

void CIPhoneTouchFilter::Activate() {
  if (bUseTiltX) m_pTilt->Activate();
}

void CIPhoneTouchFilter::Deactivate() {
  if (bUseTiltX) m_pTilt->Deactivate();
}

void CIPhoneTouchFilter::KeyUp(int iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel) {
  CStylusFilter::KeyUp(iTime, iId, pView, m_pUndoubledTouch, pModel);
}

void CIPhoneTouchFilter::ApplyTransform(myint &iDasherX, myint &iDasherY, CDasherView *pView) {
  if (bUseTiltX) {
    myint temp;
    m_pTilt->GetDasherCoords(iDasherX,temp,pView);
  }
  CStylusFilter::ApplyTransform(iDasherX, iDasherY, pView);
}