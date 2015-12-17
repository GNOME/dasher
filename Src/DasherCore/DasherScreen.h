// DasherScreen.h
//
// Copyright (c) 2001-2004 David Ward

#ifndef __DasherScreen_h_
#define __DasherScreen_h_

#include "DasherTypes.h"
#include "../DasherCore/ColourIO.h"
#include <set>

// DJW20050505 - renamed DrawText to DrawString - windows defines DrawText as a macro and it's 
// really hard to work around
// Also make CDasher screen operate in UTF8 strings only

namespace Dasher {
  class CDasherScreen;
  class CLabelListScreen;
  class CDasherInterfaceBase;
}

/// \ingroup View
/// @{
/// Abstract interface for drawing operations, implemented by platform-specific canvases.
/// Instances have _mutable_ dimensions; changes should be reported to the
/// interface's ScreenResized method.
/// Note the DrawString and TextSize methods: these now take platform-specific
/// Label objects returned from MakeLabel. Thus, it is up to external clients to
/// cache and reuse Labels. (This replaces the previous scheme where these methods
/// took arbitrary std::strings, which were cached in a hashmap internal to each
/// platform's screen. The new scheme allows clients to control cache preloading
/// and flushing.)
class Dasher::CDasherScreen
{
public:
  //! \param width Width of the screen
  //! \param height Height of the screen
  CDasherScreen(screenint width, screenint height)
  :m_iWidth(width), m_iHeight(height) {
  } 

  virtual ~ CDasherScreen() {
  }

/*   //! Set the widget interface used for communication with the core */
/*   virtual void SetInterface(CDasherInterfaceBase * DasherInterface) { */
/*     m_pDasherInterface = DasherInterface; */
/*   } */

  //! Return the width of the screen
  screenint GetWidth() {
    return m_iWidth;
  }
  //! Return the height of the screen screen
  int GetHeight() {
    return m_iHeight;
  }
  //! Structure defining a point on the screen 
  typedef struct tagpoint {
    screenint x;
    screenint y;
  } point;
  
  /// (Default implementation returns false)
  ///\return true if this Screen can efficiently support fonts of many sizes (by continuous scaling);
  /// false if clients should try to minimise the number of distinct font sizes required.
  virtual bool MultiSizeFonts() {return false;}
  
  ///Abstract class for objects representing strings that can be drawn on the screen.
  /// Platform-specific instances are created by the MakeLabel(String) method, which
  /// may then be passed to GetSize() and DrawText().
  class Label {
    friend class CDasherScreen;
  protected:
    Label(const std::string &strText, unsigned int iWrapSize)
    : m_strText(strText), m_iWrapSize(iWrapSize) {};
  public:
    const std::string m_strText;
    ///If 0, Label is to be rendered on a single line.
    /// Any other value, Label need only be renderable at that size, but should 
    /// be _wrapped_ to fit the screen width. (It is up to platforms to decide
    /// whether to support DrawString/TextSize at any other size but this is
    /// NOT required.)
    unsigned int m_iWrapSize;
    ///Delete the label. This should free up any resources associated with
    /// drawing the string onto the screen, e.g. layouts or textures.
    virtual ~Label() {}
  };

  ///Make a label for use with this screen.
  /// \param strText UTF8-encoded text.
  /// \param iWrapSize 0 => create a Label that will be rendered on a single line,
  /// potentially at multiple sizes; appropriate for DasherNode labels.
  /// Any other value => Label SHOULD ONLY BE USED AT THAT SIZE, but should 
  /// be _wrapped_ onto multiple lines if necessary to fit within the screen width.
  /// (DrawString/TextSize with any other font size may produce unpredictable results,
  /// depending on platform.)
  virtual Label *MakeLabel(const std::string &strText, unsigned int iWrapSize=0) {return new Label(strText,iWrapSize);}

  ///Get Width and Height of a Label previously created by MakeLabel. Note behaviour
  /// undefined if the Label is not one returned from a call to MakeLabel _on_this_Screen_.
  virtual std::pair<screenint,screenint> TextSize(Label *label, unsigned int iFontSize) = 0;

  /// Draw a label at position (x1,y1)
  /// \param label a Label previously created by MakeLabel. Note behaviour
  /// undefined if the Label is not one returned from a call to MakeLabel _on_this_Screen_.
  /// \param x Coordinate of top left corner (i.e., left hand side)
  /// \param y Coordinate of top left corner (i.e., top)
  virtual void DrawString(Label *label, screenint x, screenint y, unsigned int iFontSize, int iColour) = 0;

  // Send a marker to indicate 'phases' of drawing. 

  virtual void SendMarker(int /*iMarker*/ ) {
  };

  /// Draw a filled rectangle
  ///
  /// Draw a coloured rectangle on the screen
  /// \param x1 top left of rectangle (x coordinate)
  /// \param y1 top left corner of rectangle (y coordinate)
  /// \param x2 bottom right of rectangle (x coordinate)
  /// \param y2 bottom right of rectangle (y coordinate)
  /// \param Colour the colour to be used (numeric), or -1 for no fill
  /// \param iOutlineColour The colour for the node outlines; -1 = use default
  /// \param iThickness Line thickness for outline; <1 for no outline
  virtual void DrawRectangle(screenint x1, screenint y1, screenint x2, screenint y2, int Colour, int iOutlineColour, int iThickness) = 0;

  ///Draw a circle, potentially filled and/or outlined
  /// \param iFillColour colour in which to fill; -1 for no fill
  /// \param iLineColour colour to draw outline; -1 = use default
  /// \param iLineWidth line width for outline; <1 for no outline
  virtual void DrawCircle(screenint iCX, screenint iCY, screenint iR, int iFillColour, int iLineColour, int iLineWidth) = 0;

  /// Draw a line of fixed colour (usually black). Intended for static UI elements such as a cross-hair
  /// Draw a line between each of the points in the array
  ///
  /// \param Points an array of points
  /// \param Number the number of points in the array
  /// \param iWidth The line width
  /// \todo This is dumb - why does this need to be a separate function to the coloured version?

  virtual void Polyline(point * Points, int Number, int iWidth) {
    Polyline(Points, Number, iWidth, 0);
  };

  // Draw a line of arbitrary colour.
  //! Draw a line between each of the points in the array
  //!
  //! \param Points an array of points
  //! \param Number the number of points in the array
  //! \param iWidth Width of the line
  //! \param Colour the colour to be drawn

  virtual void Polyline(point * Points, int Number, int iWidth, int Colour) = 0;

  /// Draw a polygon - given vertices and colour id
  ///
  /// \param Points Vertices of polygon in clockwise order. (No need to repeat the first point at the end)
  /// \param Number number of points in the array
  /// \param fillColour colour to fill polygon (numeric); -1 for don't fill
  /// \param outlineColour colour to draw polygon outline (right the way around, i.e. repeating first point)
  /// \param lineWidth thickness of outline; 0 or less => don't draw outline.
  virtual void Polygon(point * Points, int Number, int fillColour, int outlineColour, int lineWidth) = 0;

  //! Signal that a frame is finished - the screen should be updated
  virtual void Display() = 0;

  /// Set a colour scheme
  ///
  /// \param pColourScheme A colour scheme that should be used
  virtual void SetColourScheme(const Dasher::CColourIO::ColourInfo *pColourScheme) = 0;
  
  // Returns true if cursor is over visible part of this window.
  virtual bool IsWindowUnderCursor() = 0;

private:
  //! Width and height of the screen
  screenint m_iWidth, m_iHeight;

protected:
  ///Subclasses should call this if the canvas dimensions have changed.
  /// It is up to subclasses to make sure they also call
  /// ScreenResized on the intf.
  void resize(screenint width, screenint height) {
    m_iWidth = width; m_iHeight = height;
  }
};

/// Subclass that preserves a list of all labels returned from MakeLabel
/// (and not yet deleted) so that they can be mutated en mass (by further
/// subclasses) if necessary. Note we have to return a new Labels each time,
/// and cannot hash/flyweight together similar Labels, because _clients_ are
/// in control of deletion.
class Dasher::CLabelListScreen : public Dasher::CDasherScreen {
protected:
  CLabelListScreen(screenint width, screenint height) : CDasherScreen(width,height) {
  }
  class Label : public CDasherScreen::Label {
  public: //to instances of CLabelListScreen and subclasses
    Label(CLabelListScreen *pScreen, const std::string &strText, unsigned int iWrapSize)
    : CDasherScreen::Label(strText, iWrapSize), m_pScreen(pScreen) {
      m_pScreen->m_sLabels.insert(this);
    }
    ~Label() {
      std::set<Label *>::iterator it = m_pScreen->m_sLabels.find(this);
      DASHER_ASSERT(it != m_pScreen->m_sLabels.end());
      m_pScreen->m_sLabels.erase(it);
    }
    CLabelListScreen * const m_pScreen;
  };
  ///An iterator pointing to the first extant (non-deleted) label created
  /// from this screen. This allows iteration through modifiable labels,
  /// but without being able to access or hence modify the set.
  std::set<Label *>::iterator LabelsBegin() {return m_sLabels.begin();}

  ///An iterator pointing just beyond the last extant (non-deleted) label
  /// created from this screen. This allows iteration through modifiable labels,
  /// but without being able to access or hence modify the set.
  std::set<Label *>::iterator LabelsEnd() {return m_sLabels.end();}
private:
  std::set<Label *> m_sLabels;
};
/// @}

#endif /* #ifndef __DasherScreen_h_ */
