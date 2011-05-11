/*
 *  IPhoneInput.cpp
 *  Dasher
 *
 *  Created by Alan Lawrence on 29/04/2009.
 *  Copyright 2009 Cavendish Laboratory. All rights reserved.
 *
 */


#import "IPhoneInputs.h"
#import "DasherUtil.h"
#import "DasherAppDelegate.h"
#import "EAGLView.h"

using namespace std;
using namespace Dasher;

@interface Accel : NSObject<UIAccelerometerDelegate> {
  CIPhoneTiltInput *tilt;
}
-(id)initWithInput:(CIPhoneTiltInput *)_tilt;
@end

@implementation Accel
-(id)initWithInput:(CIPhoneTiltInput *)_tilt {
  if (self = [super init]) {
    tilt=_tilt;
  }
  return self;
}
-(void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
	tilt->NotifyTilt(acceleration.x, acceleration.y, acceleration.z);
}
@end

CIPhoneTiltInput::CIPhoneTiltInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore) 
	: CScreenCoordInput(pEventHandler, pSettingsStore, 8, TILT_INPUT) {
	deleg = [[Accel alloc] initWithInput:this];	
	xTilts = NULL;
};

CIPhoneTiltInput::~CIPhoneTiltInput() { [deleg release]; }
	
void CIPhoneTiltInput::NotifyTilt(float fx, float fy, float fz) {
	Vec3 in = Vec3Make(fx,fy,fz);
	float y = Vec3Dot(main, in)-offset;
	y = std::max(0.0f, std::min(1.0f, y));
	float x = Vec3Dot(slow, in) - slow_off;
	x = std::max(0.0f, std::min(1.0f,x));
	const screenint iY(y * (double)maxY);
	const screenint iX(x * (double)maxX);
		
	//apply median filter
	if (xTilts)
	{
		xTilts->Add(iX);
		yTilts->Add(iY);
		if (xTilts->GetCount() > 20)
		{
			xTilts = xTilts->Delete(xHist->front());
			xHist->pop_front();
			yTilts = yTilts->Delete(yHist->front());
			yHist->pop_front();
		}
	}
	else
	{
		xTilts = new SBTree(iX);
		yTilts = new SBTree(iY);
		xHist = new deque<int>();
		yHist = new deque<int>();
	}
	xHist->push_back(iX);
	yHist->push_back(iY);
	const int median(xTilts->GetCount()/2);
	m_iX = xTilts->GetOffset(median);
	m_iY = yTilts->GetOffset(median);
}

void CIPhoneTiltInput::Activate() {
  [[DasherAppDelegate theApp] setLandscapeSupported:NO];
  UIAccelerometer*  theAccelerometer = [UIAccelerometer sharedAccelerometer];
  theAccelerometer.updateInterval = 0.01; //in secs
  theAccelerometer.delegate = deleg;
}
void CIPhoneTiltInput::Deactivate() {
  [[DasherAppDelegate theApp] setLandscapeSupported:YES];
  [UIAccelerometer sharedAccelerometer].delegate = nil;
}

bool CIPhoneTiltInput::GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView) {
  CDasherScreen *pScreen(pView->Screen());
  maxX = pScreen->GetWidth();
  maxY = pScreen->GetHeight();
  
  //could check we're active, but not bothering for now...
  iX=m_iX; iY=m_iY;
  return true;
}

UndoubledTouch::UndoubledTouch(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore) : CScreenCoordInput(pEventHandler, pSettingsStore, 7, UNDOUBLED_TOUCH) {
}

UndoubledTouch::UndoubledTouch(CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, ModuleID_t iId, const char *szName) : CScreenCoordInput(pEventHandler, pSettingsStore, iId, szName) {
}

bool UndoubledTouch::GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView) {
  CDasherScreenBridge *sb(static_cast<CDasherScreenBridge *>(pView->Screen()));
  return sb->GetTouchCoords(iX, iY);
}


CIPhoneMouseInput::CIPhoneMouseInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore) 
	: UndoubledTouch(pEventHandler, pSettingsStore, 9, TOUCH_INPUT) {
};

bool CIPhoneMouseInput::GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView) {
  if (!UndoubledTouch::GetScreenCoords(iX,iY, pView)) return false;
  CDasherScreen *scr(pView->Screen());
  //double x/y
  if (GetBoolParameter(BP_DOUBLE_X)) {
    switch (GetLongParameter(LP_REAL_ORIENTATION)) {
      case Opts::LeftToRight:
        iX=min(iX*2, scr->GetWidth());
        break;
      case Opts::RightToLeft:
        iX=max(iX*2-scr->GetWidth(), 0);
        break;
      case Opts::TopToBottom:
        iY=min(iY*2, scr->GetHeight());
        break;
      case Opts::BottomToTop:
        iY=max(iY*2 - scr->GetHeight(), 0);
        break;
    }
  }
  return true;
}
