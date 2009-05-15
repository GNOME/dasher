//
//  AlphabetLetter.h
//  Dasher
//
//  Created by Alan Lawrence on 20/03/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import <AppKit/AppKit.h>
#import <OpenGL/gl.h>

@interface AlphabetLetter : NSObject
{
	NSString *string;
	GLuint texture;
	GLfloat texcoords[8];
}

- (id)initWithString:(NSString *)string;
- (void)drawWithSize:(int)aSize x:(int)x y:(int)y r:(float)r g:(float)g b:(float)b;
- (NSSize)sizeWithSize:(int)aSize;
@end
