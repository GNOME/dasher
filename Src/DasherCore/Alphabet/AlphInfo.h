// AlphIO.h
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

#ifndef __ALPHINFO_H__
#define __ALPHINFO_H__

#include "../../Common/Common.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../DasherTypes.h"
#include "AlphabetMap.h"
#include "GroupInfo.h"
#include "AlphIO.h"

#include <string>
#include <vector>
#include <utility>              // for std::pair

namespace Dasher {
  class CAlphInfo;
  class CAlphIO;
} 

/// \ingroup Alphabet
/// @{

/// This structure completely describes the characters used in alphabet.
/// It maps from the "symbol" type (integers, starting at 1 for alphabet
/// characters, with 0 used for error / "unknown") to display text (that
/// rendered onto the canvas - e.g. a box or "_" for a space character),
/// text (that which is written/output, or indeed, read in by CAlphabetMap),
/// colour (an index into the current colour scheme; note values below 130
/// are increased by 130 on alternate offsets, this is known as the "phase"),
/// also foreground colour information (but these does not seem to be used
/// ATM).
///
/// One CAlphInfo object is created per alphabet when the alphabet.*.xml
/// files are read in by CAlphIO, and a CAlphabetMap object is created for
/// the alphabet currently in use (and deleted when the alphabet is changed).
class Dasher::CAlphInfo {
private:
  friend class CAlphIO;
  struct character {
    character();
    std::string Display;
    std::string Text;
    int Colour;
    std::string Foreground;
  };  
public:
  /// Return number of text symbols - inc space and para, but no control/conversion start/end
  /// Note symbol numbers are 1-indexed; 0 is reserved to indicate an "unknown symbol" (-1 = End-Of-Stream),
  /// and for element 0 of the probability array to contain a 0.
  int GetNumberTextSymbols() const {return m_vCharacters.size();}
  
  Opts::ScreenOrientations GetOrientation() const {return Orientation;} 
  
  Opts::AlphabetTypes GetType() const {return Type;}
  
  const std::string & GetTrainingFile() const {return TrainingFile;}
  
  const std::string &GetGameModeFile() const {return GameModeFile;}
  
  const std::string & GetPalette() const {return PreferredColours;}
  
  symbol GetParagraphSymbol() const {return iParagraphCharacter;}
  
  symbol GetSpaceSymbol() const {return iSpaceCharacter;}
  
  //symbol GetStartConversionSymbol() const;
  //symbol GetEndConversionSymbol() const;
  
  /// return display string for i'th symbol
  const std::string & GetDisplayText(symbol i) const {return m_vCharacters[i-1].Display;}
  
  /// return text for edit box for i'th symbol
  const std::string & GetText(symbol i) const {return m_vCharacters[i-1].Text;} 
  
  // return string for i'th symbol
  
  int GetColour(symbol i, int iPhase) const;
  
  /// Text foreground colour for i'th symbol; default 4 if unspecified
  int GetTextColour(symbol i) const;
  /// Text foreground colour for i'th symbol, as specified in XML
  const std::string & GetForeground(symbol i) const {return m_vCharacters[i-1].Foreground;}
    
  const std::string &GetDefaultContext() const {return m_strDefaultContext;}
  
  SGroupInfo *m_pBaseGroup;
  int iNumChildNodes;
  int m_iConversionID;
  
  CAlphabetMap *MakeMap() const;
  
  ~CAlphInfo();
  
private:
  CAlphInfo();
  // Basic information
  std::string AlphID;
  bool Mutable;               // If from user we may play. If from system defaults this is immutable. User should take a copy.
  
  // Complete description of the alphabet:
  std::string TrainingFile;
  std::string GameModeFile;
  std::string PreferredColours;
  Opts::AlphabetTypes Encoding;
  Opts::AlphabetTypes Type;
  Opts::ScreenOrientations Orientation;
    
  /*     // Obsolete groups stuff */
  /*     struct group { */
  /*       std::string Description; */
  /*       std::vector < character > Characters; */
  /*       int Colour; */
  /*       std::string Label; */
  /*     }; */
  /*     std::vector < group > Groups; */
  /*     // --- */
    
  std::vector<character> m_vCharacters;
  
  symbol iParagraphCharacter;       // symbol number (index into m_vCharacters +1) of paragraph char (for display and default edit-text), 0 for none.
  symbol iSpaceCharacter;   // symbol number (index into m_vCharacters +1) of space char (display and edit text), 0 for none.
  character *ControlCharacter; // display and edit text of Control character. Typically ("", "Control"). Use ("", "") if no control character.
  character *StartConvertCharacter;
  character *EndConvertCharacter;
  
  std::string m_strDefaultContext;
};


/// @}

#endif /* #ifndef __AlphIO_h__ */
