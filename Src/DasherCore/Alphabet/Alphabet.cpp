// Alphabet.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"

#include "Alphabet.h"
#include "AlphabetMap.h"

#include <iostream>

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

/////////////////////////////////////////////////////////////////////////////

CAlphabet::CAlphabet()
:m_DefaultEncoding(Opts::Western), m_Orientation(Opts::LeftToRight), m_ControlSymbol(-1), m_iGroups(0) {
  m_Characters.push_back("");
  m_Display.push_back("");
  m_Colours.push_back(-1);
  m_Foreground.push_back("");
}

/////////////////////////////////////////////////////////////////////////////

CAlphabet::CAlphabet(const CAlphIO::AlphInfo &AlphInfo)
:m_DefaultEncoding(Opts::Western), m_Orientation(Opts::LeftToRight), m_ControlSymbol(-1), m_iGroups(0) {
  m_Characters.push_back("");
  m_Display.push_back("");
  m_Colours.push_back(-1);
  m_Foreground.push_back("");

  // Set miscellaneous options

  SetOrientation(AlphInfo.Orientation);
  SetLanguage(AlphInfo.Type);
  SetTrainingFile(AlphInfo.TrainingFile);
  SetGameModeFile(AlphInfo.GameModeFile);
  SetPalette(AlphInfo.PreferredColours);

//      m_AlphInfo=&AlphInfo;

  // Add all the characters.
  for(unsigned int i = 0; i < AlphInfo.Groups.size(); i++) {    // loop groups
    CGroupAdder *pAdder = GetGroupAdder(AlphInfo.Groups[i].Colour, AlphInfo.Groups[i].Label);
    for(unsigned int j = 0; j < AlphInfo.Groups[i].Characters.size(); j++) {    // loop characters
      pAdder->AddChar(AlphInfo.Groups[i].Characters[j].Text, AlphInfo.Groups[i].Characters[j].Display, AlphInfo.Groups[i].Characters[j].Colour, AlphInfo.Groups[i].Characters[j].Foreground);
    }
    delete pAdder;
  }

  // Set Space character if requested

  // This line makes it a bit easier for our WindowsCE compiler
  std::string empty = "";

  if(AlphInfo.ParagraphCharacter.Text != empty)
    AddParagraphSymbol(AlphInfo.ParagraphCharacter.Text, AlphInfo.ParagraphCharacter.Display, AlphInfo.ParagraphCharacter.Colour, AlphInfo.ParagraphCharacter.Foreground);

  if(AlphInfo.SpaceCharacter.Text != empty)
    AddSpaceSymbol(AlphInfo.SpaceCharacter.Text, AlphInfo.SpaceCharacter.Display, AlphInfo.SpaceCharacter.Colour, AlphInfo.SpaceCharacter.Foreground);

  //-- Added for Kanji Conversion 13 July 2005 by T.Kaburagi START
  if(AlphInfo.StartConvertCharacter.Text != empty)
    AddStartConversionSymbol(AlphInfo.StartConvertCharacter.Text, AlphInfo.StartConvertCharacter.Display, AlphInfo.StartConvertCharacter.Colour, AlphInfo.StartConvertCharacter.Foreground);

  if(AlphInfo.EndConvertCharacter.Text != empty)
    AddEndConversionSymbol(AlphInfo.EndConvertCharacter.Text, AlphInfo.EndConvertCharacter.Display, AlphInfo.EndConvertCharacter.Colour, AlphInfo.EndConvertCharacter.Foreground);
  //-- Added for Kanji Conversion 13 July 2005 by T.Kaburagi END

  // DJW - now the control symbol is always a part of the alphabet
  // DasherModel knows whether or not to use it

  // FIXME - We really need to ensure that the control symbol is last in the alphabet with the current logic.

  if(AlphInfo.ControlCharacter.Display != std::string("") && GetControlSymbol() == -1)
    AddControlSymbol(AlphInfo.ControlCharacter.Text, AlphInfo.ControlCharacter.Display, AlphInfo.ControlCharacter.Colour, AlphInfo.ControlCharacter.Foreground);

#ifdef DASHER_TRACE
  Trace();
#endif
}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::GetSymbols(vector <symbol >*Symbols, string *Input, bool IsMore) const {
  string Tmp;
  symbol CurSymbol = 0, TmpSymbol = 0;
  bool KeyIsPrefix = false;
  int extras;
  unsigned int bit;

  for(unsigned int i = 0; i < Input->size(); i++) {

    Tmp = (*Input)[i];

    /* The string we've been given is in UTF-8. The symbols are
       also in UTF-8, so we need to pass the entire UTF-8 character
       which may be several bytes long. RFC 2279 describes this
       encoding */

    if((*Input)[i] & 0x80) {    // Character is more than 1 byte long
      extras = 1;
      for(bit = 0x20; ((*Input)[i] & bit) != 0; bit >>= 1)
        extras++;
      if(extras > 5) {
      }                         // Malformed character
      while(extras-- > 0) {
        Tmp += (*Input)[++i];
      }
    }

    CurSymbol = TextMap.Get(Tmp, &KeyIsPrefix);

    if(KeyIsPrefix) {
      CurSymbol = 0;
      for(; i < Input->size(); i++) {

        Tmp += (*Input)[i];

        TmpSymbol = TextMap.Get(Tmp, &KeyIsPrefix);
        if(TmpSymbol > 0) {
          CurSymbol = TmpSymbol;
        }
        if(!KeyIsPrefix) {
          if(CurSymbol != 0) {
            Symbols->push_back(CurSymbol);
          }
          else {
            i -= Tmp.size() - 1;
            //Tmp.erase(Tmp.begin(), Tmp.end());
            Tmp = "";
          }
          break;
        }
      }
    }
    else {
      if(CurSymbol != 0)
        Symbols->push_back(CurSymbol);
    }
  }

  if(IsMore)
    if(KeyIsPrefix)
      *Input = Tmp;
    else
      *Input = "";
  else if(KeyIsPrefix)
    Symbols->push_back(CurSymbol);
}

// add single char to the character set
void CAlphabet::AddChar(const string NewCharacter, const string Display, int Colour, const string Foreground) {
  m_Characters.push_back(NewCharacter);
  m_Display.push_back(Display);
  m_Colours.push_back(Colour);
  m_Foreground.push_back(Foreground);

  symbol ThisSymbol = m_Characters.size() - 1;
  TextMap.Add(NewCharacter, ThisSymbol);
}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::AddParagraphSymbol(const string NewCharacter, const string Display, int Colour, const string Foreground) {
  AddChar(NewCharacter, Display, Colour, Foreground);
  m_ParagraphSymbol = GetNumberSymbols() - 1;
}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::AddSpaceSymbol(const string NewCharacter, const string Display, int Colour, const string Foreground) {
  AddChar(NewCharacter, Display, Colour, Foreground);
  m_SpaceSymbol = GetNumberSymbols() - 1;
}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::AddControlSymbol(const string NewCharacter, const string Display, int Colour, const string Foreground) {
  AddChar(NewCharacter, Display, Colour, Foreground);
  m_ControlSymbol = GetNumberSymbols() - 1;
}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::AddStartConversionSymbol(const string NewCharacter, const string Display, int Colour, const string Foreground) {
  AddChar(NewCharacter, Display, Colour, Foreground);
  m_StartConversionSymbol = GetNumberSymbols() - 1;
}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::AddEndConversionSymbol(const string NewCharacter, const string Display, int Colour, const string Foreground) {
  AddChar(NewCharacter, Display, Colour, Foreground);
  m_EndConversionSymbol = GetNumberSymbols() - 1;
}

/////////////////////////////////////////////////////////////////////////////
// diagnostic dump of character set

void CAlphabet::Trace() const {
  int i;
  DASHER_TRACEOUTPUT("GetNumberSymbols() %d\n", GetNumberSymbols());
  DASHER_TRACEOUTPUT("GetNumberTextSymbols() %d\n", GetNumberTextSymbols());

  int iGroup = 0;
  for(i = 0; i < (int) m_Characters.size(); i++) {
    if(iGroup < m_iGroups && i == m_GroupStart[iGroup]) {
      DASHER_TRACEOUTPUT("Group %d '%s'\n", iGroup, m_GroupLabel[iGroup].c_str());
    }
    if(iGroup < m_iGroups && i == m_GroupEnd[iGroup]) {
      DASHER_TRACEOUTPUT("--------\n");
      iGroup++;
    }

    DASHER_TRACEOUTPUT("Symbol %d Character:'%s' Display:'%s'\n", i, m_Characters[i].c_str(), m_Display[i].c_str());

  }

}

/////////////////////////////////////////////////////////////////////////////

int CAlphabet::GetTextColour(symbol Symbol) {
  std::string TextColour = m_Foreground[Symbol];
  if(TextColour != std::string("")) {
    return atoi(TextColour.c_str());
  }
  else {
    return 4;
  }
}

/////////////////////////////////////////////////////////////////////////////

CAlphabet::CGroupAdder * CAlphabet::GetGroupAdder(int iColour, const std::string &strLabel) {
  return new CGroupAdder(*this, iColour, strLabel);
}

/////////////////////////////////////////////////////////////////////////////

CAlphabet::CGroupAdder::CGroupAdder(CAlphabet &alphabet, int iColour, std::string strLabel)
:m_Alphabet(alphabet) {
  m_Alphabet.m_iGroups++;
  m_Alphabet.m_GroupColour.push_back(iColour);
  m_Alphabet.m_GroupLabel.push_back(strLabel);
  m_Alphabet.m_GroupStart.push_back(m_Alphabet.GetNumberSymbols());

}

/////////////////////////////////////////////////////////////////////////////

CAlphabet::CGroupAdder::~CGroupAdder() {
  m_Alphabet.m_GroupEnd.push_back(m_Alphabet.GetNumberSymbols());

}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::CGroupAdder::AddChar(const std::string NewCharacter, const std::string Display, int Colour, const std::string Foreground) {
  m_Alphabet.AddChar(NewCharacter, Display, Colour, Foreground);

}

/////////////////////////////////////////////////////////////////////////////
