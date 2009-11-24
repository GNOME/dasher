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

#include "../DasherTypes.h"
//#include "Alphabet.h"
#include "GroupInfo.h"

#include <expat.h>
#include <string>
#include <map>
#include <vector>
#include <utility>              // for std::pair
#include <stdio.h>              // for C style file IO

namespace Dasher {
  class CAlphIO;
} 


/// \ingroup Alphabet
/// @{

class Dasher::CAlphIO {
public:
  // This structure completely describes the characters used in alphabet

  struct AlphInfo {
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

    struct character {
      std::string Display;
      std::string Text;
      int Colour;
      std::string Foreground;
    };

/*     // Obsolete groups stuff */
/*     struct group { */
/*       std::string Description; */
/*       std::vector < character > Characters; */
/*       int Colour; */
/*       std::string Label; */
/*     }; */
/*     std::vector < group > Groups; */
/*     // --- */

    int m_iCharacters;
    SGroupInfo *m_pBaseGroup;
    int iNumChildNodes;

    std::vector<character> m_vCharacters;

    character ParagraphCharacter;       // display and edit text of paragraph character. Use ("", "") if no paragraph character.
    character SpaceCharacter;   // display and edit text of Space character. Typically (" ", "_"). Use ("", "") if no space character.
    character ControlCharacter; // display and edit text of Control character. Typically ("", "Control"). Use ("", "") if no control character.
    character StartConvertCharacter;
    character EndConvertCharacter;

    int m_iConversionID;
    std::string m_strDefaultContext;
  };

  CAlphIO(std::string SystemLocation, std::string UserLocation, std::vector < std::string > &Filenames);
  ~CAlphIO();
  void GetAlphabets(std::vector < std::string > *AlphabetList) const;
  std::string GetDefault();
  const AlphInfo & GetInfo(const std::string & AlphID);
  void SetInfo(const AlphInfo & NewInfo);
  void Delete(const std::string & AlphID);
private:
  std::vector<SGroupInfo *> m_vGroups;
  AlphInfo BlankInfo;
  std::string SystemLocation;
  std::string UserLocation;
  std::map < std::string, AlphInfo > Alphabets; // map short names (file names) to descriptions
  std::vector < std::string > Filenames;

  void Save(const std::string & AlphID);
  void CreateDefault();         // Give the user an English alphabet rather than nothing if anything goes horribly wrong.

  void DeleteGroups(SGroupInfo *Info);
  void DeleteAlphabet(AlphInfo *Alphabet);

  // XML handling:
  /////////////////////////

  bool LoadMutable;
  void ParseFile(std::string Filename);

  // Alphabet types:
  std::map < std::string, Opts::AlphabetTypes > StoT;
  std::map < Opts::AlphabetTypes, std::string > TtoS;

  // & to &amp;  < to &lt; and > to &gt;  and if (Attribute) ' to &apos; and " to &quot;
  void XML_Escape(std::string * Text, bool Attribute);

  // Data gathered
  std::string CData;            // Text gathered from when an elemnt starts to when it ends
  AlphInfo InputInfo;
  bool bFirstGroup;
  int iGroupIdx;

  // Callback functions. These involve the normal dodgy casting to a pointer
  // to an instance to get a C++ class to work with a plain C library.
  static void XML_StartElement(void *userData, const XML_Char * name, const XML_Char ** atts);
  static void XML_EndElement(void *userData, const XML_Char * name);
  static void XML_CharacterData(void *userData, const XML_Char * s, int len);
};

/// @}

#endif /* #ifndef __AlphIO_h__ */
