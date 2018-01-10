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

@interface Accel : NSObject<UIAccelerometerDelegate> {
  CIPhoneTiltInput *tilt;
  UIInterfaceOrientation m_orient;
}
-(id)initWithInput:(CIPhoneTiltInput *)_tilt;
-(void)orientationDidChange:(NSNotification *)note;
@end

@implementation Accel
-(id)initWithInput:(CIPhoneTiltInput *)_tilt {
  if (self = [super init]) {
    tilt=_tilt;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationDidChange:) name:UIApplicationDidChangeStatusBarOrientationNotification object:[UIApplication sharedApplication]];
    dispatch_async(dispatch_get_main_queue(), ^{
      [self orientationDidChange:nil];
    });
  }
  return self;
}
-(void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration {
  float x,y;
  switch(m_orient) {
    case UIInterfaceOrientationLandscapeLeft:
      x = acceleration.y; y=-acceleration.x; break;
    case UIInterfaceOrientationLandscapeRight:
      x = -acceleration.y; y=acceleration.x; break;
    default:
      DASHER_ASSERT(false);
    case UIInterfaceOrientationPortrait:
      x=acceleration.x; y=acceleration.y; break;
  }
  tilt->NotifyTilt(x, y, acceleration.z);
}

-(void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationDidChangeStatusBarOrientationNotification object:[UIApplication sharedApplication]];
  [super dealloc];
}
-(void)orientationDidChange:(NSNotification *)note {
  m_orient = [UIApplication sharedApplication].statusBarOrientation;
}
@end

CIPhoneTiltInput::CIPhoneTiltInput() 
	: CScreenCoordInput(8, TILT_INPUT) {
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
	if (xTilts) {
		xTilts->Add(iX);
		yTilts->Add(iY);
		if (xTilts->GetCount() > 20) {
			xTilts = xTilts->Delete(xHist.front());
			xHist.pop_front();
			yTilts = yTilts->Delete(yHist.front());
			yHist.pop_front();
		}
	} else {
		xTilts = new SBTree(iX);
		yTilts = new SBTree(iY);
	}
	xHist.push_back(iX);
	yHist.push_back(iY);
	const int median(xTilts->GetCount()/2);
	m_iX = xTilts->GetOffset(median);
	m_iY = yTilts->GetOffset(median);
}

void CIPhoneTiltInput::Activate() {
  [DasherAppDelegate theApp].allowsRotation=NO;
  UIAccelerometer*  theAccelerometer = [UIAccelerometer sharedAccelerometer];
  theAccelerometer.updateInterval = 0.01; //in secs
  theAccelerometer.delegate = deleg;
}

void CIPhoneTiltInput::Deactivate() {
  [DasherAppDelegate theApp].allowsRotation=YES;
  [UIAccelerometer sharedAccelerometer].delegate = nil;
  delete xTilts; xTilts=NULL;
  delete yTilts; yTilts=NULL;
  xHist.clear(); yHist.clear();
}

bool CIPhoneTiltInput::GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView) {
  CDasherScreen *pScreen(pView->Screen());
  maxX = pScreen->GetWidth();
  maxY = pScreen->GetHeight();
  
  //could check we're active, but not bothering for now...
  iX=m_iX; iY=m_iY;
  return true;
}

UndoubledTouch::UndoubledTouch(EAGLView *pView) : CScreenCoordInput(7, UNDOUBLED_TOUCH), m_pView(pView) {
}

UndoubledTouch::UndoubledTouch(ModuleID_t iId, const char *szName, EAGLView *pView) : CScreenCoordInput(iId, szName), m_pView(pView) {
}

bool UndoubledTouch::GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView) {
  CGPoint p = [m_pView lastTouchCoords];
  if (p.x==-1) return false;
  iX = p.x; iY=p.y;
  return true;
}


CIPhoneMouseInput::CIPhoneMouseInput(EAGLView *pView) 
	: UndoubledTouch(9, TOUCH_INPUT, pView) {
    ObserveKeys(DOUBLE_TOUCH_X, nil);
};

bool CIPhoneMouseInput::GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView) {
  if (!UndoubledTouch::GetScreenCoords(iX,iY, pView)) return false;
  CDasherScreen *scr(pView->Screen());
  //double x/y
  if ([[NSUserDefaults standardUserDefaults] boolForKey:DOUBLE_TOUCH_X]) {
    switch (pView->GetOrientation()) {
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

void CIPhoneMouseInput::iPhonePrefsChanged(NSString *str) {
  if ([str isEqualToString:DOUBLE_TOUCH_X])
    m_bDoubleX = [[NSUserDefaults standardUserDefaults] boolForKey:DOUBLE_TOUCH_X];
}

CIPhoneTwoFingerInput::CIPhoneTwoFingerInput(EAGLView *pView) : CDasherCoordInput(10, TWO_FINGER_INPUT), m_pGlView(pView) {
}

bool CIPhoneTwoFingerInput::GetDasherCoords(myint &iX, myint &iY, CDasherView *pView) {
  vector<CGPoint> pts;
  [m_pGlView getAllTouchCoordsInto:&pts];
  if (pts.size()<2) return false;
  myint x1,y1,x2,y2;
  
  //target Y is midpoint of Y coordinates of first two touches
  pView->Screen2Dasher(pts[0].x, pts[0].y, x1, y1);
  pView->Screen2Dasher(pts[1].x, pts[1].y, x2, y2);
  iY = (y1+y2)/2;
  //target X is half the distance between them (-> top/bottom of target range = top/bottom of first two touches)
  iX = abs(y1-y2)/2;

  if (pts.size()>2) {
    //three or more fingers, go backwards...
    pView->VisibleRegion(x1, y1, x2, y2); //x2 is now maxX
    iX = (x2*iX)/2048; //(first two) fingers wider apart, will go backwards faster.
  } 
  return true;
}
