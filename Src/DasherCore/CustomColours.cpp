// CustomColours.cpp
//
// Copyright (c) 2002 Iain Murray

#include "../Common/Common.h"

#include "CustomColours.h"
using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CCustomColours::CCustomColours(const CColourIO::ColourInfo &ColourInfo) {
  m_ColourInfo = &ColourInfo;

  // Add all the colours.
  for(unsigned int i = 0; i < ColourInfo.Reds.size(); i++) {    // loop colours
    m_Red.push_back(ColourInfo.Reds[i]);
    m_Green.push_back(ColourInfo.Greens[i]);
    m_Blue.push_back(ColourInfo.Blues[i]);
  }
}
