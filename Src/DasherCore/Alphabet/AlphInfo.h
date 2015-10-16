// AlphInfo.h
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
///
/// Note the group structure stored by inheriting from SGroupInfo; these are filled
/// with iStart==1 (as symbol numbers are 1-indexed; 0 is reserved to indicate an
/// "unknown symbol", and for element 0 of the prob. array to contain a 0, and
/// symbol -1 indicates End-Of-Stream), and iEnd == one more than the number of
/// "text" symbols (i.e. inc space and para, but no control/conversion start/end)
/// - this is for consistency with SGroupInfo, preserving that iEnd is one more
/// than the highest valid index.
class Dasher::CAlphInfo : public SGroupInfo {
public:
  ///Format a character ready to write to a training file, by doubling
  /// up any escape character (context-switch / conversion-start)
  std::string escape(const std::string &ch) const;
  
  const std::string &GetID() const {return AlphID;}

  Opts::ScreenOrientations GetOrientation() const {return Orientation;}

  Opts::AlphabetTypes GetType() const {return Type;}

  const std::string & GetTrainingFile() const {return TrainingFile;}

  const std::string &GetGameModeFile() const {return GameModeFile;}

  const std::string & GetPalette() const {return PreferredColours;}
  
  const std::string & GetLanguageCode() const {return LanguageCode;}

  symbol GetParagraphSymbol() const {return iParagraphCharacter;}

  ///Space symbol is special in three ways:
  /// (1) defines word boundaries for speak-as-we-go, i.e. we speak when we see a space;
  /// (2) Unknown characters in game mode text file are converted into spaces;
  /// (3) Default colour is 9 if none specified
  symbol GetSpaceSymbol() const {return iSpaceCharacter;}

  //symbol GetStartConversionSymbol() const;
  //symbol GetEndConversionSymbol() const;

  /// return display string for i'th symbol
  const std::string & GetDisplayText(symbol i) const {return m_vCharacters[i-1].Display;}

  /// return text for edit box for i'th symbol
  const std::string & GetText(symbol i) const {return m_vCharacters[i-1].Text;}

  // return colour specified for i'th symbol, or -1 if nothing in the XML
  int GetColour(symbol i) const {
    return m_vCharacters[i-1].Colour;
  };

  const std::string &GetDefaultContext() const {return m_strDefaultContext;}

  ///A single unicode character to use as an escape sequence in training files
  ///to indicate context-switching commands; 0-length => don't use context-switching commands.
  /// Defaults to § if not specified in alphabet.
  const std::string &GetContextEscapeChar() const {return m_strCtxChar;}

  ///0 = normal alphabet, contains symbols to output
  ///1 = Japanese (defunct)
  ///2 = Mandarin: symbols are merely phonemes, and match up (via displaytext)
  /// with groups in a second alphabet, identified by strConversionTarget,
  /// which contains actual output symbols possibly including duplicates;
  /// all this handled by MandarinAlphMgr (+MandarinTrainer, PPMPYLanguageModel).
  int m_iConversionID;

  ///Single-unicode characters used in the training file to delimit the name of a group
  /// containing the next symbol, in order to disambiguate which group (=route, pronunciation)
  /// was used to produce the symbol in this case (see MandarinTrainer).
  /// Only used if m_iConversionID==2, 3 or 4. Default to "<" and ">"
  std::string m_strConversionTrainStart,m_strConversionTrainStop;

  ~CAlphInfo();

private:
  friend class CAlphIO;
  CAlphInfo();
  // Basic information
  std::string AlphID;
  bool Mutable;               // If from user we may play. If from system defaults this is immutable. User should take a copy.

  // Complete description of the alphabet:
  std::string TrainingFile;
  std::string GameModeFile;
  std::string PreferredColours;
  std::string LanguageCode; //LanguageCode in IETF Format. https://en.wikipedia.org/wiki/IETF_language_tag
  Opts::AlphabetTypes Type;
  Opts::ScreenOrientations Orientation;

  std::string m_strDefaultContext;
  std::string m_strCtxChar;

protected:
  struct character {
    character();
    std::string Display;
    std::string Text;
    int Colour;
  };
  std::vector<character> m_vCharacters;

  symbol iParagraphCharacter;       // symbol number (index into m_vCharacters +1) of paragraph char (for display and default edit-text), 0 for none.
  symbol iSpaceCharacter;   // symbol number (index into m_vCharacters +1) of space char (display and edit text), 0 for none.
  character *ControlCharacter; // display and edit text of Control character. Typically ("", "Control"). Use ("", "") if no control character.
  character *StartConvertCharacter;
  character *EndConvertCharacter;

  void copyCharacterFrom(const CAlphInfo *other, int idx);
};


/// @}

#endif /* #ifndef __AlphInfo_h__ */
