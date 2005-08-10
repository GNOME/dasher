
// DasherButtons.h 
// Copyright 2005 by Chris Ball

#ifndef __DASHER_BUTTONS_H__
#define __DASHER_BUTTONS_H__

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "Alphabet/Alphabet.h"
#include <algorithm>
#include "DasherComponent.h"
#include "Event.h"

using namespace std;

class CDasherButtons
{
public:
  CDasherButtons(CSettingsStore *pSettingsStore);

 ~CDasherButtons();

  void                    InitMemberVars();
  void                        SetupBoxes();
  void                          GetBoxes();
  struct box {
    int top;
	int bottom;
	int zoomloc;
	int zoomfactor;
  };
  
  struct box boxes_ptr;

 protected:
  CSettingsStore*  m_pSettingsStore;

  int r, B, Z, p, displayedtop, displayedbottom, zoom, s, pdash, displayedcentre, truecentre;
};

#endif
