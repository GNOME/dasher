//
//  AlphabetLetter.h
//  Alphabet
//
//  Created by mtrent on Fri Feb 08 2002.
//  Copyright (c) 2002 xxxxx. All rights reserved.
//

#import <AppKit/AppKit.h>
#import <OpenGL/gl.h>

@interface AlphabetLetter : NSObject
{
    NSBitmapImageRep *_imageRep;
    GLuint _tex;
}

- (id)initWithString:(NSString *)aString small:(BOOL)small;

- (void)drawWithSize:(float)aSize x:(float)x y:(float)y r:(float)r g:(float)g b:(float)b;
- (NSSize)sizeWithSize:(float)aSize;

@end

