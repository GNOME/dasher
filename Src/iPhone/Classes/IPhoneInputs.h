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
	std::deque<int> *xHist, *yHist;
	id<UIAccelerometerDelegate> deleg;
};

class UndoubledTouch : public CScreenCoordInput {
public:
  UndoubledTouch();
  bool GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView);
protected:
  UndoubledTouch(ModuleID_t iId, const char *szName);
};

class CIPhoneMouseInput : public UndoubledTouch, protected CSettingsUser {
public:
	CIPhoneMouseInput(CSettingsUser *pCreator);
  
  bool GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView);
};
}

