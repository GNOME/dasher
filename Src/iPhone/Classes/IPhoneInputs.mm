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
	: CIPhoneInput(pEventHandler, pSettingsStore, TILT_INPUT) {
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
  [UIApplication sharedApplication].idleTimerDisabled = YES;
  UIAccelerometer*  theAccelerometer = [UIAccelerometer sharedAccelerometer];
  theAccelerometer.updateInterval = 0.01; //in secs
  theAccelerometer.delegate = deleg;
}
void CIPhoneTiltInput::Deactivate() {
  [UIApplication sharedApplication].idleTimerDisabled = NO;
  [UIAccelerometer sharedAccelerometer].delegate = nil;
}

CIPhoneMouseInput::CIPhoneMouseInput(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore) 
	: CIPhoneInput(pEventHandler, pSettingsStore, TOUCH_INPUT) {
};

int CMixedInput::GetCoordinates(int iN, myint * pCoordinates) {
	DASHER_ASSERT(iN == 2);
	myint pXCoords[2];
	m_pYinput->GetCoordinates(2, pCoordinates);
	m_pXinput->GetCoordinates(2, pXCoords);
	pCoordinates[0] = pXCoords[0];
	
	return 0;
};

void CMixedInput::Activate() {
  m_pYinput->Activate(); m_pXinput->Activate();
}

void CMixedInput::Deactivate() {
  m_pYinput->Deactivate(); m_pXinput->Deactivate();
}