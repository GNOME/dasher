// AlphInfo.cpp
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "AlphInfo.h"

#include <iostream>
#include <cstring>

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CAlphInfo::CAlphInfo() {
  iSpaceCharacter=0;
  iParagraphCharacter = 0;
  ControlCharacter=NULL;
  StartConvertCharacter=NULL;
  EndConvertCharacter=NULL;
  //Members of SGroupInfo:
  pChild=pNext=NULL; iStart=iEnd=1; bVisible=true;
  iNumChildNodes = 0;
  
  m_iConversionID = 0; m_strConversionTrainStart="<"; m_strConversionTrainStop=">";
  m_strDefaultContext = ". ";
  m_strCtxChar = "§";
}

string CAlphInfo::escape(const string &ch) const {
  if ((m_strConversionTrainStart.length() && ch==m_strConversionTrainStart)
      || (m_strCtxChar.length() && ch==m_strCtxChar))
    return ch+ch;
  return ch;
}

CAlphInfo::~CAlphInfo() {
  pChild->RecursiveDelete();
  pNext->RecursiveDelete();
}

void CAlphInfo::copyCharacterFrom(const CAlphInfo *other, int idx) {
  m_vCharacters.push_back(other->m_vCharacters[idx-1]);
}

CAlphInfo::character::character() {
  Display="";
  Text="";
  Colour=-1;
}
