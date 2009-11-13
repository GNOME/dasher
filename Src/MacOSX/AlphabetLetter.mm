//
//  AlphabetLetter.m
//  Dasher
//
//  Created by Alan Lawrence on 20/03/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import "AlphabetLetter.h"
#import <iostream>

@implementation AlphabetLetter

static NSDictionary *fontAttrs;

+(void)initialize {
  //white text on (default) transparent background means that when we texture
  //a surface using a colour, the text appears in that colour...
  fontAttrs = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont systemFontOfSize:36.0],NSFontAttributeName,[NSColor whiteColor],NSForegroundColorAttributeName,nil];
  //dictionaryWith...: does an autorelease - only "alloc" methods do not.
  // But we want to keep the fontAttrs indefinitely...
  [fontAttrs retain];
}

/*void dump(char *data, int width, int height)
{
	static char buf[10240]; buf[0] = 0;
	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
			sprintf(buf+strlen(buf), "%i ", *((int*)&data[4* (w + h * width)]));
		sprintf(buf + strlen(buf), "\n");
	}
	std::cout << buf;
}*/

-(id)initWithString:(NSString *)aString
{
    if (self = [super init])
	{
		string = aString;
		int width=1, height=1;
		GLfloat texw,texh;
		{
			NSSize sz = [self sizeWithSize:36];
			while (width<sz.width) width<<=1;
			while (height<sz.height) height<<=1;
			texw = sz.width/(float)width;
			texh = sz.height/(float)height;
		}

		char *data = new char[width*height*4];
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		CGContextRef context = CGBitmapContextCreate(data, width, height, 8, width*4, colorSpace, kCGImageAlphaPremultipliedLast);
		NSGraphicsContext *old = [NSGraphicsContext currentContext];
		[NSGraphicsContext setCurrentContext:[NSGraphicsContext graphicsContextWithGraphicsPort:context flipped:YES]];

		CGContextClearRect(context, CGRectMake(0.0, 0.0, width, height));
		[string drawAtPoint:NSMakePoint(0.0, 0.0) withAttributes:fontAttrs];
		[NSGraphicsContext setCurrentContext:old];

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
    //...but tell the GL _not_ to interpolate between texels, as that results in a _big_
    // grey border round each letter (?!)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, CGBitmapContextGetData(context));

		//set texture coords for the corners of the part of the texture we actually
		// drew into (as the texture had to be sized to a power of 2; note we rendered
		// into the *bottom* left portion)
		texcoords[0] = 0.0; texcoords[1] = 1.0;
		texcoords[2] = texw; texcoords[3] = 1.0;
		texcoords[4] = 0.0; texcoords[5] = 1.0-texh;
		texcoords[6] = texw; texcoords[7] = 1.0-texh;
		CGContextRelease(context);
		free(data);
	}
	return self;
}

-(NSSize)sizeWithSize:(int)iSize {
	return [string sizeWithAttributes:[NSDictionary dictionaryWithObject:[NSFont systemFontOfSize:iSize] forKey:NSFontAttributeName]];
}

- (void)drawWithSize:(int)iSize x:(int)x y:(int)y r:(float)r g:(float)g b:(float)b {
	
	// bind and draw
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
  //"modulate" means to multiply the texture (i.e. 100%=white text, 0%=transparent bagkground)
  // by the currently selected GL foreground colour
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4f(r, g, b, 1.0); //so we select the colour we want the text to appear in
	NSSize sz = [self sizeWithSize:iSize];
	GLshort coords[8];
	coords[0] = x; coords[1]=y;
	coords[2] = x+sz.width; coords[3] = y;
	coords[4] = x; coords[5] = y+sz.height;
	coords[6] = x+sz.width; coords[7]=y+sz.height;
	glVertexPointer(2, GL_SHORT, 0, coords);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
}

@end

