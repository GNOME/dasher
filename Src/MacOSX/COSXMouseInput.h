/*
 *  COSXMouseInput.h
 *  Dasher
 *
 *  Created by Doug Dickinson on 21/11/2006.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#import "../DasherCore/DasherInput.h"
#import "../DasherCore/DasherTypes.h"

#import <iostream>

using namespace Dasher;

class COSXMouseInput : public CScreenCoordInput {
public:
  COSXMouseInput() 
  : CScreenCoordInput(0, "Mouse Input") {
  };
  virtual bool GetScreenCoords(screenint &iX, screenint &iY, CDasherView *pView) {
    iX = m_iX;
    iY = m_iY;
    return true;
  }
  
  void SetCoordinates(myint _iX, myint _iY) {
    m_iX = _iX;
    m_iY = _iY;
  };
  
private:
    myint m_iX;
  myint m_iY;
  
};

static SModuleSettings sSettings[] = {
  {LP_YSCALE, T_LONG, 10, 2000, 1, 1, "Pixels covering Y range:"}
};

class COSX1DMouseInput:public CDasherCoordInput {
public:
  COSX1DMouseInput() 
  : CDasherCoordInput(2, "One Dimensional Mouse Input") {
    
    m_iOffset = 0;
  };
  
  virtual bool GetDasherCoords(myint &iDasherX, myint &iDasherY, CDasherView *pView) {
    iDasherX = 0;
    iDasherY = m_iY - m_iOffset;
    return true;
  };
  
  virtual void SetMaxCoordinates(int iN, myint * iDasherMax) {
    
    // FIXME - need to cope with the more general case here
    
    m_iDasherMaxX = iDasherMax[0];
    m_iDasherMaxY = iDasherMax[1];
  };
  
  void SetCoordinates(myint _iX, myint _iY) {
    m_iX = _iX;
    m_iY = _iY;
  };
  
  void KeyDown(unsigned long iTime, int iId) {
    if(iId == 10) {
      m_iOffset = m_iY - 2048;
    }
  };
  
  bool GetSettings(SModuleSettings **pSettings, int *iCount) {
    *pSettings = sSettings;
    *iCount = sizeof(sSettings) / sizeof(SModuleSettings);
    
    return true;
  };
  
private:
    
    myint m_iDasherMaxX;
  myint m_iDasherMaxY;
  
  myint m_iX;
  myint m_iY;
  
  myint m_iOffset;
};
