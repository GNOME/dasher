#import "../Common/Common.h"


#import "../DasherCore/DasherTypes.h"

#import "COSXDasherScreen.h"
#import <Cocoa/Cocoa.h>
#import "DasherUtil.h"
#import "DasherViewCocoa.h"

using namespace Dasher;

COSXDasherScreen::COSXDasherScreen(id <DasherViewCocoa> dv)
: CDasherScreen([dv boundsWidth], [dv boundsHeight]) {
  
  dasherView = dv;
  
}

COSXDasherScreen::~COSXDasherScreen() {
  dasherView = nil; // didn't retain it, so don't release it.
}

void COSXDasherScreen::Blank() {
}

void COSXDasherScreen::Display() {
  [dasherView displayCallback];
}

void COSXDasherScreen::DrawRectangle(int x1, int y1, int x2, int y2, int Color, int iOutlineColour, Opts::ColorSchemes ColorScheme, bool bDrawOutline, bool bFill, int iThickness) {

  [dasherView rectangleCallbackX1:x1 y1:y1 x2:x2 y2:y2 fillColorIndex:Color outlineColorIndex:iOutlineColour shouldOutline:bDrawOutline shouldFill:bFill lineWidth:iThickness];
}

void COSXDasherScreen::DrawCircle(screenint iCX, screenint iCY, screenint iR, int iColour, int iFillColour, int iThickness, bool bFill) {

  [dasherView circleCallbackCentrePoint:NSMakePoint(iCX, iCY) radius:iR outlineColorIndex:iColour fillColourIndex:iFillColour shouldFill:bFill lineWidth:iThickness];
}

void COSXDasherScreen::Polygon(Dasher::CDasherScreen::point *Points, int Number, int Colour, int iWidth) {
  // TODO polygon is not the same as polyline--fix
  NSMutableArray *p = [NSMutableArray arrayWithCapacity:Number];
  int i;
  
  for (i = 0; i < Number; i++) {
    [p addObject:[NSValue valueWithPoint:NSMakePoint(Points[i].x, Points[i].y)]];
  }
  
  [dasherView polylineCallbackPoints:p width:iWidth colorIndex:Colour];
  
}

void COSXDasherScreen::Polyline(Dasher::CDasherScreen::point *Points, int Number, int iWidth, int Colour) {
  NSMutableArray *p = [NSMutableArray arrayWithCapacity:Number];
  int i;
  
  for (i = 0; i < Number; i++) {
    [p addObject:[NSValue valueWithPoint:NSMakePoint(Points[i].x, Points[i].y)]];
  }
  
  [dasherView polylineCallbackPoints:p width:iWidth colorIndex:Colour];
}

void COSXDasherScreen::DrawString(const std::string &String, int x1, int y1, int size) {
  // TODO is that hardcoded 4 correct?
  [dasherView drawTextCallbackWithString:NSStringFromStdString(String) x1:x1 y1:y1 size:size colorIndex:4];
}

void COSXDasherScreen::TextSize(const std::string &String, int *Width, int *Height, int size) {
  // TODO what colour should it be?
  NSSize s = [dasherView textSizeCallbackWithString:NSStringFromStdString(String) size:size colorIndex:4];
  
  *Width = (int)s.width;
  *Height = (int)s.height;
}

void COSXDasherScreen::SendMarker(int iMarker) {
  [dasherView sendMarker:iMarker];
}

void COSXDasherScreen::SetColourScheme(const CColourIO::ColourInfo *pColourScheme) {

  [dasherView setColourSchemeFromColourInfo:pColourScheme];
}

