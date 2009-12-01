// ColourIO.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __ColourIO_h__
#define __ColourIO_h__

#include "DasherTypes.h"

#include <expat.h>
#include <string>
#include <map>
#include <vector>
#include <utility>              // for std::pair
#include <stdio.h>              // for C style file IO

namespace Dasher {
  class CColourIO;
}


/// \defgroup Colours Colour scheme information
/// @{
class Dasher::CColourIO {
public:
  // This structure completely describes the characters used in alphabet
  struct ColourInfo {
    // Basic information
    std::string ColourID;
    bool Mutable;               // If from user we may play. If from system defaults this is immutable. User should take a copy.

    // TODO: It would make a lot more sense if this was a vector of triples rather than three vectors
    // Complete description of the alphabet:
    std::vector < int >Reds;
    std::vector < int >Greens;
    std::vector < int >Blues;
  };

  CColourIO(std::string SystemLocation, std::string UserLocation, std::vector < std::string > &Filenames);
  void GetColours(std::vector < std::string > *ColourList) const;
  const ColourInfo & GetInfo(const std::string & ColourID);
  void SetInfo(const ColourInfo & NewInfo);
  void Delete(const std::string & ColourID);
private:
  ColourInfo BlankInfo;
  std::string SystemLocation;
  std::string UserLocation;
  std::map < std::string, ColourInfo > Colours; // map short names (file names) to descriptions
  std::vector < std::string > Filenames;

  void Save(const std::string & ColourID);
  void CreateDefault();         // Give the user a default colour scheme rather than nothing if anything goes horribly wrong.

  // XML handling:
  /////////////////////////

  bool LoadMutable;
  void ParseFile(std::string Filename);

  // & to &amp;  < to &lt; and > to &gt;  and if (Attribute) ' to &apos; and " to &quot;
  void XML_Escape(std::string * Text, bool Attribute);

  // Data gathered
  std::string CData;            // Text gathered from when an elemnt starts to when it ends
  ColourInfo InputInfo;

  // Callback functions. These involve the normal dodgy casting to a pointer
  // to an instance to get a C++ class to work with a plain C library.
  static void XML_StartElement(void *userData, const XML_Char * name, const XML_Char ** atts);
  static void XML_EndElement(void *userData, const XML_Char * name);
  static void XML_CharacterData(void *userData, const XML_Char * s, int len);
};
/// @}

#endif /* #ifndef __ColourIO_h__ */
