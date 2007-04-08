// AlphIO.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __AlphIO_h__
#define __AlphIO_h__

#include "../DasherTypes.h"
//#include "Alphabet.h"
#include "GroupInfo.h"

#include <string>
#include <map>
#include <vector>
#include <utility>              // for std::pair
#include <stdio.h>              // for C style file IO

/* namespace Dasher { */
/*   class CAlphabet { */
/*   public: */
/*     struct SGroupInfo;  */
/*   }; */
/* } */

/// \cond expat

namespace expat {
#include "../../Common/Expat/lib/expat.h"
}

/// \endcond

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
    std::vector<SGroupInfo *> m_vGroups;
    SGroupInfo *m_pBaseGroup;

    std::vector<character> m_vCharacters;

    character ParagraphCharacter;       // display and edit text of paragraph character. Use ("", "") if no paragraph character.
    character SpaceCharacter;   // display and edit text of Space character. Typically (" ", "_"). Use ("", "") if no space character.
    character ControlCharacter; // display and edit text of Control character. Typically ("", "Control"). Use ("", "") if no control character.
    character StartConvertCharacter;
    character EndConvertCharacter;
    int m_iConversionID;
    std::string m_strDefaultContext;
  };

  CAlphIO(std::string SystemLocation, std::string UserLocation, std::vector < std::string > Filenames);
  void GetAlphabets(std::vector < std::string > *AlphabetList) const;
  std::string GetDefault();
  const AlphInfo & GetInfo(const std::string & AlphID);
  void SetInfo(const AlphInfo & NewInfo);
  void Delete(const std::string & AlphID);
private:
  AlphInfo BlankInfo;
  std::string SystemLocation;
  std::string UserLocation;
  std::map < std::string, AlphInfo > Alphabets; // map short names (file names) to descriptions
  std::vector < std::string > Filenames;

  void Save(const std::string & AlphID);
  void CreateDefault();         // Give the user an English alphabet rather than nothing if anything goes horribly wrong.

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
  static void XML_StartElement(void *userData, const expat::XML_Char * name, const expat::XML_Char ** atts);
  static void XML_EndElement(void *userData, const expat::XML_Char * name);
  static void XML_CharacterData(void *userData, const expat::XML_Char * s, int len);
};

/// @}

#endif /* #ifndef __AlphIO_h__ */
