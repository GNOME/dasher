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

#include "AbstractXMLParser.h"

#include <map>
#include <vector>
#include <utility>              // for std::pair

namespace Dasher {
  class CColourIO;
}


/// \defgroup Colours Colour scheme information
/// @{
class Dasher::CColourIO : private AbstractXMLParser {
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

  CColourIO(CMessageDisplay *pMsgs, const std::string &SystemLocation, const std::string &UserLocation, const std::vector < std::string > &Filenames);
  void GetColours(std::vector < std::string > *ColourList) const;
  const ColourInfo & GetInfo(const std::string & ColourID);
private:
  ColourInfo BlankInfo;
  std::map < std::string, ColourInfo > Colours; // map short names (file names) to descriptions

  void CreateDefault();         // Give the user a default colour scheme rather than nothing if anything goes horribly wrong.

  // XML handling:
  /////////////////////////

  bool LoadMutable;

  // Data gathered
  std::string CData;            // Text gathered from when an elemnt starts to when it ends
  ColourInfo InputInfo;

  void XmlStartHandler(const XML_Char * name, const XML_Char ** atts);
  void XmlEndHandler(const XML_Char * name);
  void XmlCData(const XML_Char * s, int len);
};
/// @}

#endif /* #ifndef __ColourIO_h__ */
