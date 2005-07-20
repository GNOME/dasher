// DasherViewSquare.h
//
// Copyright (c) 2001-2004 David Ward

#ifndef __DasherViewSquare_h__
#define __DasherViewSquare_h__

#include "DasherScreen.h"
#include "DasherModel.h"
#include "DasherView.h"
#include "View/DelayedDraw.h"

namespace Dasher {
  class CDasherViewSquare;
}
////// An implementation of the DasherView class
////// This class renders Dasher in the vanilla style,
/// but with horizontal and vertical mappings
////// Horizontal mapping - linear and log
/// Vertical mapping - linear with different gradient

class Dasher::CDasherViewSquare:public Dasher::CDasherView
{
public:
  CDasherViewSquare(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherScreen * DasherScreen, CDasherModel & DasherModel);
  void TapOnDisplay(screenint mousex, screenint mousey, unsigned long Time);
  void GoTo(screenint mousex, screenint mousey);

  virtual void RenderNodes();

  void ChangeScreen(CDasherScreen * NewScreen);
  void CDasherViewSquare::DrawGoTo(screenint mousex, screenint mousey);
  void DrawMouse(screenint mousex, screenint mousey);
  void DrawMouseLine(screenint mousex, screenint mousey);
  void DrawKeyboard();
  void DrawGameModePointer();
  virtual bool HandleStartOnMouse(int iTime);

  virtual void HandleEvent(Dasher::CEvent * pEvent);

  int RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme, myint y1, myint y2, int &mostleft, const std::string & displaytext);

  // Responsible for all the Render_node calls
  int RecursiveRender(CDasherNode * Render, myint y1, myint y2, int mostleft);

  // Displays some nodes inside one parent node. Used to group capital letters, accents, punctuation etc.
  void RenderGroups(CDasherNode * Render, myint y1, myint y2, int mostleft);

  int GetAutoOffset() const;

  virtual void ResetSum();
  virtual void ResetSumCounter();
  virtual void ResetYAutoOffset();

private:

  void Input2Dasher(screenint iInputX, screenint iInputY, myint & iDasherX, myint & iDasherY, int iType, int iMode);
  void Screen2Dasher(screenint iInputX, screenint iInputY, myint & iDasherX, myint & iDasherY);
  void Dasher2Screen(myint iDasherX, myint iDasherY, screenint & iScreenX, screenint & iScreenY);
  void Dasher2OneD(myint & iDasherX, myint & iDasherY);
  void Dasher2Eyetracker(myint & iDasherX, myint & iDasherY);

  void TruncateToScreen(screenint & iX, screenint & iY);

  void DasherPolyline(myint * x, myint * y, int n, int iColour);
  void DasherPolygon(myint * x, myint * y, int n, int iColour);
  void DasherDrawRectangle(myint iLeft, myint iTop, myint iRight, myint iBottom, const int Color, Opts::ColorSchemes ColorScheme,bool bDrawOutline);
  void DasherDrawCentredRectangle(myint iDasherX, myint iDasherY, screenint iSize, const int Color, Opts::ColorSchemes ColorScheme, bool bDrawOutline);
  void DasherDrawText(myint iAnchorX1, myint iAnchorY1, myint iAnchorX2, myint iAnchorY2, const std::string & sDisplayText, int &mostleft);

  myint DasherVisibleMinY();
  myint DasherVisibleMaxY();

  myint DasherVisibleMaxX();

  myint m_iDasherXCache;
  myint m_iDasherYCache;

  // Class definitions

  class Cymap {
  public:
    Cymap(myint iScale);
    Cymap() {}
    myint map(myint y) const;
    myint unmap(myint y) const;
  private:
    myint m_Y1, m_Y2, m_Y3;
  };

  // the x and y non-linearities
  void screen2dasher(screenint mousex, screenint mousey, myint * dasherx, myint * dashery) const;
  void AutoCalibrate(screenint * mousex, screenint * mousey);
  screenint dasherx2screen(myint sx) const;
  screenint dashery2screen(myint sy) const;
  Cint32 dashery2screen(myint y1, myint y2, screenint & s1, screenint & s2) const;

  double eyetracker_get_x(double x, double y);
  double eyetracker_get_y(double x, double y);
  double xmax(double x, double y) const;
  double xmap(double x) const;
  double ixmap(double x) const;

  void CheckForNewRoot();
  inline void Crosshair(myint sx);

  // Data

  bool bInBox;                  // Whether we're in the mouseposstart box
  int iBoxStart;                // Time that the current box was drawn
  int iBoxEntered;              // Time when the user enttered the current box

  CDelayedDraw m_DelayDraw;

  double m_dXmpa, m_dXmpb, m_dXmpc, m_dXmpd;
  screenint CanvasX, CanvasY, CanvasBorder;

  int m_ySum, m_ySumCounter, m_yFilterTimescale, m_ySigBiasPixels, m_ySigBiasPercentage, m_yAutoOffset;

  Cymap m_ymap;

};

#include "DasherViewSquare.inl"

#endif /* #ifndef __DasherViewSquare_h__ */
