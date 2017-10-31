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

#import "../Common/OpenGLScreen.h"

@class DasherAppDelegate;
@class EAGLView;

class CDasherScreenBridge : public Dasher::OpenGLScreen {
  EAGLView *view;
public:
  CDasherScreenBridge(EAGLView *_view, Dasher::screenint iWidth, Dasher::screenint iHeight, GLshort backingWidth, GLshort backingHeight, GLfloat tc_x, GLfloat tc_y, GLuint *textures);
  ///Only for EAGLView to call...
  void resize(Dasher::screenint iWidth, Dasher::screenint iHeight, GLshort backingWidth, GLshort backingHeight, GLfloat tc_x, GLfloat tc_y);
  void Display() override;
  void SendMarker(int iMarker) override;
  // Returns always true, because touch is used instead of mouse
  bool IsWindowUnderCursor() override { return YES; }
protected:
  void RenderStringOntoCGContext(NSString *str, CGContextRef context, unsigned int iFontSize, bool bWrap) override;
  CGSize TextSize(NSString *str, unsigned int iFontSize, bool bWrap) override;
};

/*
This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
The view content is basically an EAGL surface you render your OpenGL scene into.
Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
*/
@interface EAGLView : UIView {
    
@private    
    EAGLContext *context;
    
  CDasherScreenBridge *dasherScreen;
  
    /* OpenGL names for the renderbuffer and framebuffers used to render to this view */
    GLuint viewRenderbuffer, viewFramebuffer;
    
	BOOL animating, doneLayout;
    NSTimeInterval animationInterval;
	DasherAppDelegate *dasherApp;
	
	GLuint buffers[2];
	GLuint textures[2];
	GLint backingWidth, backingHeight;
	GLfloat texw,texh;
    
  std::map<UITouch*,int> allTouches;
  std::map<int,CGPoint> fingerPosns;
}

//Co-ordinates of earliest-started touch still touching
-(CGPoint)lastTouchCoords;

-(void)getAllTouchCoordsInto:(std::vector<CGPoint> *)into;

//OpenGL context (needed to do any OGL operation) is only current per-thread, so must call this
// if doing anything on any thread other than the main thread.
-(void)makeContextCurrent;
- (void)drawView;
- (id)initWithFrame:(CGRect)frame Delegate:(DasherAppDelegate *)_dasherApp;
@property BOOL animating;
@end
