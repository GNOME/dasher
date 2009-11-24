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

#include "../../Common/Common.h"
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

CAlphIO::CAlphIO(std::string SystemLocation, std::string UserLocation, std::vector<std::string> &Filenames)
:BlankInfo(), SystemLocation(SystemLocation), UserLocation(UserLocation), Filenames(Filenames), LoadMutable(false), CData("") {
  CreateDefault();

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

  LoadMutable = false;
  ParseFile(SystemLocation + "alphabet.xml");
  if(Filenames.size() > 0) {
    for(unsigned int i = 0; i < Filenames.size(); i++) {
      ParseFile(SystemLocation + Filenames[i]);
    }
  }
  LoadMutable = true;
  ParseFile(UserLocation + "alphabet.xml");
  if(Filenames.size() > 0) {
    for(unsigned int i = 0; i < Filenames.size(); i++) {
      ParseFile(UserLocation + Filenames[i]);
    }
  }
}

void CAlphIO::ParseFile(std::string Filename) {
  FILE *Input;
  if((Input = fopen(Filename.c_str(), "r")) == (FILE *) 0) {
    // could not open file
    return;
  }

  XML_Parser Parser = XML_ParserCreate(NULL);

  // Members passed as callbacks must be static, so don't have a "this" pointer.
  // We give them one through horrible casting so they can effect changes.
  XML_SetUserData(Parser, this);

  XML_SetElementHandler(Parser, XML_StartElement, XML_EndElement);
  XML_SetCharacterDataHandler(Parser, XML_CharacterData);

  const unsigned int BufferSize = 1024;
  char Buffer[BufferSize];
  int Done;
  do {
    size_t len = fread(Buffer, 1, sizeof(Buffer), Input);
    Done = len < sizeof(Buffer);
    if(XML_Parse(Parser, Buffer, len, Done) == XML_STATUS_ERROR) {
      break;
    }
  } while(!Done);

  XML_ParserFree(Parser);
  fclose(Input);
}

void CAlphIO::GetAlphabets(std::vector <std::string >*AlphabetList) const {
  AlphabetList->clear();

  typedef std::map < std::string, AlphInfo >::const_iterator CI;
  CI End = Alphabets.end();

  for(CI Cur = Alphabets.begin(); Cur != End; Cur++)
    AlphabetList->push_back((*Cur).second.AlphID);
}

std::string CAlphIO::GetDefault() {
  if(Alphabets.count("English with limited punctuation") != 0) {
    return "English with limited punctuation";
  }
  else {
    return "Default";
  }
}

const CAlphIO::AlphInfo & CAlphIO::GetInfo(const std::string &AlphID) {
  if(Alphabets.count(AlphID) != 0) {
    // if we have the alphabet they ask for, return it
    Alphabets[AlphID].AlphID = AlphID;        // Ensure consistency
    return Alphabets[AlphID];
  }
  else {
    // otherwise, give them default - it's better than nothing
    return Alphabets["Default"];
  }
}

void CAlphIO::SetInfo(const AlphInfo &NewInfo) {
  Alphabets[NewInfo.AlphID] = NewInfo;
  Save(NewInfo.AlphID);
}

void CAlphIO::DeleteGroups(SGroupInfo *Info) {
  SGroupInfo *next;

  while (Info) {
    next = Info->pNext;

    if (Info->pChild)
      DeleteGroups(Info->pChild);

    delete Info;
    Info = next;
  }
}

void CAlphIO::Delete(const std::string &AlphID) {
  map<std::string, AlphInfo>::iterator it = Alphabets.find(AlphID);
  if(it != Alphabets.end()) {
    DeleteGroups(it->second.m_pBaseGroup);
    Alphabets.erase(it);
    Save("");
  }
}

void CAlphIO::Save(const std::string &AlphID) {
  // TODO: We cannot reliably output XML at the moment this will have
  // to be re-implemented if we ever decide that we need to do this
  // again


  // Write an XML file containing all the alphabets that have been defined.
  // I am not going to indent the XML file as it will just bloat it, and it
  // is very simple. There are line breaks though as it is very hard to read
  // without. I'm going to ignore AlphID and save all alphabets as the
  // overhead doesn't seem to matter and it makes things much easier.
  char Number[sizeof(int)];
  FILE *Output;
  std::string Filename = UserLocation + "alphabet.xml";
  if((Output = fopen(Filename.c_str(), "w")) == (FILE *) 0) {
    // could not open file
  }

  fwrite("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", sizeof(char), 39, Output);
  fwrite("<!DOCTYPE alphabets SYSTEM \"alphabet.dtd\">\n", sizeof(char), 43, Output);
  fwrite("<?xml-stylesheet type=\"text/xsl\" href=\"alphabet.xsl\"?>\n", sizeof(char), 55, Output);
  fwrite("<alphabets>\n", sizeof(char), 12, Output);

  typedef std::map < std::string, AlphInfo >::const_iterator CI;
  CI End = Alphabets.end();
  for(CI Cur = Alphabets.begin(); Cur != End; Cur++) {
    AlphInfo Info = (*Cur).second;      // Take a copy so that special characters can be escaped

    if(Info.Mutable == false)   // this is a system alphabet, not one we write
      continue;

    fwrite("<alphabet name=\"", sizeof(char), 16, Output);
    XML_Escape(&Info.AlphID, true);
    fwrite(Info.AlphID.c_str(), sizeof(char), Info.AlphID.size(), Output);
    fwrite("\">\n", sizeof(char), 3, Output);

    fwrite("<orientation type=\"", sizeof(char), 19, Output);
    switch (Info.Orientation) {
    case Opts::RightToLeft:
      fwrite("RL", sizeof(char), 2, Output);
      break;
    case Opts::TopToBottom:
      fwrite("TB", sizeof(char), 2, Output);
      break;
    case Opts::BottomToTop:
      fwrite("BT", sizeof(char), 2, Output);
      break;
    case Opts::LeftToRight:
      // deliberate fall through
    default:
      fwrite("LR", sizeof(char), 2, Output);
      break;
    }
    fwrite("\"/>\n", sizeof(char), 4, Output);

    // TODO Encoding properly
    fwrite("<encoding type=\"", sizeof(char), 16, Output);
    fwrite(TtoS[Info.Type].c_str(), sizeof(char), TtoS[Info.Type].size(), Output);
    fwrite("\"/>\n", sizeof(char), 4, Output);

    fwrite("<palette>", sizeof(char), 9, Output);
    XML_Escape(&Info.PreferredColours, false);
    fwrite(Info.PreferredColours.c_str(), sizeof(char), Info.PreferredColours.size(), Output);
    fwrite("</palette>\n", sizeof(char), 11, Output);

    fwrite("<train>", sizeof(char), 7, Output);
    XML_Escape(&Info.TrainingFile, false);
    fwrite(Info.TrainingFile.c_str(), sizeof(char), Info.TrainingFile.size(), Output);
    fwrite("</train>\n", sizeof(char), 9, Output);

    fwrite("<gamemode>", sizeof(char), 10, Output);
    XML_Escape(&Info.GameModeFile, false);
    fwrite(Info.GameModeFile.c_str(), sizeof(char), Info.GameModeFile.size(), Output);
    fwrite("</gamemode>\n", sizeof(char), 12, Output);

    // Write out the space character
    fwrite("<space d=\"", sizeof(char), 10, Output);
    XML_Escape(&Info.SpaceCharacter.Display, true);
    fwrite(Info.SpaceCharacter.Display.c_str(), sizeof(char), Info.SpaceCharacter.Display.size(), Output);
    fwrite("\" t=\"", sizeof(char), 5, Output);
    XML_Escape(&Info.SpaceCharacter.Text, true);
    fwrite(Info.SpaceCharacter.Text.c_str(), sizeof(char), Info.SpaceCharacter.Text.size(), Output);
    fwrite("\" b=\"", sizeof(char), 5, Output);
    sprintf(Number, "%d", Info.SpaceCharacter.Colour);
    fwrite(Number, sizeof(char), strlen(Number), Output);
    fwrite("\"/>\n", sizeof(char), 4, Output);

    // Write out the paragraph character
    fwrite("<paragraph d=\"", sizeof(char), 14, Output);
    XML_Escape(&Info.ParagraphCharacter.Display, true);
    fwrite(Info.ParagraphCharacter.Display.c_str(), sizeof(char), Info.ParagraphCharacter.Display.size(), Output);
    fwrite("\" t=\"", sizeof(char), 5, Output);
    XML_Escape(&Info.ParagraphCharacter.Text, true);
    fwrite(Info.ParagraphCharacter.Text.c_str(), sizeof(char), Info.ParagraphCharacter.Text.size(), Output);
    fwrite("\" b=\"", sizeof(char), 5, Output);
    sprintf(Number, "%d", Info.ParagraphCharacter.Colour);
    fwrite(Number, sizeof(char), strlen(Number), Output);
    fwrite("\"/>\n", sizeof(char), 4, Output);

    // Write out the control character
    fwrite("<control d=\"", sizeof(char), 12, Output);
    XML_Escape(&Info.ControlCharacter.Display, true);
    fwrite(Info.ControlCharacter.Display.c_str(), sizeof(char), Info.ControlCharacter.Display.size(), Output);
    fwrite("\" t=\"", sizeof(char), 5, Output);
    XML_Escape(&Info.ControlCharacter.Text, true);
    fwrite(Info.ControlCharacter.Text.c_str(), sizeof(char), Info.ControlCharacter.Text.size(), Output);
    fwrite("\" b=\"", sizeof(char), 5, Output);
    sprintf(Number, "%d", Info.ControlCharacter.Colour);
    fwrite(Number, sizeof(char), strlen(Number), Output);
    fwrite("\"/>\n", sizeof(char), 4, Output);

    //    typedef vector < AlphInfo::group >::iterator gi;
//     gi LG = Info.Groups.end();
//     for(gi CG = Info.Groups.begin(); CG != LG; CG++) {
//       fwrite("<group name=\"", sizeof(char), 13, Output);
//       XML_Escape(&CG->Description, true);
//       fwrite(CG->Description.c_str(), sizeof(char), CG->Description.size(), Output);
//       fwrite("\" b=\"", sizeof(char), 5, Output);
//       sprintf(Number, "%d", CG->Colour);
//       fwrite(Number, sizeof(char), strlen(Number), Output);
//       fwrite("\">\n", sizeof(char), 3, Output);

//       // Iterate over CG->Characters
//       typedef vector < AlphInfo::character >::iterator ci;
//       ci LC = CG->Characters.end();
//       for(ci CC = CG->Characters.begin(); CC != LC; CC++) {
//         fwrite("<s d=\"", sizeof(char), 6, Output);
//         XML_Escape(&CC->Display, true);
//         fwrite(CC->Display.c_str(), sizeof(char), CC->Display.size(), Output);
//         fwrite("\" t=\"", sizeof(char), 5, Output);
//         XML_Escape(&CC->Text, true);
//         fwrite(CC->Text.c_str(), sizeof(char), CC->Text.size(), Output);
//         fwrite("\" b=\"", sizeof(char), 5, Output);
//         sprintf(Number, "%d", CC->Colour);
//         fwrite(Number, sizeof(char), strlen(Number), Output);
//         fwrite("\"/>\n", sizeof(char), 4, Output);
//       }

//       fwrite("</group>\n", sizeof(char), 9, Output);
//     }

    fwrite("</alphabet>\n", sizeof(char), 12, Output);
  }

  fwrite("</alphabets>\n", sizeof(char), 13, Output);

  fclose(Output);
}

void CAlphIO::CreateDefault() {
  // TODO I appreciate these strings should probably be in a resource file.
  // Not urgent though as this is not intended to be used. It's just a
  // last ditch effort in case file I/O totally fails.
  AlphInfo & Default = Alphabets["Default"];
  Default.AlphID = "Default";
  Default.Type = Opts::Western;
  Default.Mutable = false;
  Default.Orientation = Opts::LeftToRight;
  Default.ParagraphCharacter.Display = "¶";
#ifdef WIN32
  Default.ParagraphCharacter.Text = "\r\n";
#else
  Default.ParagraphCharacter.Text = "\n";
#endif
  Default.ParagraphCharacter.Colour = 9;
  Default.SpaceCharacter.Display = "_";
  Default.SpaceCharacter.Text = " ";
  Default.SpaceCharacter.Colour = 9;
  Default.ControlCharacter.Display = "Control";
  Default.ControlCharacter.Text = "";
  Default.ControlCharacter.Colour = 8;
  Default.TrainingFile = "training_english_GB.txt";
  Default.GameModeFile = "gamemode_english_GB.txt";
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
  Default.m_pBaseGroup = 0;

  Default.m_vCharacters.resize(Chars.size());
  for(unsigned int i(0); i < Chars.size(); i++) {
    Default.m_vCharacters[i].Text = Chars[i];
    Default.m_vCharacters[i].Display = Chars[i];
    Default.m_vCharacters[i].Colour = i + 10;
  }
}

void CAlphIO::XML_Escape(std::string *Text, bool Attribute) {
  // The XML "W3C Recommendation" is here: http://www.w3.org/TR/REC-xml

  std::string & Input = *Text;       // Makes syntax less fiddly below

  for(unsigned int i = 0; i < Text->size(); i++) {
    // & and < need escaping in XML. In one rare circumstance >
    // needs escaping too. I'll always do it, as I'm allowed to.
    if(Input[i] == '&') {
      Input.replace(i, 1, "&amp;");
      continue;
    }
    if(Input[i] == '<') {
      Input.replace(i, 1, "&lt;");
      continue;
    }
    if(Input[i] == '>') {
      Input.replace(i, 1, "&gt;");
      continue;
    }
    // " and ' might need escaping inside attributes, I'll do them all.
    if(Attribute == false)
      continue;

    if(Input[i] == '\'') {
      Input.replace(i, 1, "&apos;");
      continue;
    }
    if(Input[i] == '"') {
      Input.replace(i, 1, "&quot;");
      continue;
    }
  }
}

// Below here handlers for the Expat XML input library
////////////////////////////////////////////////////////////////////////////////////

void CAlphIO::XML_StartElement(void *userData, const XML_Char *name, const XML_Char **atts) {
  CAlphIO *Me = (CAlphIO *) userData;

  Me->CData = "";

  if(strcmp(name, "alphabet") == 0) {
    AlphInfo NewInfo;
    Me->InputInfo = NewInfo;
    Me->InputInfo.Mutable = Me->LoadMutable;
    Me->InputInfo.SpaceCharacter.Colour = -1;
    Me->InputInfo.ParagraphCharacter.Colour = -1;
    Me->InputInfo.ControlCharacter.Colour = -1;
    Me->InputInfo.StartConvertCharacter.Text = "";
    Me->InputInfo.EndConvertCharacter.Text = "";
    Me->InputInfo.m_iCharacters = 1; // Start at 1 as 0 is the root node symbol
    Me->InputInfo.m_pBaseGroup = 0;
    Me->InputInfo.iNumChildNodes = 1; // the "root node" symbol (with text "")
    Me->bFirstGroup = true;
    Me->iGroupIdx = 0;
    while(*atts != 0) {
      if(strcmp(*atts, "name") == 0) {
        atts++;
        Me->InputInfo.AlphID = *atts;
        atts--;
      }
      atts += 2;
    }
    Me->InputInfo.m_iConversionID = 0;
    Me->InputInfo.m_strDefaultContext = ". ";
    Me->m_vGroups.clear();
    return;
  }

  if(strcmp(name, "orientation") == 0) {
    while(*atts != 0) {
      if(!strcmp(*atts, "type")) {
        atts++;
        if(!strcmp(*atts, "RL")) {
          Me->InputInfo.Orientation = Opts::RightToLeft;
        }
        else if(!strcmp(*atts, "TB")) {
          Me->InputInfo.Orientation = Opts::TopToBottom;
        }
        else if(!strcmp(*atts, "BT")) {
          Me->InputInfo.Orientation = Opts::BottomToTop;
        }
        else
          Me->InputInfo.Orientation = Opts::LeftToRight;
        atts--;
      }
      atts += 2;
    }
    return;
  }

  if(strcmp(name, "encoding") == 0) {
    while(*atts != 0) {
      if(strcmp(*atts, "type") == 0) {
        atts++;
        Me->InputInfo.Type = Me->StoT[*atts];
        atts--;
      }
      atts += 2;
    }
    return;
  }

  if(strcmp(name, "space") == 0) {
    while(*atts != 0) {
      if(strcmp(*atts, "t") == 0) {
        atts++;
        Me->InputInfo.SpaceCharacter.Text = *atts;
        atts--;
      }
      if(strcmp(*atts, "d") == 0) {
        atts++;
        Me->InputInfo.SpaceCharacter.Display = *atts;
        atts--;
      }
      if(strcmp(*atts, "b") == 0) {
        atts++;
        Me->InputInfo.SpaceCharacter.Colour = atoi(*atts);
        atts--;
      }
      if(strcmp(*atts, "f") == 0) {
        atts++;
        Me->InputInfo.SpaceCharacter.Foreground = *atts;
        atts--;
      }
      atts += 2;
    }
    return;
  }
  if(strcmp(name, "paragraph") == 0) {
    while(*atts != 0) {
      if(strcmp(*atts, "d") == 0) {
        atts++;
        Me->InputInfo.ParagraphCharacter.Display = *atts;
#ifdef WIN32
        Me->InputInfo.ParagraphCharacter.Text = "\r\n";
#else
        Me->InputInfo.ParagraphCharacter.Text = "\n";
#endif
        atts--;
      }
      if(strcmp(*atts, "b") == 0) {
        atts++;
        Me->InputInfo.ParagraphCharacter.Colour = atoi(*atts);
        atts--;
      }
      if(strcmp(*atts, "f") == 0) {
        atts++;
        Me->InputInfo.ParagraphCharacter.Foreground = *atts;
        atts--;
      }
      atts += 2;
    }
    return;
  }
  if(strcmp(name, "control") == 0) {
    while(*atts != 0) {
      if(strcmp(*atts, "t") == 0) {
        atts++;
        Me->InputInfo.ControlCharacter.Text = *atts;
        atts--;
      }
      if(strcmp(*atts, "d") == 0) {
        atts++;
        Me->InputInfo.ControlCharacter.Display = *atts;
        atts--;
      }
      if(strcmp(*atts, "b") == 0) {
        atts++;
        Me->InputInfo.ControlCharacter.Colour = atoi(*atts);
        atts--;
      }
      if(strcmp(*atts, "f") == 0) {
        atts++;
        Me->InputInfo.ControlCharacter.Foreground = *atts;
        atts--;
      }
      atts += 2;
    }
    return;
  }

  if(strcmp(name, "group") == 0) {
    SGroupInfo *pNewGroup(new SGroupInfo);
    pNewGroup->iNumChildNodes=0;
    pNewGroup->iColour = (Me->iGroupIdx % 3) + 110; 
    ++Me->iGroupIdx;
    if (Me->m_vGroups.empty()) Me->InputInfo.iNumChildNodes++; else Me->m_vGroups.back()->iNumChildNodes++;

    if(Me->bFirstGroup) {
      pNewGroup->bVisible = false;
      Me->bFirstGroup = false;
    }
    else {
      pNewGroup->bVisible = true;
    }

    while(*atts != 0) {
      if(strcmp(*atts, "name") == 0) {
	// TODO: Fix this, or remove if names aren't needed

//         atts++;
//         Me->InputInfo.Groups.back().Description = *atts;
//         atts--;
      }
      if(strcmp(*atts, "b") == 0) {
        atts++;
	pNewGroup->iColour = atoi(*atts);
        atts--;
      }
      if(strcmp(*atts, "visible") == 0) {
	atts++;
	if(!strcmp(*atts, "yes") || !strcmp(*atts, "on"))
	  pNewGroup->bVisible = true;
	else if(!strcmp(*atts, "no") || !strcmp(*atts, "off"))
	  pNewGroup->bVisible = false;
	atts--;
      }
      if(strcmp(*atts, "label") == 0) {
        atts++;
	pNewGroup->strLabel = *atts;
        atts--;
      }
      atts += 2;
    }

    pNewGroup->iStart = Me->InputInfo.m_iCharacters;

    pNewGroup->pChild = NULL;

    if(Me->m_vGroups.size() > 0) {
      pNewGroup->pNext = Me->m_vGroups.back()->pChild;
      Me->m_vGroups.back()->pChild = pNewGroup;
    }
    else {
      pNewGroup->pNext = Me->InputInfo.m_pBaseGroup;
      Me->InputInfo.m_pBaseGroup = pNewGroup;
    }


    Me->m_vGroups.push_back(pNewGroup);

    return;
  }

  if(!strcmp(name, "conversionmode")) {
    while(*atts != 0) {
      if(strcmp(*atts, "id") == 0) {
        atts++;
        Me->InputInfo.m_iConversionID = atoi(*atts);
        atts--;
      }
      atts += 2;
    }

    return;
  }

  // Special characters for character composition
  if(strcmp(name, "convert") == 0) {
    while(*atts != 0) {
      if(strcmp(*atts, "t") == 0) {
        atts++;
        Me->InputInfo.StartConvertCharacter.Text = *atts;
        atts--;
      }
      if(strcmp(*atts, "d") == 0) {
        atts++;
        Me->InputInfo.StartConvertCharacter.Display = *atts;
        atts--;
      }
      if(strcmp(*atts, "b") == 0) {
        atts++;
        Me->InputInfo.StartConvertCharacter.Colour = atoi(*atts);
        atts--;
      }
      if(strcmp(*atts, "f") == 0) {
        atts++;
        Me->InputInfo.StartConvertCharacter.Foreground = *atts;
        atts--;
      }
      atts += 2;
    }
    return;
  }

  if(strcmp(name, "protect") == 0) {
    while(*atts != 0) {
      if(strcmp(*atts, "t") == 0) {
        atts++;
        Me->InputInfo.EndConvertCharacter.Text = *atts;
        atts--;
      }
      if(strcmp(*atts, "d") == 0) {
        atts++;
        Me->InputInfo.EndConvertCharacter.Display = *atts;
        atts--;
      }
      if(strcmp(*atts, "b") == 0) {
        atts++;
        Me->InputInfo.EndConvertCharacter.Colour = atoi(*atts);
        atts--;
      }
      if(strcmp(*atts, "f") == 0) {
        atts++;
        Me->InputInfo.EndConvertCharacter.Foreground = *atts;
        atts--;
      }
      atts += 2;
    }
    return;
  }


  if(strcmp(name, "context") == 0) {
    while(*atts != 0) {
      if(strcmp(*atts, "default") == 0) {
        atts++;
        Me->InputInfo.m_strDefaultContext = *atts;
        atts--;
      }
      atts += 2;
    }
    return;
  }

  if(strcmp(name, "s") == 0) {
    AlphInfo::character NewCharacter;

    ++Me->InputInfo.m_iCharacters;
    if (Me->m_vGroups.empty()) Me->InputInfo.iNumChildNodes++; else Me->m_vGroups.back()->iNumChildNodes++;

    NewCharacter.Colour = -1;

    Me->InputInfo.m_vCharacters.push_back(NewCharacter);
    AlphInfo::character &Ch = Me->InputInfo.m_vCharacters.back();
    
    // FIXME - need to do a more sensible job of ensuring that
    // defaults are correct (plus more generally fixing behaviour when
    // incomplete/invalid XML is supplied)
    Ch.Colour=-1;

    while(*atts != 0) {
      if(strcmp(*atts, "t") == 0) {
        atts++;
        Ch.Text = *atts;
        atts--;
      }
      if(strcmp(*atts, "d") == 0) {
        atts++;
        Ch.Display = *atts;
        atts--;
      }
      if(strcmp(*atts, "b") == 0) {
        atts++;
        Ch.Colour = atoi(*atts);
        atts--;
      }
      if(strcmp(*atts, "f") == 0) {
        atts++;
        Ch.Foreground = *atts;
        atts--;
      }
      atts += 2;
    }
    return;
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

void CAlphIO::XML_EndElement(void *userData, const XML_Char *name) {
  CAlphIO *Me = (CAlphIO *) userData;

  if(strcmp(name, "alphabet") == 0) {
    Reverse(Me->InputInfo.m_pBaseGroup);
    if (Me->InputInfo.SpaceCharacter.Text != "") Me->InputInfo.iNumChildNodes++;
    if (Me->InputInfo.ParagraphCharacter.Text != "") Me->InputInfo.iNumChildNodes++;
    if (Me->InputInfo.StartConvertCharacter.Text != "") Me->InputInfo.iNumChildNodes++;
    if (Me->InputInfo.EndConvertCharacter.Text != "") Me->InputInfo.iNumChildNodes++;
    Me->Alphabets[Me->InputInfo.AlphID] = Me->InputInfo;
    return;
  }

  if(strcmp(name, "train") == 0) {
    Me->InputInfo.TrainingFile = Me->CData;
    return;
  }

  if(strcmp(name, "gamemode") == 0) {
    Me->InputInfo.GameModeFile = Me->CData;
    return;
  }

  if(strcmp(name, "palette") == 0) {
    Me->InputInfo.PreferredColours = Me->CData;
    return;
  }

  if(!strcmp(name, "group")) {
    Me->m_vGroups.back()->iEnd = Me->InputInfo.m_iCharacters;
    //child groups were added (to linked list) in reverse order. Put them in (iStart/iEnd) order...
    Reverse(Me->m_vGroups.back()->pChild);
    Me->m_vGroups.pop_back();
    return;
  }
}

void CAlphIO::XML_CharacterData(void *userData, const XML_Char *s, int len) {
  // CAREFUL: s points to a string which is NOT null-terminated.

  CAlphIO *Me = (CAlphIO *) userData;

  Me->CData.append(s, len);
}

CAlphIO::~CAlphIO() {
  for (std::map<std::string, AlphInfo>::iterator it = Alphabets.begin(); it != Alphabets.end(); ++it)
    DeleteGroups(it->second.m_pBaseGroup);
}
