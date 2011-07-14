/*
 *  CIPhoneInput.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 29/04/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */

#import "../DasherCore/DasherInput.h"
#import "../DasherCore/DasherTypes.h"
#import "../DasherCore/SettingsStore.h"
#import "SBTree.h"
#import <deque>
#import "Vec3.h"
//A bit odd I admit, but we use the same string as is the default input device in Parameters.h...
#define UNDOUBLED_TOUCH "Undoubled Touch"
#define TOUCH_INPUT "Mouse Input"
#define TILT_INPUT "Tilt Input"
#define TWO_FINGER_INPUT "Two-finger (multitouch) input"

@class EAGLView;
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
namespace Dasher {

class CIPhoneTiltInput : public CScreenCoordInput {
public:
	CIPhoneTiltInput(); 
	~CIPhoneTiltInput();
	
	void NotifyTilt(float fx, float fy, float fz);
	
	void SetAxes(Vec3 main, float offset, Vec3 slow, float slow_off)
	{
		this->main = main;
		this->offset = offset;
		this->slow = slow;
		this->slow_off = slow_off;
	}
	
	void Activate();
	void Deactivate();

  bool GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView);
private:
  screenint m_iX, m_iY;
  int maxX, maxY;
	Vec3 main, slow;
	float offset, slow_off;
	SBTree *xTilts, *yTilts;
	std::deque<int> xHist, yHist;
	id<UIAccelerometerDelegate> deleg;
};

class UndoubledTouch : public CScreenCoordInput {
public:
  UndoubledTouch(EAGLView *pView);
  bool GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView);
protected:
  UndoubledTouch(ModuleID_t iId, const char *szName, EAGLView *pView);
  EAGLView * const m_pView;
};

class CIPhoneMouseInput : public UndoubledTouch, private IPhonePrefsObserver {
public:
	CIPhoneMouseInput(EAGLView *pView);
  
  bool GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView);
private:
  void iPhonePrefsChanged(NSString *str);
  bool m_bDoubleX;
};
  
  class CIPhoneTwoFingerInput : public CDasherCoordInput {
  public:
    CIPhoneTwoFingerInput(EAGLView *pView);
    bool GetDasherCoords(myint &iX, myint &iY, CDasherView *pView);
  protected:
    EAGLView * const m_pGlView;
  };
}

