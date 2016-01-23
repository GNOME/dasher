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

#include "../AbstractXMLParser.h"

#include "../DasherTypes.h"
#include "AlphInfo.h"

#include <map>
#include <vector>
#include <utility>              // for std::pair

namespace Dasher {
  class CAlphIO;
}

/// \ingroup Alphabet
/// @{

/// This class is used to read in alphabet definitions from all files
/// alphabet.*.xml at startup (realization) time; it creates one CAlphInfo
/// object per alphabet at this time, and stores them in a map from AlphID
/// string until shutdown/destruction. (CAlphIO is a friend of CAlphInfo,
/// so can create/manipulate instances.)
class Dasher::CAlphIO : public AbstractXMLParser {
public:

  ///Create a new AlphIO. Initially, it will have only a 'default' alphabet
  /// definition (English); further alphabets may be loaded in by calling the
  /// Parse... methods inherited from Abstract[XML]Parser
  CAlphIO(CMessageDisplay *pMsgs);
  
  virtual ~CAlphIO();
  void GetAlphabets(std::vector < std::string > *AlphabetList) const;
  std::string GetDefault();
  const CAlphInfo *GetInfo(const std::string & AlphID) const;
private:
  CAlphInfo::character *SpaceCharacter, *ParagraphCharacter;
  std::vector<SGroupInfo *> m_vGroups;
  std::map < std::string, const CAlphInfo* > Alphabets; // map AlphabetID to AlphabetInfo. 
  CAlphInfo *CreateDefault();         // Give the user an English alphabet rather than nothing if anything goes horribly wrong.

  // XML handling:
  /////////////////////////

  void ReadCharAtts(const XML_Char **atts, CAlphInfo::character &ch);
  // Alphabet types:
  std::map < std::string, Opts::AlphabetTypes > StoT;
  std::map < Opts::AlphabetTypes, std::string > TtoS;

  // Data gathered
  std::string CData;            // Text gathered from when an elemnt starts to when it ends
  CAlphInfo *InputInfo;
  int iGroupIdx;
  std::string LanguageCode;

  void XmlStartHandler(const XML_Char * name, const XML_Char ** atts);
  void XmlEndHandler(const XML_Char * name);
  void XmlCData(const XML_Char * s, int len);
};
/// @}

#endif /* #ifndef __AlphIO_h__ */
