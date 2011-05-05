//
//  DasherViewOpenGL.h
//  MacOSX
//
//  Created by Doug Dickinson on Fri Apr 18 2003.
//  Copyright (c) 2003 Doug Dickinson (dasher AT DressTheMonkey DOT plus DOT com). All rights reserved.
//


#import <OpenGL/gl.h>
#import <AppKit/NSOpenGLView.h>
#import <AppKit/NSNibDeclarations.h>
#import "ColourIO.h"

@class NSString;
@class DasherApp;
class COSXDasherScreen;

@interface DasherViewOpenGL : NSOpenGLView {
  
  COSXDasherScreen *aquaDasherScreen;
  
  IBOutlet DasherApp *dasherApp;
  
  NSTrackingRectTag trackingRectTag;

  NSString *cachedFontName;

  GLuint frameBuffers[2];
  GLuint textures[2];
  GLfloat tc_x,tc_y;
  
  ///current (last successful) framebuffer width & height
	int fw,fh;
  
}

- (void)drawRect:(NSRect)rect;
- (void)mouseEntered:(NSEvent *)theEvent;
- (void)mouseExited:(NSEvent *)theEvent;
- (void)mouseDown:(NSEvent *)e;
- (void)mouseUp:(NSEvent *)e;
- (void)keyDown:(NSEvent *)e;
- (void)keyUp:(NSEvent *)e;
- (id)initWithFrame:(NSRect)frame;
- (void)userDefaultsDidChange:(NSNotification *)aNote;
- (void)adjustTrackingRect;
- (BOOL)isFlipped;
- (BOOL)isOpaque;
- (void)awakeFromNib;
- (BOOL)acceptsFirstResponder;
- (void)dealloc;
- (void)gl_init;
- (void)reshape;
- (void) gl_reshape:(int)w :(int)h;
- (void)redisplay;
- (NSPoint)mouseLocation;

@property (retain) NSString *cachedFontName;
@end
