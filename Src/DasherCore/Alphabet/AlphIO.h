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

#ifndef __AlphIO_h__
#define __AlphIO_h__

#include "../../Common/Common.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../DasherTypes.h"
#include "AlphabetMap.h"
#include "GroupInfo.h"

#include <expat.h>
#include <string>
#include <map>
#include <vector>
#include <utility>              // for std::pair
#include <stdio.h>              // for C style file IO

namespace Dasher {
  class CAlphInfo;
} 


/// \ingroup Alphabet
/// @{

/// This structure completely describes the characters used in alphabet
class Dasher::CAlphInfo {
private:
  struct character {
    character();
    std::string Display;
    std::string Text;
    int Colour;
    std::string Foreground;
  };  
public:
  /// Return number of text symbols - inc space and para, but no control/conversion start/end
  /// Note symbol numbers are 1-indexed; 0 is reserved (for the root symbol, or for
  /// element 0 of the probability array to contain a 0)
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
  class AlphIO {
  public:

    AlphIO(std::string SystemLocation, std::string UserLocation, std::vector < std::string > &Filenames);
    ~AlphIO();
    void GetAlphabets(std::vector < std::string > *AlphabetList) const;
    std::string GetDefault();
    const CAlphInfo *GetInfo(const std::string & AlphID);
    void SetInfo(const CAlphInfo *NewInfo);
    void Delete(const std::string & AlphID);
  private:
    character *SpaceCharacter, *ParagraphCharacter;
    std::vector<SGroupInfo *> m_vGroups;
    std::string SystemLocation;
    std::string UserLocation;
    std::map < std::string, const CAlphInfo* > Alphabets; // map short names (file names) to descriptions. We own all the values but it's easier this way...
    std::vector < std::string > Filenames;

    void Save(const std::string & AlphID);
    CAlphInfo *CreateDefault();         // Give the user an English alphabet rather than nothing if anything goes horribly wrong.

    void DeleteAlphabet(CAlphInfo *Alphabet);

    // XML handling:
    /////////////////////////

    bool LoadMutable;
    void ParseFile(std::string Filename);
    void ReadCharAtts(const XML_Char **atts, character &ch);
    // Alphabet types:
    std::map < std::string, Opts::AlphabetTypes > StoT;
    std::map < Opts::AlphabetTypes, std::string > TtoS;

    // & to &amp;  < to &lt; and > to &gt;  and if (Attribute) ' to &apos; and " to &quot;
    void XML_Escape(std::string * Text, bool Attribute);

    // Data gathered
    std::string CData;            // Text gathered from when an elemnt starts to when it ends
    CAlphInfo *InputInfo;
    bool bFirstGroup;
    int iGroupIdx;

    // Callback functions. These involve the normal dodgy casting to a pointer
    // to an instance to get a C++ class to work with a plain C library.
    static void XML_StartElement(void *userData, const XML_Char * name, const XML_Char ** atts);
    static void XML_EndElement(void *userData, const XML_Char * name);
    static void XML_CharacterData(void *userData, const XML_Char * s, int len);
  };
  
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
  
  int iParagraphCharacter;       // index into m_vCharacters of paragraph char (display and edit text), -1 for none.
  int iSpaceCharacter;   // index into m_vCharacters of space char (display and edit text), -1 for none.
  character *ControlCharacter; // display and edit text of Control character. Typically ("", "Control"). Use ("", "") if no control character.
  character *StartConvertCharacter;
  character *EndConvertCharacter;
  
  std::string m_strDefaultContext;
};


/// @}

#endif /* #ifndef __AlphIO_h__ */
