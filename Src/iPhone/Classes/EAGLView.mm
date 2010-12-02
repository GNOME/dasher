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
#import "AlphabetLetter.h"

// A class extension to declare private methods
@interface EAGLView ()

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end


@implementation EAGLView

@synthesize lastTouchCoords;

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
    circ_rad=-1.0f;
      
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
        
        animationInterval = 1.0 / 60.0;
		colourTable = nil;
    }
    return self;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	NSAssert([touches count] == 1, @"Multitouch?!");
	lastTouchCoords = [((UITouch *)[touches anyObject]) locationInView:self];
	NSAssert(!anyDown,@"Touches began when already in progress - multitouch enabled?!?!\n");
	anyDown = YES;
	dasherApp.dasherInterface->KeyDown(get_time(), 100, true, lastTouchCoords.x, lastTouchCoords.y);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	lastTouchCoords = [[touches anyObject] locationInView:self];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	NSAssert([touches count] == 1, @"Multitouch?!");
	NSAssert(anyDown,@"Touches ended when not in progress - multitouch enabled?!?!\n");
	lastTouchCoords = [(UITouch *)[touches anyObject] locationInView:self];
	dasherApp.dasherInterface->KeyUp(get_time(), 100, true, lastTouchCoords.x, lastTouchCoords.y);
  //finished dealing with touch-up event. Finger is now officially off the screen...
  lastTouchCoords.x = lastTouchCoords.y = -1;
  anyDown = NO;
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
  if (doneLayout)
    dasherApp.dasherInterface->ChangeScreen(new CDasherScreenBridge(self));
  else //first time, DasherAppDelegate will create screen as part of startup
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
	int backingWidth, backingHeight;
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
	//boxes buffer
	glGenTextures(1, &boxesTex);
	glGenFramebuffersOES(1, &boxesBuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, boxesBuffer);
	glBindTexture(GL_TEXTURE_2D, boxesTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nil);
	glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, boxesTex, 0);
	if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
	}

	//mouse buffer
	glGenTextures(1, &mouseTex);
	glGenFramebuffersOES(1, &mouseBuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, mouseBuffer);
	glBindTexture(GL_TEXTURE_2D, mouseTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nil);
	glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, mouseTex, 0);
	if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
	}

	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	//glOrthof(0.0, 1.0, 1.0, 0.0, -1.0, 1.0);
#if !TARGET_IPHONE_SIMULATOR || __IPHONE_3_0
#define IntToFixed(A) A<<16
#endif
    glOrthox(0, IntToFixed(backingWidth), IntToFixed(backingHeight), 0, IntToFixed(-1), IntToFixed(1));
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	rectcoords[0] = rectcoords[1] = 0;
	rectcoords[2] = backingWidth; rectcoords[3] = 0;
	rectcoords[4] = 0; rectcoords[5] = backingHeight;
	rectcoords[6] = backingWidth; rectcoords[7] = backingHeight;
	
	GLfloat texw = backingWidth/(float)w, texh = backingHeight/(float)h;
	texcoords[0] = 0.0; texcoords [1] = texh;//0.0;
	texcoords[2] = texw; texcoords[3] = texh;//0.0;
	texcoords[4] = 0.0; texcoords[5] = 0.0;//texh;
	texcoords[6] = texw; texcoords[7] = 0.0;//texh;

	return YES;
}


- (void)destroyFramebuffer {
    
    glDeleteFramebuffersOES(1, &viewFramebuffer); viewFramebuffer = 0;
	glDeleteFramebuffersOES(1, &mouseBuffer); mouseBuffer = 0;
	glDeleteFramebuffersOES(1, &boxesBuffer); boxesBuffer = 0;
    glDeleteRenderbuffersOES(1, &viewRenderbuffer);
    viewRenderbuffer = 0;
}


- (void)startAnimation {
  if (animating) return;
  animating = YES;
  if (doneLayout)
    [self performSelector:@selector(drawView) withObject:nil afterDelay:animationInterval];
  else
    [self setNeedsDisplay];
}


- (void)stopAnimation {
  //stop for now at least - shutdown, settings dialog, etc.
  // (causes drawView not to enqueue another repaint)
  animating = NO;
}

- (void)dealloc {
    
    [self stopAnimation];
    
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [context release];  
    [super dealloc];
}

//DasherScreenCallbacks...
-(void)blankCallback {};

-(void)displayCallback {
	if (!viewFramebuffer) return; //can't display anything yet!
	[self sendMarker:-1];
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    	
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_SHORT, 0, rectcoords);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	for (int i=0; i<2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, i==0 ? boxesTex : mouseTex);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	glFlush();
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
};

-(void)rectangleCallbackX1:(int)x1 y1:(int)y1 x2:(int)x2 y2:(int)y2 fillColorIndex:(int)aFillColorIndex outlineColorIndex:(int)anOutlineColorIndex lineWidth:(int)aLineWidth {
	glDisable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	if (aFillColorIndex != -1) {
		glColor4f(colourTable[aFillColorIndex].r, colourTable[aFillColorIndex].g, colourTable[aFillColorIndex].b, 1.0);
		GLshort coords[8] = {x1,y1, x2,y1, x1,y2, x2,y2};
		glVertexPointer(2, GL_SHORT, 0, coords);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
	if (aLineWidth>0) {
		int oci = anOutlineColorIndex == -1 ? 3 : anOutlineColorIndex;
		glColor4f(colourTable[oci].r, colourTable[oci].g, colourTable[oci].b, 1.0);
		glLineWidth(aLineWidth);
		GLshort coords[] = {x1,y1, x2,y1, x2,y2,  x1,y2};
		glVertexPointer(2, GL_SHORT, 0, coords);
		glDrawArrays(GL_LINE_LOOP, 0, 4);
	}
}

-(void)circleCallbackCentrePoint:(CGPoint)aCentrePoint radius:(float)aRadius fillColourIndex:(int)aFillColourIndex outlineColorIndex:(int)anOutlineColorIndex lineWidth:(int)aLineWidth {
  //it's a bit of a hack, but we cache the last-computed set of points round the circle,
  // as these are the same for all calls with the same radius - and (the hack!) it happens
  // that the radius tends to be the same every time (as the only call to CDasherScreen::DrawCircle
  // is from CircleStartHandler!)...
  if (circ_rad != aRadius) {
    delete circ_coords;
    double costh=1.0f - 1.0f/(2.0f*aRadius);
    double th = acos(costh);
    int numPoints = circPoints = ceil(M_PI/th/2.0f); //for a quarter-circle
    double sinth = sin(th),x(aRadius),y(0.0);
    circ_coords = new GLshort[numPoints*8]; circ_rad = aRadius;
    circ_coords[0] = x; circ_coords[1] = y;
    for (int i=1; i<numPoints; i++) {
      double nx = x*costh - y*sinth;
      double ny = x*sinth + y*costh;
      circ_coords[2*i] = nx;
      circ_coords[2*i+1] = ny;
      x=nx; y=ny;
    }
    for (int i=0; i<numPoints; i++) {
      circ_coords[2*(i+numPoints)] = -circ_coords[2*i+1];
      circ_coords[2*(i+numPoints)+1] = circ_coords[2*i];
      
      circ_coords[2*(i+numPoints*2)] = -circ_coords[2*i];
      circ_coords[2*(i+numPoints*2)+1] = -circ_coords[2*i+1];
      
      circ_coords[2*(i+numPoints*3)] = circ_coords[2*i+1];
      circ_coords[2*(i+numPoints*3)+1] = -circ_coords[2*i];
    }
  }
  
  glDisable(GL_TEXTURE_2D);
  glEnableClientState(GL_VERTEX_ARRAY);
  glTranslatef(aCentrePoint.x, aCentrePoint.y, 0.0);
  if (aFillColourIndex!=-1) {
    glColor4f(colourTable[aFillColourIndex].r, colourTable[aFillColourIndex].g, colourTable[aFillColourIndex].b, 1.0);
    glVertexPointer(2, GL_SHORT, 0, circ_coords);
    glDrawArrays(GL_TRIANGLE_FAN, 0, circPoints*4);
  }
  if (aLineWidth>0) {
    int oci = anOutlineColorIndex == -1 ? 3 : anOutlineColorIndex;
		glColor4f(colourTable[oci].r, colourTable[oci].g, colourTable[oci].b, 1.0);
		glLineWidth(aLineWidth);
		glVertexPointer(2, GL_SHORT, 0, circ_coords);
		glDrawArrays(GL_LINE_LOOP, 0, circPoints*4);
  }
  glTranslatef(-aCentrePoint.x, -aCentrePoint.y, 0.0);
};

-(CGSize)textSizeCallbackWithString:(NSString *)aString size:(int)aSize {
	return [aString sizeWithFont:[UIFont systemFontOfSize:aSize]];
}

- (void)drawTextCallbackWithString:(NSString *)aString x1:(int)x1 y1:(int)y1 size:(int)aSize colorIndex:(int)aColorIndex
{
	AlphabetLetter *letter = [AlphabetLetter forString:aString];
	// TODO could pass the whole colour_t in and let it deal with splitting out the items
	[letter drawWithSize:aSize x:x1 y:y1 r:colourTable[aColorIndex].r g:colourTable[aColorIndex].g b:colourTable[aColorIndex].b];
}

-(void)sendMarker:(int)iMarker {
	if (iMarker != -1)
	{
		glDisable(GL_TEXTURE_2D);
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, iMarker == 0 ? boxesBuffer : mouseBuffer);
		glClearColor(1.0, 1.0, 1.0, iMarker == 0 ? 1.0 : 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}

-(void)polylineCallback:(int)iNum points:(Dasher::CDasherScreen::point *)points width:(int)iWidth colourIndex:(int)iColour {
	if (iNum < 2) return;
	GLshort *coords = new GLshort[iNum*2];
	for (int i = 0; i<iNum; i++)
	{
		coords[2*i] = points[i].x;
		coords[2*i+1] = points[i].y;
	}
	glDisable(GL_TEXTURE_2D);
	glColor4f(colourTable[iColour].r, colourTable[iColour].g, colourTable[iColour].b, 1.0);
	glLineWidth(iWidth);
	glVertexPointer(2, GL_SHORT, 0, coords);
	glDrawArrays(GL_LINE_STRIP, 0, iNum);
	delete coords;
}

-(void)polygonCallback:(int)iNum points:(Dasher::CDasherScreen::point *)points fillColourIndex:(int)iFillColour outlineColourIndex:(int)iOutlineColour width:(int)iWidth {
  if (iNum < 2) return;
  GLshort *coords = new GLshort[iNum*2];
  for (int i = 0; i<iNum; i++)
  {
    coords[2*i] = points[i].x;
    coords[2*i+1] = points[i].y;
  }
  glDisable(GL_TEXTURE_2D);
  if (iFillColour != -1) {
    glColor4f(colourTable[iFillColour].r, colourTable[iFillColour].g, colourTable[iFillColour].b, 1.0);
    glVertexPointer(2, GL_SHORT, 0, coords);
    glDrawArrays(GL_TRIANGLE_FAN, 0, iNum);
  }
  if (iWidth>0) {
    glColor4f(colourTable[iOutlineColour].r, colourTable[iOutlineColour].g, colourTable[iOutlineColour].b, 1.0);
    glLineWidth(iWidth);
    glVertexPointer(2, GL_SHORT, 0, coords);
    glDrawArrays(GL_LINE_LOOP, 0, iNum);
  }
  delete coords;
}

-(void)setColourSchemeWithColourTable:(colour_t *)aColourTable {
	if (colourTable != NULL) {
		free(colourTable);
	}
	colourTable = aColourTable;
}

-(int)boundsWidth {
	return [self bounds].size.width;
}

-(int)boundsHeight {
	return [self bounds].size.height;
}

@end
