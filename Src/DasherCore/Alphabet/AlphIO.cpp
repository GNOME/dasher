// AlphIO.cpp
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

#include "AlphIO.h"

#include <iostream>
#include <cstring>

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CAlphIO::CAlphIO(CMessageDisplay *pMsgs) : AbstractXMLParser(pMsgs) {
  Alphabets["Default"]=CreateDefault();

  typedef pair < Opts::AlphabetTypes, std::string > AT;
  vector < AT > Types;
  Types.push_back(AT(Opts::MyNone, "None"));
  Types.push_back(AT(Opts::Arabic, "Arabic"));
  Types.push_back(AT(Opts::Baltic, "Baltic"));
  Types.push_back(AT(Opts::CentralEurope, "CentralEurope"));
  Types.push_back(AT(Opts::ChineseSimplified, "ChineseSimplified"));
  Types.push_back(AT(Opts::ChineseTraditional, "ChineseTraditional"));
  Types.push_back(AT(Opts::Cyrillic, "Cyrillic"));
  Types.push_back(AT(Opts::Greek, "Greek"));
  Types.push_back(AT(Opts::Hebrew, "Hebrew"));
  Types.push_back(AT(Opts::Japanese, "Japanese"));
  Types.push_back(AT(Opts::Korean, "Korean"));
  Types.push_back(AT(Opts::Thai, "Thai"));
  Types.push_back(AT(Opts::Turkish, "Turkish"));
  Types.push_back(AT(Opts::VietNam, "VietNam"));
  Types.push_back(AT(Opts::Western, "Western"));
  for(unsigned int i = 0; i < Types.size(); i++) {
    StoT[Types[i].second] = Types[i].first;
    TtoS[Types[i].first] = Types[i].second;
  }

}

void CAlphIO::GetAlphabets(std::vector <std::string >*AlphabetList) const {
  AlphabetList->clear();

  for (auto alphabet : Alphabets)
    AlphabetList->push_back(alphabet.second->AlphID);
}

std::string CAlphIO::GetDefault() {
  if(Alphabets.count("English with limited punctuation") != 0) {
    return "English with limited punctuation";
  }
  else {
    return "Default";
  }
}

const CAlphInfo *CAlphIO::GetInfo(const std::string &AlphID) const {
  auto it = Alphabets.find(AlphID);
  if (it == Alphabets.end()) //if we don't have the alphabet they ask for,
    it = Alphabets.find("Default"); //give them default - it's better than nothing
  return it->second;
}

CAlphInfo *CAlphIO::CreateDefault() {
  // TODO I appreciate these strings should probably be in a resource file.
  // Not urgent though as this is not intended to be used. It's just a
  // last ditch effort in case file I/O totally fails.
  CAlphInfo &Default(*(new CAlphInfo()));
  Default.AlphID = "Default";
  Default.Type = Opts::Western;
  Default.Mutable = false;
  Default.TrainingFile = "training_english_GB.txt";
  Default.GameModeFile = "gamemode_english_GB.txt";
  Default.LanguageCode = "en-GB";
  Default.PreferredColours = "Default";
  std::string Chars = "abcdefghijklmnopqrstuvwxyz";

//   // Obsolete
//   Default.Groups.resize(1);
//   Default.Groups[0].Description = "Lower case Latin letters";
//   Default.Groups[0].Characters.resize(Chars.size());
//   Default.Groups[0].Colour = 0;
//   Default.m_pBaseGroup = 0;
//   for(unsigned int i = 0; i < Chars.size(); i++) {
//     Default.Groups[0].Characters[i].Text = Chars[i];
//     Default.Groups[0].Characters[i].Display = Chars[i];
//     Default.Groups[0].Characters[i].Colour = i + 10;
//   }
  // ---
  Default.pChild = 0;
  Default.Orientation = Opts::LeftToRight;

  //The following creates Chars.size()+2 actual character structs in the vector,
  // all initially blank. The extra 2 are for paragraph and space.
  Default.m_vCharacters.resize(Chars.size()+2);
  //fill in structs for characters in Chars...
  for(unsigned int i(0); i < Chars.size(); i++) {
    Default.m_vCharacters[i].Text = Chars[i];
    Default.m_vCharacters[i].Display = Chars[i];
    Default.m_vCharacters[i].Colour = i + 10;
  }

  //note iSpaceCharacter/iParagraphCharacter, as all symbol numbers, are one _more_
  // than their index into m_vCharacters... (as "unknown symbol" 0 does not appear in vector)
  Default.iParagraphCharacter = Chars.size()+1;
  Default.m_vCharacters[Chars.size()].Display = "¶";
#ifdef _WIN32
  Default.m_vCharacters[Chars.size()].Text = "\r\n";
#else
  Default.m_vCharacters[Chars.size()].Text = "\n";
#endif
  Default.m_vCharacters[Chars.size()].Colour = 9;

  Default.iSpaceCharacter = Chars.size()+2;
  Default.m_vCharacters[Chars.size()+1].Display = "_";
  Default.m_vCharacters[Chars.size()+1].Text = " ";
  Default.m_vCharacters[Chars.size()+1].Colour = 9;

  Default.ControlCharacter = new CAlphInfo::character();
  Default.ControlCharacter->Display = "Control";
  Default.ControlCharacter->Text = "";
  Default.ControlCharacter->Colour = 8;

  Default.iStart=1; Default.iEnd=Default.m_vCharacters.size()+1;
  Default.iNumChildNodes = Default.m_vCharacters.size();
  Default.pNext=Default.pChild=NULL;
  
  return &Default;
}

// Below here handlers for the Expat XML input library
////////////////////////////////////////////////////////////////////////////////////

void CAlphIO::XmlStartHandler(const XML_Char *name, const XML_Char **atts) {

  CData = "";

  if (strcmp(name, "alphabets") == 0) {
    while(*atts != 0) {
      if(strcmp(*atts, "langcode") == 0) {
        LanguageCode = *(atts+1);
      }
      atts += 2;
    }
  }

  if(strcmp(name, "alphabet") == 0) {
    InputInfo = new CAlphInfo();
    InputInfo->Mutable = isUser();
    ParagraphCharacter = NULL;
    SpaceCharacter = NULL;
    iGroupIdx = 0;
    while(*atts != 0) {
      if(strcmp(*atts, "name") == 0) {
        InputInfo->AlphID = *(atts+1);
      } else if (strcmp(*atts, "escape") == 0) {
        InputInfo->m_strCtxChar = *(atts+1);
      }
      atts += 2;
    }
    m_vGroups.clear();
    return;
  }

  if(strcmp(name, "orientation") == 0) {
    while(*atts != 0) {
      if(!strcmp(*atts, "type")) {
        if(!strcmp(*(atts+1), "RL")) {
          InputInfo->Orientation = Opts::RightToLeft;
        }
        else if(!strcmp(*(atts+1), "TB")) {
          InputInfo->Orientation = Opts::TopToBottom;
        }
        else if(!strcmp(*(atts+1), "BT")) {
          InputInfo->Orientation = Opts::BottomToTop;
        }
        else
          InputInfo->Orientation = Opts::LeftToRight;
      }
      atts += 2;
    }
    return;
  }

  if(strcmp(name, "encoding") == 0) {
    while(*atts != 0) {
      if(strcmp(*atts, "type") == 0) {
        InputInfo->Type = StoT[*(atts+1)];
      }
      atts += 2;
    }
    return;
  }

  if(strcmp(name, "space") == 0) {
    if (!SpaceCharacter) SpaceCharacter = new CAlphInfo::character();
    ReadCharAtts(atts,*SpaceCharacter);
    if (SpaceCharacter->Colour==-1) SpaceCharacter->Colour = 9;
    return;
  }
  if(strcmp(name, "paragraph") == 0) {
    if (!ParagraphCharacter) ParagraphCharacter=new CAlphInfo::character();
    ReadCharAtts(atts,*ParagraphCharacter);
#ifdef _WIN32
        ParagraphCharacter->Text = "\r\n";
#else
        ParagraphCharacter->Text = "\n";
#endif
    return;
  }
  if(strcmp(name, "control") == 0) {
    if (!InputInfo->ControlCharacter) InputInfo->ControlCharacter = new CAlphInfo::character();
    ReadCharAtts(atts, *(InputInfo->ControlCharacter));
    return;
  }

  if(strcmp(name, "group") == 0) {
    SGroupInfo *pNewGroup(new SGroupInfo);
    pNewGroup->iNumChildNodes=0;
    pNewGroup->iColour = -1; //marker for "none specified"; if so, will compute later
    if (m_vGroups.empty()) InputInfo->iNumChildNodes++; else m_vGroups.back()->iNumChildNodes++;

    //by default, the first group in the alphabet is invisible
    pNewGroup->bVisible = (InputInfo->pChild!=NULL);

    while(*atts != 0) {
      if(strcmp(*atts, "name") == 0)
         pNewGroup->strName = *(atts+1);
      else if(strcmp(*atts, "b") == 0) {
        pNewGroup->iColour = atoi(*(atts+1));
      } else if(strcmp(*atts, "visible") == 0) {
        atts++;
        if(!strcmp(*atts, "yes") || !strcmp(*atts, "on"))
          pNewGroup->bVisible = true;
        else if(!strcmp(*atts, "no") || !strcmp(*atts, "off"))
          pNewGroup->bVisible = false;
        atts--;
      } else if(strcmp(*atts, "label") == 0) {
        pNewGroup->strLabel = *(atts+1);
      }
      atts += 2;
    }

    SGroupInfo *&prevSibling = (m_vGroups.empty() ? InputInfo->pChild : m_vGroups.back()->pChild);

	if (pNewGroup->iColour==-1 && pNewGroup->bVisible) {
      //no colour specified. Try to colour cycle, but make sure we choose
      // a different colour from both its parent and any previous sibling
      SGroupInfo *parent=NULL;
      for (vector<SGroupInfo *>::reverse_iterator it = m_vGroups.rbegin(); it!=m_vGroups.rend(); it++)
        if ((*it)->bVisible) {parent=*it; break;}
      for (;;) {
        pNewGroup->iColour=(iGroupIdx++ % 3) + 110;
        if (parent && parent->iColour == pNewGroup->iColour)
          continue; //same colour as parent -> try again
        if (prevSibling && prevSibling->iColour == pNewGroup->iColour)
          continue; //same colour as previous sibling -> try again
        break; //different from parent and previous sibling (if any!), so ok
      }
    }

    pNewGroup->iStart = InputInfo->m_vCharacters.size()+1;

    pNewGroup->pChild = NULL;

    pNewGroup->pNext = prevSibling;
    prevSibling = pNewGroup;

    m_vGroups.push_back(pNewGroup);

    return;
  }

  if(!strcmp(name, "conversionmode")) {
    while(*atts != 0) {
      if(strcmp(*atts, "id") == 0) {
        InputInfo->m_iConversionID = atoi(*(atts+1));
      } else if (strcmp(*atts, "start") == 0) {
        //TODO, should check this is only a single unicode character;
        // no training will occur, if not...
        InputInfo->m_strConversionTrainStart = *(atts+1);
      } else if (strcmp(*atts, "stop") == 0) //similarly
        InputInfo->m_strConversionTrainStop = *(atts+1);
      atts += 2;
    }

    return;
  }

  // Special characters for character composition
  if(strcmp(name, "convert") == 0) {
    if (!InputInfo->StartConvertCharacter) InputInfo->StartConvertCharacter = new CAlphInfo::character();
    ReadCharAtts(atts, *(InputInfo->StartConvertCharacter));
    return;
  }

  if(strcmp(name, "protect") == 0) {
    if (!InputInfo->EndConvertCharacter) InputInfo->EndConvertCharacter = new CAlphInfo::character();
    ReadCharAtts(atts, *(InputInfo->EndConvertCharacter));
    return;
  }

  if(strcmp(name, "context") == 0) {
    while(*atts != 0) {
      if(strcmp(*atts, "default") == 0) {
        InputInfo->m_strDefaultContext = *(atts+1);
      }
      atts += 2;
    }
    return;
  }

  if(strcmp(name, "s") == 0) {

    if (m_vGroups.empty()) InputInfo->iNumChildNodes++; else m_vGroups.back()->iNumChildNodes++;
    InputInfo->m_vCharacters.resize(InputInfo->m_vCharacters.size()+1);
    CAlphInfo::character &Ch(InputInfo->m_vCharacters.back());

    // FIXME - need to do a more sensible job of ensuring that
    // defaults are correct (plus more generally fixing behaviour when
    // incomplete/invalid XML is supplied)
    ReadCharAtts(atts, Ch);
    return;
  }
}

void CAlphIO::ReadCharAtts(const XML_Char **atts, CAlphInfo::character &ch) {
  while(*atts != 0) {
    if(strcmp(*atts, "t") == 0) ch.Text = *(atts+1);
    else if(strcmp(*atts, "d") == 0) ch.Display = *(atts+1);
    else if(strcmp(*atts, "b") == 0) ch.Colour = atoi(*(atts+1));
    atts += 2;
  }
}

void Reverse(SGroupInfo *&pList) {
  SGroupInfo *pFirst = pList;
  SGroupInfo *pPrev = NULL;
  while (pFirst) {
    SGroupInfo *pNext = pFirst->pNext;
    pFirst->pNext = pPrev;
    pPrev = pFirst;
    pFirst = pNext;
  }
  pList=pPrev;
}

void CAlphIO::XmlEndHandler(const XML_Char *name) {

  if (strcmp(name, "alphabets") == 0) {
    LanguageCode = "";
  }

  if(strcmp(name, "alphabet") == 0) {
    Reverse(InputInfo->pChild);

    if (ParagraphCharacter) {
      InputInfo->iParagraphCharacter = InputInfo->m_vCharacters.size()+1;
      InputInfo->m_vCharacters.push_back(*ParagraphCharacter);
      InputInfo->iNumChildNodes++;
      delete ParagraphCharacter;
    }
    if (SpaceCharacter) {
      InputInfo->iSpaceCharacter = InputInfo->m_vCharacters.size()+1;
      InputInfo->m_vCharacters.push_back(*SpaceCharacter);
      InputInfo->iNumChildNodes++;
      delete SpaceCharacter;
    }

    InputInfo->LanguageCode = LanguageCode;

    InputInfo->iEnd = InputInfo->m_vCharacters.size()+1;

    //if (InputInfo->StartConvertCharacter.Text != "") InputInfo->iNumChildNodes++;
    //if (InputInfo->EndConvertCharacter.Text != "") InputInfo->iNumChildNodes++;
    Alphabets[InputInfo->AlphID] = InputInfo;
    return;
  }

  if(strcmp(name, "train") == 0) {
    InputInfo->TrainingFile = CData;
    return;
  }
  
  if(strcmp(name, "gamemode") == 0) {
    InputInfo->GameModeFile = CData;
    return;
  }

  if(strcmp(name, "palette") == 0) {
    InputInfo->PreferredColours = CData;
    return;
  }

  if(!strcmp(name, "group")) {
    SGroupInfo *finished = m_vGroups.back();
    m_vGroups.pop_back();
    finished->iEnd = InputInfo->m_vCharacters.size()+1;
    if (finished->iEnd == finished->iStart) {
      //empty group. Delete it now, and elide from sibling chain
      SGroupInfo *&ptr=(m_vGroups.empty() ? InputInfo : m_vGroups.back())->pChild;
      DASHER_ASSERT(ptr == finished);
      ptr = finished->pNext;
      delete finished;
    } else {
      //child groups were added (to linked list) in reverse order. Put them in (iStart/iEnd) order...
      Reverse(finished->pChild);
    }
    return;
  }
}

void CAlphIO::XmlCData(const XML_Char *s, int len) {
  // CAREFUL: s points to a string which is NOT null-terminated.
  CData.append(s, len);
}

CAlphIO::~CAlphIO() {
  for (auto it : Alphabets) {
    delete it.second;
  }
}
