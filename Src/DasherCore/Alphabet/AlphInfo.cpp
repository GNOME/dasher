// AlphIO.cpp
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

#include "AlphIO.h"

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

int CAlphInfo::GetTextColour(symbol Symbol) const {
  const std::string &TextColour(GetForeground(Symbol));
  if(TextColour != std::string("")) {
    return atoi(TextColour.c_str());
  }
  else {
    return 4;
  }
}

CAlphabetMap *CAlphInfo::MakeMap() const {
  CAlphabetMap *map = new CAlphabetMap();
  if (iParagraphCharacter!=0) map->AddParagraphSymbol(iParagraphCharacter);
  int i;
  for(i = 0; i < m_vCharacters.size(); i++) {
    if (i+1!=iParagraphCharacter) map->Add(m_vCharacters[i].Text, i+1); //1-indexed
  }
  /*ACL I'm really not sure where conversion characters should/shouldn't be included.
     They seemed to be included in the Alphabet Map, i.e. for reading training text via GetSymbols;
     but a TODO comment suggested they should _not_ be included in GetNumberSymbols(),
     and I couldn't find any code which would have called e.g. GetText on them.
     Moreover, if these characters are put into the AlphabetMap, they'll be fed into the
     LanguageModel just as any other "symbol", but with an out-of-bounds symbol number!
     (So maybe the range of allowed symbol numbers is wrong?). Hence, not including them atm.
     If they were needed, we could do something like the following:
        if (StartConvertCharacter)
          map->Add(StartConvertCharacter->Text, ++i);
        if (EndConvertCharacter)
          map->Add(EndConvertCharacter->Text, ++i);
  */
  return map;
}

CAlphInfo::CAlphInfo() {
  iSpaceCharacter=0;
  iParagraphCharacter = 0;
  ControlCharacter=NULL;
  StartConvertCharacter=NULL;
  EndConvertCharacter=NULL;
  m_pBaseGroup = 0;
  iNumChildNodes = 0;
  m_iConversionID = 0;
  m_strDefaultContext = ". ";
  m_strCtxChar = "§";
  m_bHidden=false;
}

void DeleteGroups(SGroupInfo *Info) {
  for(SGroupInfo *next; Info; Info=next) {
    next = Info->pNext;
    DeleteGroups(Info->pChild);
    delete Info;
    Info = next;
  }
}

CAlphInfo::~CAlphInfo() {
  DeleteGroups(m_pBaseGroup);
}

CAlphInfo::character::character() {
  Display="";
  Text="";
  Colour=-1;
  Foreground="";
}
