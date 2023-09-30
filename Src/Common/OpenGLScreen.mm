//
//  OpenGLScreen.cpp
//  Dasher
//
//  Created by Alan Lawrence on 20/03/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//

#import "OpenGLScreen.h"
#import <iostream>
#import "DasherUtil.h"

using namespace Dasher;
using namespace std;

#define BASE_SIZE 32

OpenGLScreen::OpenGLScreen(screenint iWidth, screenint iHeight, GLshort backingWidth, GLshort backingHeight, GLfloat tc_x, GLfloat tc_y, GLuint *_textures, double _screenToOpenGLScaleFactor)
: CLabelListScreen(iWidth,iHeight), colourTable(NULL), circ_rad(-1.0f), circ_coords(NULL), circPoints(0), textures(_textures), screenToOpenGLScaleFactor(_screenToOpenGLScaleFactor) {
  resize(iWidth,iHeight,backingWidth,backingHeight,tc_x,tc_y);
}

void OpenGLScreen::resize(screenint iWidth, screenint iHeight, GLshort backingWidth, GLshort backingHeight, GLfloat tc_x, GLfloat tc_y) {
  CLabelListScreen::resize(iWidth, iHeight);
  rectcoords[0] = rectcoords[1] = 0;
  rectcoords[2] = backingWidth; rectcoords[3] = 0;
  rectcoords[4] = 0; rectcoords[5] = backingHeight;
  rectcoords[6] = backingWidth; rectcoords[7] = backingHeight;
  
  texcoords[0] = 0.0; texcoords[1] = tc_y;
  texcoords[2] = tc_x; texcoords[3] = tc_y;
  texcoords[4] = 0.0; texcoords[5] = 0.0;
  texcoords[6] = tc_x; texcoords[7] = 0.0;  
}

OpenGLScreen::~OpenGLScreen() {
  delete colourTable;
  delete circ_coords;
}

void OpenGLScreen::Display() {
  SendMarker(-1);
  
	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_SHORT, 0, rectcoords);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
	for (int i=0; i<2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glFlush();
}

void OpenGLScreen::SetColourScheme(const CColourIO::ColourInfo *pColourScheme) {
 	delete colourTable;
  
  int iNumColours = pColourScheme->Reds.size();
    
  colourTable = new colour_t[iNumColours];
    
  for(int i = 0; i < iNumColours; i++) {
    colour_t &ct(colourTable[i]);
    ct.r = pColourScheme->Reds[i] / 255.0;
    ct.g = pColourScheme->Greens[i] / 255.0;
    ct.b = pColourScheme->Blues[i] / 255.0;
  }
}
  
void OpenGLScreen::Polyline(point *Points, int iNum, int iWidth, int iColour) {
	if (iNum < 2) return;
	GLshort *coords = new GLshort[iNum*2];
	for (int i = 0; i<iNum; i++)
	{
		coords[2*i] = Points[i].x;
		coords[2*i+1] = Points[i].y;
	}
	glDisable(GL_TEXTURE_2D);
	glColor4f(colourTable[iColour].r, colourTable[iColour].g, colourTable[iColour].b, 1.0);
  glLineWidth(iWidth * screenToOpenGLScaleFactor);
  glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_SHORT, 0, coords);
	glDrawArrays(GL_LINE_STRIP, 0, iNum);
  glDisableClientState(GL_VERTEX_ARRAY);
	delete coords;
}

void OpenGLScreen::Polygon(point *points, int iNum, int iFillColour, int iOutlineColour, int iWidth) {
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
  delete[] coords;
}

void OpenGLScreen::DrawRectangle(int x1, int y1, int x2, int y2, int iFillColorIndex, int iOutlineColour, int iThickness) {
  glDisable(GL_TEXTURE_2D);
  glEnableClientState(GL_VERTEX_ARRAY);
  if (iFillColorIndex != -1) {
    glColor4f(colourTable[iFillColorIndex].r, colourTable[iFillColorIndex].g, colourTable[iFillColorIndex].b, 1.0);
    GLshort coords[8] = {x1,y1, x2,y1, x1,y2, x2,y2};
    glVertexPointer(2, GL_SHORT, 0, coords);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
  if (iThickness>0) {
    int oci = iOutlineColour == -1 ? 3 : iOutlineColour;
    glColor4f(colourTable[oci].r, colourTable[oci].g, colourTable[oci].b, 1.0);
    glLineWidth(iThickness);
    GLshort coords[] = {x1,y1, x2,y1, x2,y2,  x1,y2};
    glVertexPointer(2, GL_SHORT, 0, coords);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
  }
}

void OpenGLScreen::DrawCircle(screenint iCX, screenint iCY, screenint iR, int iFillColour, int iOutlineColour, int iLineWidth) {
  //it's a bit of a hack, but we cache the last-computed set of points round the circle,
  // as these are the same for all calls with the same radius - and (the hack!) it happens
  // that the radius tends to be the same every time (as the only call to CDasherScreen::DrawCircle
  // is from CircleStartHandler!)...
  if (circ_rad != iR) {
    delete circ_coords;
    double costh=1.0f - 1.0f/(2.0f*iR);
    double th = acos(costh);
    int numPoints = circPoints = ceil(M_PI/th/2.0f); //for a quarter-circle
    double sinth = sin(th),x(iR),y(0.0);
    circ_coords = new GLshort[numPoints*8]; circ_rad = iR;
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
  glTranslatef(iCX, iCY, 0.0);
  if (iFillColour!=-1) {
    glColor4f(colourTable[iFillColour].r, colourTable[iFillColour].g, colourTable[iFillColour].b, 1.0);
    glVertexPointer(2, GL_SHORT, 0, circ_coords);
    glDrawArrays(GL_TRIANGLE_FAN, 0, circPoints*4);
  }
  if (iLineWidth>0) {
    int oci = iOutlineColour == -1 ? 3 : iOutlineColour;
    glColor4f(colourTable[oci].r, colourTable[oci].g, colourTable[oci].b, 1.0);
    glLineWidth(iLineWidth);
    glVertexPointer(2, GL_SHORT, 0, circ_coords);
    glDrawArrays(GL_LINE_LOOP, 0, circPoints*4);
  }
  glTranslatef(-iCX, -iCY, 0.0);
}


#pragma mark text/label classes+routines

void dump(char *data, int width, int height)
{
	static char buf[10240]; buf[0] = 0;
	for (int h = 0; h < height; h++)
	{
		for (int w = 0; w < width; w++)
			sprintf(buf+strlen(buf), "%i ", *((int*)&data[4* (w + h * width)]));
		sprintf(buf + strlen(buf), "\n");
	}
	std::cout << buf;
}

OpenGLScreen::AlphabetLetter::AlphabetLetter(OpenGLScreen *pScreen, const string &strText, unsigned int iWrapSize) : Label(pScreen,strText,iWrapSize), str(NSStringFromStdString(strText)) {
  [str retain];

  glGenTextures(1, &texture);

  PrepareTexture();
}

void OpenGLScreen::AlphabetLetter::PrepareTexture() {
  
  sz = static_cast<OpenGLScreen *>(m_pScreen)->TextSize(str,m_iWrapSize ? m_iWrapSize : BASE_SIZE,m_iWrapSize);
  
  int width=1, height=1;
  GLfloat texw,texh;
  
  while (width<sz.width) width<<=1;
  while (height<sz.height) height<<=1;
  texw = sz.width/(float)width;
  texh = sz.height/(float)height;

  char *data = new char[width*height*4];
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
  CGContextRef context = CGBitmapContextCreate(data, width, height, 8, width*4, colorSpace, kCGImageAlphaPremultipliedLast);
  CGContextClearRect(context, CGRectMake(0.0, 0.0, width, height));

  static_cast<OpenGLScreen *>(m_pScreen)->RenderStringOntoCGContext(str,context,m_iWrapSize ? m_iWrapSize : BASE_SIZE,m_iWrapSize);

  glBindTexture(GL_TEXTURE_2D, texture);
  //...but tell the GL _not_ to interpolate between texels, as that results in a _big_
  // grey border round each letter (?!)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, CGBitmapContextGetData(context));

  CGColorSpaceRelease(colorSpace);
  CGContextRelease(context);
  free(data);
  
  //set texture coords for the corners of the part of the texture we actually
  // drew into (as the texture had to be sized to a power of 2; note we rendered
  // into the *bottom* left portion)
  texcoords[0] = 0.0; texcoords[1] = 1.0;
  texcoords[2] = texw; texcoords[3] = 1.0;
  texcoords[4] = 0.0; texcoords[5] = 1.0-texh;
  texcoords[6] = texw; texcoords[7] = 1.0-texh;
}

OpenGLScreen::AlphabetLetter::~AlphabetLetter() {
  [str release];
  glDeleteTextures(1, &texture);
}

OpenGLScreen::AlphabetLetter *OpenGLScreen::MakeLabel(const std::string &strText,unsigned int iWrapSize) {
  return new AlphabetLetter(this,strText,iWrapSize);
}

void OpenGLScreen::RegenerateLabels() {
  for (set<Label *>::iterator it=LabelsBegin(); it!=LabelsEnd(); it++)
    static_cast<AlphabetLetter *>(*it)->PrepareTexture();
}

void OpenGLScreen::DrawString(CDasherScreen::Label *label, screenint x, screenint y, unsigned int iFontSize, int iColour) {
  //(void)drawWithSize:(int)iSize x:(int)x y:(int)y r:(float)r g:(float)g b:(float)b {
	const AlphabetLetter *l(static_cast<AlphabetLetter *> (label));
	// bind and draw
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, l->texture);
  //"modulate" means to multiply the texture (i.e. 100%=white text, 0%=transparent background)
  // by the currently selected GL foreground colour
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glColor4f(colourTable[iColour].r, colourTable[iColour].g, colourTable[iColour].b, 1.0); //so we select the colour we want the text to appear in
	pair<screenint,screenint> rect = TextSize(label, iFontSize);
	GLshort coords[8];
	coords[0] = x; coords[1]=y;
	coords[2] = x+rect.first; coords[3] = y;
	coords[4] = x; coords[5] = y+rect.second;
	coords[6] = x+rect.first; coords[7]=y+rect.second;
  glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_SHORT, 0, coords);
	glTexCoordPointer(2, GL_FLOAT, 0, l->texcoords);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
	glDisable(GL_TEXTURE_2D);
}

pair<screenint,screenint> OpenGLScreen::TextSize(CDasherScreen::Label *label, unsigned int iFontSize) {
  const AlphabetLetter *l(static_cast<AlphabetLetter *> (label));
  const int baseSize(l->m_iWrapSize ? l->m_iWrapSize : BASE_SIZE);
  //apply "ceil" to floating-point width/height ?
  return pair<screenint,screenint>((l->sz.width*iFontSize)/baseSize, (l->sz.height*iFontSize)/baseSize);
}

