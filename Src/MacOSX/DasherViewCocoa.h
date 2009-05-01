/*
 *  DasherViewCocoa.h
 *  Dasher
 *
 *  Created by Doug Dickinson on 26/06/2007.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#import <Cocoa/Cocoa.h>
#import "../DasherCore/ColourIO.h"

@class COSXDasherScreen;

@protocol DasherViewCocoa
// protocol to allow easily changing implementation of the view.


- (void)sendMarker:(int)iMarker;
- (void)displayCallback;
- (void)circleCallbackCentrePoint:(NSPoint)aCentrePoint radius:(float)aRadius outlineColorIndex:(int)anOutlineColorIndex fillColourIndex:(int)aFillColourIndex shouldFill:(BOOL)shouldFill lineWidth:(int)aLineWidth;
- (void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 fillColorIndex:(int)aFillColorIndex outlineColorIndex:(int)anOutlineColorIndex shouldOutline:(BOOL)shouldOutline shouldFill:(BOOL)shouldFill lineWidth:(int)aLineWidth;
- (NSSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize colorIndex:(int)aColorIndex;
- (void)drawTextCallbackWithString:(NSString *)aString x1:(int)x1 y1:(int)y1 size:(int)aSize colorIndex:(int)aColorIndex;
- (void)polylineCallbackPoints:(NSArray *)points width:(int)aWidth colorIndex:(int)aColorIndex;
- (void)finishRealization;
- (COSXDasherScreen *)aquaDasherScreen;
- (void)setColourSchemeFromColourInfo:(const Dasher::CColourIO::ColourInfo *)pColourScheme;
- (NSPoint)mouseLocation;
- (float)boundsWidth;
- (float)boundsHeight;
- (void)redisplay;

@end
