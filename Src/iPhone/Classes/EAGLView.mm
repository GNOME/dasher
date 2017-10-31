//
//  EAGLView.m
//  Dasher
//
//  Created by Alan Lawrence on 18/03/2009.
//  Copyright Cavendish Laboratory 2009. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#import "EAGLView.h"
#import "DasherUtil.h"
#import "DasherAppDelegate.h"

// A class extension to declare private methods
@interface EAGLView ()
- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;
- (void)displayCallback;
-(void)sendMarker:(int)iMarker;
@property (readonly) bool readyToDisplay;
@end

using namespace Dasher;

CDasherScreenBridge::CDasherScreenBridge(EAGLView *_view, screenint iWidth, screenint iHeight, GLshort backingWidth, GLshort backingHeight, GLfloat tc_x, GLfloat tc_y, GLuint *textures)
: OpenGLScreen(iWidth, iHeight, backingWidth, backingHeight, tc_x, tc_y, textures), view(_view) {
}

void CDasherScreenBridge::resize(screenint iWidth, screenint iHeight, GLshort backingWidth, GLshort backingHeight, GLfloat tc_x, GLfloat tc_y) {
  OpenGLScreen::resize(iWidth, iHeight, backingWidth, backingHeight, tc_x, tc_y);
}

void CDasherScreenBridge::Display() {
  if (!view.readyToDisplay) return; //can't display anything yet!
  OpenGLScreen::Display();
    [view displayCallback];
};
  
void CDasherScreenBridge::SendMarker(int iMarker) {
  [view sendMarker:iMarker];
}

void CDasherScreenBridge::RenderStringOntoCGContext(NSString *str, CGContextRef context, unsigned int iFontSize, bool bWrap) {
  UIGraphicsPushContext(context);
  //white text on transparent background means that when we texture
  //a surface using a colour, the text appears in that colour...
  const CGFloat whiteComps[] = {1.0, 1.0, 1.0, 1.0};
  CGColorRef white = CGColorCreate(CGBitmapContextGetColorSpace(context), whiteComps);
  CGContextSetFillColorWithColor(context, white);
  UIFont *font = [UIFont systemFontOfSize:iFontSize];
  if (bWrap)
    [str drawInRect:CGRectMake(0.0, 0.0, GetWidth(), CGFLOAT_MAX) withFont:font];
  else
    [str drawAtPoint:CGPointMake(0.0, 0.0) withFont:font];
  CGColorRelease(white);
  UIGraphicsPopContext();  
}
  
CGSize CDasherScreenBridge::TextSize(NSString *str, unsigned int iFontSize, bool bWrap) {
  UIFont *font = [UIFont systemFontOfSize:iFontSize];
  return bWrap
    ? [str sizeWithFont:font constrainedToSize:CGSizeMake(GetWidth(), CGFLOAT_MAX) lineBreakMode:UILineBreakModeWordWrap]
    : [str sizeWithFont:font];
}

bool operator<(CGPoint p,CGPoint q) {
  if (p.x<q.x) return true;
  if (p.x>q.x) return false;
  //equal x's
  return (p.y<q.y);
}

bool operator==(CGPoint p,CGPoint q) {
  return CGPointEqualToPoint(p, q);
}

@implementation EAGLView

@synthesize animating;

-(CGPoint)lastTouchCoords {
  return fingerPosns.empty() ? CGPointMake(-1,-1) : fingerPosns.begin()->second;
}

-(void)getAllTouchCoordsInto:(vector<CGPoint> *)into {
  for (map<int,CGPoint>::iterator it=fingerPosns.begin(); it!=fingerPosns.end(); it++)
    into->push_back(it->second);
}

// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame {
  NSLog(@"initWithFrame needs delegate\n");
  return nil;
}

- (id)initWithFrame:(CGRect)frame Delegate:(DasherAppDelegate *)_dasherApp {
    
    if ((self = [super initWithFrame:frame])) {
		dasherApp = _dasherApp;
      
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context]) {
            [self release];
            return nil;
        }
		glShadeModel(GL_FLAT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
      glGenTextures(2, textures);
      glGenFramebuffersOES(2, buffers);

        animationInterval = 1.0 / 60.0;
      //textures is a pointer to 2 ints, so even tho the ints haven't been init'd yet, we can still pass the ptr
      dasherScreen = new CDasherScreenBridge(self, frame.size.width, frame.size.height, 0, 0, 0, 0, textures);
      dasherApp.dasherInterface->ChangeScreen(dasherScreen);
    }
    return self;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
  const unsigned long time(get_time());
  for (UITouch *touch in touches) {
    const int finger(fingerPosns.empty() ? 0 : fingerPosns.rbegin()->first+1);
    NSAssert(allTouches.find(touch)==allTouches.end(),@"Touch beginning already in progress?");
    CGPoint p=[touch locationInView:self];
    allTouches[touch]=finger;
    fingerPosns[finger]=p;
    //first finger with button id 100 = left button, then 101 and so on (element already inserted)
    dasherApp.dasherInterface->KeyDown(time, allTouches.size()+99);
  }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
  for (UITouch *touch in touches) {
    const int finger(allTouches[touch]);
    map<int,CGPoint>::iterator it = fingerPosns.find(finger);
    NSAssert(it!=fingerPosns.end(),@"Moving finger not found?");
    it->second=[touch locationInView:self];
  }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
  const unsigned long time(get_time());
  for (UITouch *touch in touches) {
    map<UITouch*,int>::iterator it = allTouches.find(touch);
    NSAssert(it != allTouches.end(), @"Release touch not in progress?");
    map<int,CGPoint>::iterator it2=fingerPosns.find(it->second);
    NSAssert(it2 != fingerPosns.end(), @"No coordinates for touch?");
    //For consistency with MacOS and Gtk2, and to allow filters
    // to check the coordinates of the click up, call KeyUp first...
    it2->second=[touch locationInView:self];
    dasherApp.dasherInterface->KeyUp(time, allTouches.size()+99);
    //...and only _then_ forget the coordinates:
    fingerPosns.erase(it2);
    allTouches.erase(it);
  }
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	[self touchesEnded:touches withEvent:event];
}

- (void)drawView {
	[EAGLContext setCurrentContext:context];
  if (animating) {
    dasherApp.dasherInterface->NewFrame(get_time(), false);
    [self performSelector:@selector(drawView) withObject:nil afterDelay:animationInterval];
  }
}

- (void)layoutSubviews {
  [EAGLContext setCurrentContext:context];
  [self destroyFramebuffer];
  [self createFramebuffer];
  CGSize sz = [self bounds].size;
  dasherScreen->resize(sz.width, sz.height, backingWidth, backingHeight, texw, texh);
  dasherApp.dasherInterface->ScreenResized(dasherScreen);
  doneLayout = YES;
  [self drawView];
}

- (BOOL)createFramebuffer {
    //dasherApp.dasherInterface->ChangeView();//???
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
	//CGRect r = [self bounds];
    //glViewport(r.origin.x, r.origin.y, r.size.width, r.size.height);

    // set backingWidth and backingHeight
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
    
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
	glViewport(0, 0, backingWidth, backingHeight);
	
	int w=1, h=1;
	while (w < backingWidth) w <<= 1;
	while (h < backingHeight) h <<= 1;

	for (int i=0; i<(sizeof(textures)/sizeof(textures[0])); i++) {
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, buffers[i]);
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nil);
    glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, textures[i], 0);
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
      NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
      return NO;
    }
  }

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	//glOrthof(0.0, 1.0, 1.0, 0.0, -1.0, 1.0);
/*
#if !TARGET_IPHONE_SIMULATOR || __IPHONE_3_0
#define IntToFixed(A) A<<16
#endif
*/
#define IntToFixed(A) A
    glOrthof(0, IntToFixed(backingWidth), IntToFixed(backingHeight), 0, IntToFixed(-1), IntToFixed(1));
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	texw = backingWidth/(float)w, texh = backingHeight/(float)h;
	
	return YES;
}

-(bool)readyToDisplay {
  return viewFramebuffer!=0;
}

-(void)makeContextCurrent {
  [EAGLContext setCurrentContext:context];
}

- (void)destroyFramebuffer {
    
    glDeleteFramebuffersOES(1, &viewFramebuffer); viewFramebuffer = 0;
	  glDeleteRenderbuffersOES(1, &viewRenderbuffer);
    viewRenderbuffer = 0;
}

-(void)setAnimating:(BOOL)bNewValue {
  if (animating==bNewValue) return;
  animating = bNewValue;
  if (animating) {
    if (doneLayout)
      [self performSelector:@selector(drawView) withObject:nil afterDelay:animationInterval];
    else
      [self setNeedsDisplay];
  }
  //setting to false, e.g. for shutdown / settings dialog / etc.,
  // causes drawView not to enqueue another repaint.
}

- (void)dealloc {
    
  self.animating=NO;

  glDeleteFramebuffersOES(2, buffers);
  glDeleteTextures(2,textures);
  for (int i=0; i<2; i++)
    buffers[i]=textures[i]=0;
  
  [self destroyFramebuffer];
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [context release];  
    [super dealloc];
}

-(void)displayCallback {
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
  [context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

-(void)sendMarker:(int)iMarker {
  if (iMarker == -1) {
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
  } else {
    glDisable(GL_TEXTURE_2D);
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, buffers[iMarker]);
    glClearColor(1.0, 1.0, 1.0, iMarker == 0 ? 1.0 : 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
  }  
}


@end
