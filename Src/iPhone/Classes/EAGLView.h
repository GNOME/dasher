//
//  EAGLView.h
//  iPhone
//
//  Created by Alan Lawrence on 12/03/2009.
//  Copyright Cavendish Laboratory 2009. All rights reserved.
//


#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

#import "DasherScreenCallbacks.h"
#import "DasherAppDelegate.h"

/*
This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
The view content is basically an EAGL surface you render your OpenGL scene into.
Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
*/
@interface EAGLView : UIView<DasherScreenCallbacks> {
    
@private    
    EAGLContext *context;
    
    /* OpenGL names for the renderbuffer and framebuffers used to render to this view */
    GLuint viewRenderbuffer, viewFramebuffer;
    
	BOOL animating, doneLayout, anyDown;
    NSTimeInterval animationInterval;
	DasherAppDelegate *dasherApp;
	colour_t *colourTable;
	
	GLuint mouseBuffer, boxesBuffer;
	GLuint mouseTex, boxesTex;
	GLshort rectcoords[8];
	GLfloat texcoords[8];
}

- (void)startAnimation;
- (void)stopAnimation;
- (void)drawView;
- (id)initWithFrame:(CGRect)frame Delegate:(DasherAppDelegate *)_dasherApp;
@end
