// Alphabet.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "Alphabet.h"
#include "AlphabetMap.h"

//#include <iostream>
//WinCE doesn't have iostream!

using namespace Dasher;
using namespace std;


CAlphabet::CAlphabet() : m_Groups(0), m_DefaultEncoding(Opts::Western), m_Orientation(Opts::LeftToRight)
{
	m_Characters.push_back("");
	m_Display.push_back("");
	m_Colours.push_back("");
	m_Foreground.push_back("");
	m_Group.push_back(0);
}


void CAlphabet::GetSymbols(vector<symbol>* Symbols, string* Input, bool IsMore)
{
	string Tmp;
	symbol CurSymbol=0, TmpSymbol=0;
	bool KeyIsPrefix;
	int z= Input->size();
	int extras;
	unsigned int bit;

	for (unsigned int i=0; i<Input->size(); i++) {

		Tmp = (*Input)[i];

		/* The string we've been given is in UTF-8. The symbols are
		   also in UTF-8, so we need to pass the entire UTF-8 character
		   which may be several bytes long. RFC 2279 describes this
		   encoding */

		if ((*Input)[i] & 0x80) { // Character is more than 1 byte long
		  extras = 1;
		  for (bit = 0x20; ((*Input)[i] & bit) != 0; bit >>= 1)
		    extras++;
		  if (extras > 5) {
		  } // Malformed character
		  while (extras-->0) {
		    Tmp += (*Input)[++i];
		  }
		}

		CurSymbol = TextMap.Get(Tmp, &KeyIsPrefix);

		if (KeyIsPrefix) {
			CurSymbol = 0;
			for (; i<Input->size(); i++) {

				Tmp += (*Input)[i];

				TmpSymbol = TextMap.Get(Tmp, &KeyIsPrefix);
				if (TmpSymbol>0) {
					CurSymbol = TmpSymbol;
				}
				if (!KeyIsPrefix) {
					if (CurSymbol!=0) {
						Symbols->push_back(CurSymbol);
					} else {
						i -= Tmp.size()-1;
						//Tmp.erase(Tmp.begin(), Tmp.end());
						Tmp = "";
					}
					break;
				}
			}
		} else {
			if (CurSymbol!=0)
				Symbols->push_back(CurSymbol);
		}
	}

	if (IsMore)
		if (KeyIsPrefix)
			*Input = Tmp;
		else
			*Input = "";
	else
		if (KeyIsPrefix)
			Symbols->push_back(CurSymbol);
}


// add single char to the character set
void CAlphabet::AddChar(const string NewCharacter, const string Display, const string Colour, const string Foreground)
{
	m_Characters.push_back(NewCharacter);
	m_Display.push_back(Display);
	m_Colours.push_back(Colour);
	m_Foreground.push_back(Foreground);
	m_Group.push_back(m_Groups);

	symbol ThisSymbol = m_Characters.size()-1;
	TextMap.Add(NewCharacter, ThisSymbol);
}


void CAlphabet::StartNewGroup()
{
	m_Groups++;
}


// diagnostic dump of character set
void CAlphabet::dump() const {
// TODO
/*
	dchar deb[256];
	unsigned int i;
	for (i=1;i<m_vtCharacters.size();i++) {
		//wsprintf(deb,TEXT("%d %c %d\n"),i,m_vtCharacters[i],m_viGroup[i]); // Windows specific
		Usprintf(deb,TEXT("%d %c %d\n"),i,m_vtCharacters[i],m_viGroup[i]);
		DebugOutput(deb);
	}
*/
}

int CAlphabet::GetTextColour(symbol Symbol)
{
  std::string TextColour=m_Foreground[Symbol];
  if (TextColour != "") {
    return atoi(TextColour.c_str());
  } else {
    return 0;
  }
}
