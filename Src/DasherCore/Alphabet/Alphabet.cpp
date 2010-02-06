// Alphabet.cpp
//
// Copyright (c) 2008 The Dasher Team
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "../../Common/Common.h"
#include "../TrainingHelper.h"
#include "Alphabet.h"
#include "AlphabetMap.h"
#include <cstring>
#include <sstream>

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
:m_DefaultEncoding(Opts::Western), m_Orientation(Opts::LeftToRight), m_ControlSymbol(-1) {
  m_Characters.push_back("");
  m_Display.push_back("");
  m_Colours.push_back(-1);
  m_Foreground.push_back("");
}

/////////////////////////////////////////////////////////////////////////////

CAlphabet::CAlphabet(const CAlphIO::AlphInfo &AlphInfo)
:m_DefaultEncoding(Opts::Western), m_Orientation(Opts::LeftToRight), m_ControlSymbol(-1) {
  m_Characters.push_back("");
  m_Display.push_back("");
  m_Colours.push_back(-1);
  m_Foreground.push_back("");

  m_StartConversionSymbol = -1;
  m_EndConversionSymbol = -1;

  m_strDefaultContext = AlphInfo.m_strDefaultContext;

  // Set miscellaneous options

  SetOrientation(AlphInfo.Orientation);
  SetLanguage(AlphInfo.Type);
  SetTrainingFile(AlphInfo.TrainingFile);
  SetGameModeFile(AlphInfo.GameModeFile);
  SetPalette(AlphInfo.PreferredColours);

  for(std::vector<CAlphIO::AlphInfo::character>::const_iterator it(AlphInfo.m_vCharacters.begin()); it != AlphInfo.m_vCharacters.end(); ++it)
    AddChar(it->Text, it->Display, it->Colour, it->Foreground);


  // TODO: Special characters are a mess - really need to think these through

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



  // New group stuff

  m_pBaseGroup = AlphInfo.m_pBaseGroup;
  iNumChildNodes = AlphInfo.iNumChildNodes;
#ifdef DASHER_TRACE
  Trace();
#endif
}

/////////////////////////////////////////////////////////////////////////////

CAlphabet::utf8_length::utf8_length()
{
  int i;

  memset(utf8_count_array, 0, sizeof(utf8_count_array));
  for (i = 0x00; i <= 0x7f; ++i) utf8_count_array[i] = 1;
  for (i = 0xc0; i <= 0xdf; ++i) utf8_count_array[i] = 2;
  for (i = 0xe0; i <= 0xef; ++i) utf8_count_array[i] = 3;
  for (i = 0xf0; i <= 0xf7; ++i) utf8_count_array[i] = 4;
  max_length = 4;
/* The following would be valid according to RFC 2279 which was rendered
 * obsolete by RFC 3629
 * for (i = 0xf8; i <= 0xfb; ++i) utf8_count_array[i] = 5;
 * for (i = 0xfc; i <= 0xfd; ++i) utf8_count_array[i] = 6;
 * max_length = 6;
 *
 * and from RFC 3629:
 * o  The octet values C0, C1, F5 to FF never appear.
 */
  utf8_count_array[0xc0] = utf8_count_array[0xc1] = 0;
  for (i = 0xf5; i <= 0xff; ++i) utf8_count_array[i] = 0;
}

CAlphabet::utf8_length CAlphabet::m_utf8_count_array;

int CAlphabet::utf8_length::operator[](const unsigned char i) const
{
  return utf8_count_array[i];
}

CAlphabet::SymbolStream::SymbolStream(const CAlphabet *pAlph, std::istream &_in)
: map(pAlph->TextMap), in(_in), pos(0), len(0) {
  readMore();
}

void CAlphabet::SymbolStream::readMore() {
  //len is first unfilled byte
  in.read(&buf[len], 1024-len);
  if (in.good()) {
    DASHER_ASSERT(in.gcount() == 1024-len);
    len = 1024;
  } else {
    len+=in.gcount();
    DASHER_ASSERT(len<1024);
    //next attempt to read more will fail.
  }
}

symbol CAlphabet::SymbolStream::next()
{
  int numChars;

  for (;;) {
    if (pos + m_utf8_count_array.max_length > len && len==1024) {
    //may need more bytes for next char; and input not yet exhausted.

      if (pos) {
        //shift remaining bytes to beginning
        len-=pos; //len of them
        memcpy(buf, &buf[pos], len);
        pos=0;
      }
      readMore();
    }
  //if still don't have any chars after attempting to read more...EOF!
    if (pos==len) return -1;
    numChars = m_utf8_count_array[buf[pos]];
    if (numChars != 0) break;
#ifdef DEBUG
    std::cerr << "Read invalid UTF-8 character 0x" << hex << buf[pos]
              << dec << std::endl;
#endif
    ++pos;
  }
  if (numChars == 1)
    return map.GetSingleChar(buf[pos++]);
  if (pos+numChars > len) {
    //no more bytes in file (would have tried to read earlier), but not enough for char
#ifdef DEBUG
    std::cerr << "Incomplete UTF-8 character beginning 0x" << hex << buf[pos] << dec;
    std::cerr << "(expecting " << numChars << " bytes but only " << (len-pos) << ")" << std::endl;
#endif
    pos=len;
    return -1;
  }
  int sym=map.Get(string(&buf[pos], numChars));
  pos+=numChars;
  return sym;
}

void CAlphabet::GetSymbols(std::vector<symbol>& Symbols, const std::string& Input) const
{
  std::istringstream in(Input);
  SymbolStream syms(this, in);
  for (symbol sym; (sym=syms.next())!=-1;)
    Symbols.push_back(sym);
}

// add single char to the character set
void CAlphabet::AddChar(const std::string NewCharacter, const std::string Display, int Colour, const std::string Foreground) {
  m_Characters.push_back(NewCharacter);
  m_Display.push_back(Display);
  m_Colours.push_back(Colour);
  m_Foreground.push_back(Foreground);

  symbol ThisSymbol = m_Characters.size() - 1;
  TextMap.Add(NewCharacter, ThisSymbol);
}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::AddParagraphSymbol(const std::string NewCharacter, const std::string Display, int Colour, const std::string Foreground) {
  AddChar(NewCharacter, Display, Colour, Foreground);
  m_ParagraphSymbol = GetNumberSymbols() - 1;
}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::AddSpaceSymbol(const std::string NewCharacter, const std::string Display, int Colour, const std::string Foreground) {
  AddChar(NewCharacter, Display, Colour, Foreground);
  m_SpaceSymbol = GetNumberSymbols() - 1;
}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::AddControlSymbol(const std::string NewCharacter, const std::string Display, int Colour, const std::string Foreground) {
  AddChar(NewCharacter, Display, Colour, Foreground);
  m_ControlSymbol = GetNumberSymbols() - 1;
}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::AddStartConversionSymbol(const std::string NewCharacter, const std::string Display, int Colour, const std::string Foreground) {
  AddChar(NewCharacter, Display, Colour, Foreground);
  m_StartConversionSymbol = GetNumberSymbols() - 1;
}

/////////////////////////////////////////////////////////////////////////////

void CAlphabet::AddEndConversionSymbol(const std::string NewCharacter, const std::string Display, int Colour, const std::string Foreground) {
  AddChar(NewCharacter, Display, Colour, Foreground);
  m_EndConversionSymbol = GetNumberSymbols() - 1;
}


/////////////////////////////////////////////////////////////////////////////
// diagnostic dump of character set

void CAlphabet::Trace() const {
//   int i;
//   DASHER_TRACEOUTPUT("GetNumberSymbols() %d\n", GetNumberSymbols());
//   DASHER_TRACEOUTPUT("GetNumberTextSymbols() %d\n", GetNumberTextSymbols());

//   int iGroup = 0;
//   for(i = 0; i < (int) m_Characters.size(); i++) {
//     if(iGroup < m_iGroups && i == m_GroupStart[iGroup]) {
//       DASHER_TRACEOUTPUT("Group %d '%s'\n", iGroup, m_GroupLabel[iGroup].c_str());
//     }
//     if(iGroup < m_iGroups && i == m_GroupEnd[iGroup]) {
//       DASHER_TRACEOUTPUT("--------\n");
//       iGroup++;
//     }

//     DASHER_TRACEOUTPUT("Symbol %d Character:'%s' Display:'%s'\n", i, m_Characters[i].c_str(), m_Display[i].c_str());

//   }

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

int 
CAlphabet::GetColour(symbol i, int iPhase) const {
  int iColour = m_Colours[i];
  
  // This is for backwards compatibility with old alphabet files -
  // ideally make this log a warning (unrelated TODO: automate
  // validation of alphabet files, plus maintenance of repository
  // etc.)
  if(iColour == -1) {
    if(i == m_SpaceSymbol) {
      iColour = 9;
    }
    else {
      iColour = (i % 3) + 10;
    }
  }
  
  // Loop on low colours for nodes (TODO: go back to colour namespaces?)
  if(iPhase == 1 && iColour < 130)
    iColour += 130;
  
  return iColour;
} 

