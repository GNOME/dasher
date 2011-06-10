//
//  AlphabetLetter.h
//  Dasher
//
//  Created by Alan Lawrence on 20/03/2009.
//  Copyright 2009 Cavendish Laboratory. All rights reserved.
//
#ifdef TARGET_OS_IPHONE
#import <OpenGLES/ES1/gl.h>
#else
#import <AppKit/AppKit.h>
#import <OpenGL/gl.h>
#endif

#import "DasherScreen.h"
#include <string>

namespace Dasher {
  class OpenGLScreen : public CLabelListScreen {
  protected:
    class AlphabetLetter : public Label {
    public: //to OpenGLScreen and subclasses - all read by DrawString
      NSString *str;
      GLuint texture;
      GLfloat texcoords[8];
      AlphabetLetter(OpenGLScreen *pScreen, const std::string &strText, unsigned int iWrapSize);
      ~AlphabetLetter();
      void PrepareTexture();
    };    
  public:
    OpenGLScreen(screenint iWidth, screenint iHeight, GLshort backingWidth, GLshort backingHeight, GLfloat tc_x, GLfloat tc_y, GLuint *textures);
    ~OpenGLScreen();
    
    ///Note, subclasses should override to additionally have buffers ready, etc.
    void Display();
    void SetColourScheme(const CColourIO::ColourInfo *pColourScheme);
    void Polyline(point *Points, int iNum, int iWidth, int iColour);
    void Polygon(point *points, int iNum, int iFillColour, int iOutlineColour, int iWidth);  
    void DrawRectangle(int x1, int y1, int x2, int y2, int iFillColorIndex, int iOutlineColour, int iThickness);
    void DrawCircle(screenint iCX, screenint iCY, screenint iR, int iFillColour, int iLineColour, int iLineWidth);
    
    bool MultiSizeFonts() {return true;}
    AlphabetLetter *MakeLabel(const std::string &strText,unsigned int iWrapSize=0);
    std::pair<screenint,screenint> TextSize(CDasherScreen::Label *label, unsigned int iFontSize);
    void DrawString(CDasherScreen::Label *label, screenint x, screenint y, unsigned int iFontSize, int iColour);
    //leave virtual: SendMarker, Display
  protected:
    void resize(screenint iWidth, screenint iHeight, GLshort backingWidth, GLshort backingHeight, GLfloat tc_x, GLfloat tc_y);
    void RegenerateLabels();
    ///Render a string onto a CoreGraphics context, using the context's current colour etc.
    /// \param iFontSize if 0, render on a single line, in 36pt font; any other value,
    /// render in that size, but constrained to the screen width, wrapping across multiple
    /// lines if necessary
    virtual void RenderStringOntoCGContext(NSString *string, CGContextRef context, unsigned int iFontSize)=0;
    /// Get the pixel dimensions of a string when rendered in a specified font size
    /// \param bWrap if true, string should be wrapped to the screen width, possibly
    /// over multiple lines (=> returned height will reflect this); if false,
    /// keep on one line (even if that makes it wider than the screen)
    virtual CGSize TextSize(NSString *str, unsigned int iFontSize, bool bWrap)=0;
  private:
    typedef struct {
      float r, g, b;
    } colour_t;
    
    //colours in use, we cache these as floats for feeding to OpenGL.
    colour_t *colourTable;
    
    ///Caches for circleCallbackWithCentrePoint:... (see therein)
    float circ_rad;
    GLshort *circ_coords;
    int circPoints;

    GLshort rectcoords[8];
    GLfloat texcoords[8];
    GLuint *textures;
    
  };
}