// CustomColours.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////


#include "CustomColours.h"
using namespace Dasher;
using namespace std;

CCustomColours::CCustomColours(const CColourIO::ColourInfo& ColourInfo)
{
	m_ColourInfo=&ColourInfo;

	// Add all the colours.
	for (unsigned int i=0; i<ColourInfo.Reds.size(); i++) { // loop colours
	  m_Red.push_back(ColourInfo.Reds[i]);
	  m_Green.push_back(ColourInfo.Greens[i]);
	  m_Blue.push_back(ColourInfo.Blues[i]);
	}
}
