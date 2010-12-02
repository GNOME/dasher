#import "../Common/Common.h"


#import "../DasherCore/DasherTypes.h"

#import "CDasherScreenBridge.h"
#import "DasherUtil.h"

using namespace Dasher;

CDasherScreenBridge::CDasherScreenBridge(id <DasherScreenCallbacks> dv)
: CDasherScreen([dv boundsWidth], [dv boundsHeight]) {
  
  dasherView = dv;
  
}

CDasherScreenBridge::~CDasherScreenBridge() {
  dasherView = nil; // didn't retain it, so don't release it.
}

bool CDasherScreenBridge::GetTouchCoords(screenint &iX, screenint &iY) {
  CGPoint p = dasherView.lastTouchCoords;
  if (p.x==-1) return false;
  iX=p.x; iY=p.y;
  return true;
}

void CDasherScreenBridge::Blank() {
  [dasherView blankCallback];
}

void CDasherScreenBridge::Display() {
  [dasherView displayCallback];
}

void CDasherScreenBridge::DrawRectangle(int x1, int y1, int x2, int y2, int Color, int iOutlineColour, int iThickness) {

  [dasherView rectangleCallbackX1:x1 y1:y1 x2:x2 y2:y2 fillColorIndex:Color outlineColorIndex:iOutlineColour lineWidth:iThickness];
}

void CDasherScreenBridge::DrawCircle(screenint iCX, screenint iCY, screenint iR, int iColour, int iFillColour, int iThickness, bool bFill) {

  [dasherView circleCallbackCentrePoint:CGPointMake(iCX, iCY) radius:iR outlineColorIndex:iColour fillColourIndex:iFillColour shouldFill:bFill lineWidth:iThickness];
}

void CDasherScreenBridge::Polygon(Dasher::CDasherScreen::point *Points, int Number, int fillColour, int outlineColour, int iWidth) {
	[dasherView polygonCallback:Number points:Points fillColourIndex:fillColour outlineColourIndex:outlineColour width:iWidth];
  
}

void CDasherScreenBridge::Polyline(Dasher::CDasherScreen::point *Points, int Number, int iWidth, int Colour) {
	[dasherView polylineCallback:Number points:Points width:iWidth colourIndex:Colour];
}

void CDasherScreenBridge::DrawString(const std::string &String, screenint x1, screenint y1, int size, int iColour) {
	[dasherView drawTextCallbackWithString:NSStringFromStdString(String) x1:x1 y1:y1 size:size colorIndex:iColour];
}

void CDasherScreenBridge::TextSize(const std::string &String, screenint *Width, screenint *Height, int size) {
  // TODO what colour should it be?
  CGSize s = [dasherView textSizeCallbackWithString:NSStringFromStdString(String) size:size];
  
  *Width = (screenint)s.width;
  *Height = (screenint)s.height;
}

void CDasherScreenBridge::SendMarker(int iMarker) {
  [dasherView sendMarker:iMarker];
}

void CDasherScreenBridge::SetColourScheme(const CColourIO::ColourInfo *pColourScheme) {
	int iNumColours = pColourScheme->Reds.size();
	
	colour_t *ct = (colour_t *)malloc(iNumColours * sizeof(colour_t));
	
	for(int i = 0; i < iNumColours; i++) {
		ct[i].r = pColourScheme->Reds[i] / 255.0;
		ct[i].g = pColourScheme->Greens[i] / 255.0;
		ct[i].b = pColourScheme->Blues[i] / 255.0;
	}
	
	// colour table must be freed elsewhere...
  [dasherView setColourSchemeWithColourTable:ct];
}