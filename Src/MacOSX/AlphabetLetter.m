//
//  AlphabetLetter.m
//  Alphabet
//
//  Created by mtrent on Fri Feb 08 2002.
//  Copyright (c) 2002 xxxxx. All rights reserved.
//

#import <ScreenSaver/ScreenSaver.h>
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>

#import "AlphabetLetter.h"
#import "GLUtils.h"

@implementation AlphabetLetter

- (id)initWithString:(NSString *)aString small:(BOOL)small
{
    self = [super init];
    
    if (self) {
        static NSTextFieldCell *cell = NULL;
      NSRect textFrame = NSZeroRect;
      NSRect bigRect = NSZeroRect;
      NSRect drawFrame = NSZeroRect;
        NSImage *image;
        NSBitmapImageRep *upsideDown;
        int bytesPerRow, bitsPerPixel, height;
        BOOL hasAlpha;
        
        if (!cell) {
            cell = [[NSTextFieldCell allocWithZone:[self zone]] initTextCell:aString];
            [cell setBordered:NO];
            [cell setBezeled:NO];
            [cell setDrawsBackground:NO];
            [cell setWraps:NO];
            [cell setTextColor:[NSColor greenColor]];
            [cell setBackgroundColor:[NSColor clearColor]];
            [cell setFont:[NSFont fontWithName:@"Cochin" size:small ? 18.0 : 36.0]];
            textFrame.origin = NSZeroPoint;
        }
        
        [cell setStringValue:aString];
        textFrame.size = [cell cellSize];
        
        // if using textures, remember we have to use powers of two
        bigRect = textFrame;
        bigRect.size.width = (float)P2Ceiling(bigRect.size.width);
        bigRect.size.height = (float)P2Ceiling(bigRect.size.height);
    
        // prepare bitmap image rep
        image = [[NSImage alloc] initWithSize:bigRect.size];
        [image lockFocus];
        drawFrame = SSCenteredRectInRect(textFrame, bigRect);
        [cell drawInteriorWithFrame:drawFrame inView:[NSView focusView]];
        upsideDown = [[NSBitmapImageRep alloc] initWithFocusedViewRect:bigRect];
        [image unlockFocus];
        [image release];
        
        // since OpenGL draws images upside down, we need to flip this image along 
        // the y axis. I know a cool trick for doing this when texture mapping, 
        // but I want something a little more general for now
        bytesPerRow = [upsideDown bytesPerRow];
        bitsPerPixel = [upsideDown bitsPerPixel];
        hasAlpha = [upsideDown hasAlpha];
        height = (int)bigRect.size.height;
        _imageRep = [[NSBitmapImageRep alloc] 
            initWithBitmapDataPlanes:NULL
            pixelsWide:(int)bigRect.size.width
            pixelsHigh:height
            bitsPerSample:[upsideDown bitsPerSample]
            samplesPerPixel:[upsideDown samplesPerPixel]
            hasAlpha:hasAlpha
            isPlanar:[upsideDown isPlanar]
            colorSpaceName:NSCalibratedRGBColorSpace
            bytesPerRow:bytesPerRow
            bitsPerPixel:bitsPerPixel];
        
        // create the texture

            GLenum format;

            format = hasAlpha ? GL_RGBA : GL_RGB;
            glGenTextures(1, &_tex);
            glBindTexture(GL_TEXTURE_2D, _tex);
            CheckGLError("glBindTexture");
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            CheckGLError("glTexParameteri");
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, bytesPerRow / (bitsPerPixel >> 3));
            CheckGLError("glPixelStorei");
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bigRect.size.width, bigRect.size.height, 0, format, GL_UNSIGNED_BYTE, [upsideDown bitmapData]);
            CheckGLError("glTexImage2D");
            [upsideDown release];
        
    }
    return self;
}

- (void)dealloc
{
  if (_tex) {
    glDeleteTextures(1, &_tex);
  }
  [_imageRep release];
  
  [super dealloc];
}

- (NSSize)sizeWithSize:(float)aSize {
  // the first size refers to the image size, the second to the text's point size
  // maybe some new nomenclature would be a good idea?
  return [_imageRep size];
}

- (void)drawWithSize:(float)aSize x:(float)x y:(float)y r:(float)r g:(float)g b:(float)b
{
    GLenum format;
    NSSize size = [_imageRep size];
    
    // draw our image
    if (_tex) {
        GLfloat l, t, r, b, z;

        // load our coordinates
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
    
        // seed initial position
        glTranslatef(x + (size.width / 2), y + (size.height / 2), 0.0);
        glScalef(aSize, aSize, 1.0);

        // i'm lazy, so I like to use these simple variables
        l = - (size.width / 2);
        r = + (size.width / 2);
        t = + (size.height / 2);
        b = - (size.height / 2);
        z = 0.0;

        // bind and draw
        glBindTexture(GL_TEXTURE_2D, _tex);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glBegin(GL_QUADS);
            glColor4f(r, g, b, 1.0);
            glTexCoord2f(0.0,0.0);  glVertex3f(l, b, z);
            glTexCoord2f(0.0,1.0);  glVertex3f(l, t, z);
            glTexCoord2f(1.0,1.0);  glVertex3f(r, t, z);
            glTexCoord2f(1.0,0.0);  glVertex3f(r, b, z);
        glEnd();
        glPopMatrix();
    } else {
        // we don't support fade-out effects if we're not using textures
//        format = [_imageRep hasAlpha] ? GL_RGBA : GL_RGB;
//        glPixelTransferf(GL_RED_SCALE, r);
//        glPixelTransferf(GL_GREEN_SCALE, g);
//        glPixelTransferf(GL_BLUE_SCALE, b);
//        glPixelTransferf(GL_ALPHA_SCALE, 1.0);
//        glRasterPos2f(x, y);
//        glDrawPixels(size.width, size.height, format, GL_UNSIGNED_BYTE, [_imageRep bitmapData]);
    }
    CheckGLError("draw");
}

@end

