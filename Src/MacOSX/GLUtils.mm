//
//  GLUtils.m
//  Alphabet
//
//  Created by mtrent on Sat Feb 09 2002.
//  Copyright (c) 2002 xxxxx. All rights reserved.
//

#import <OpenGL/glu.h>
#import "GLUtils.h"

void CheckGLError(const char *note)
{
    GLenum error;
	while (error = glGetError()) {
        NSLog(@"%s: %s (%d)", note, gluErrorString(error), error);
    }
}

unsigned int P2Ceiling(unsigned int n)
{
    unsigned long i = 1;
    while (i < n) i <<= 1;
    return i;
}

