#import <Foundation/Foundation.h>
#import "../DasherCore/DasherTypes.h"
#import "../DasherCore/DasherScreen.h"

using namespace Dasher;

typedef struct {
	float r, g, b;
} colour_t;

@protocol DasherScreenCallbacks

-(void)blankCallback;
-(void)displayCallback;
-(void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 fillColorIndex:(int)aFillColorIndex outlineColorIndex:(int)anOutlineColorIndex lineWidth:(int)aLineWidth;
-(void)circleCallbackCentrePoint:(CGPoint)aCentrePoint radius:(float)aRadius outlineColorIndex:(int)anOutlineColorIndex fillColourIndex:(int)aFillColourIndex shouldFill:(BOOL)shouldFill lineWidth:(int)aLineWidth;
-(CGSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize;
-(void)drawTextCallbackWithString:(NSString *)aString x1:(int)x1 y1:(int)y1 size:(int)aSize colorIndex:(int)aColorIndex;
-(void)sendMarker:(int)iMarker;
-(void)polylineCallback:(int)Number points:(Dasher::CDasherScreen::point *)points width:(int)iWidth colourIndex:(int)iColour;
-(void)polygonCallback:(int)Number points:(Dasher::CDasherScreen::point *)points fillColourIndex:(int)iColour outlineColourIndex:(int)iLineColour width:(int)iWidth;
-(void)blankCallback;
-(void)displayCallback;
-(void)setColourSchemeWithColourTable:(colour_t *)colourTable;
-(int)boundsWidth;
-(int)boundsHeight;
@end