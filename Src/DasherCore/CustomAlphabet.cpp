// CustomAlphabet.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "CustomAlphabet.h"

using namespace Dasher;
using namespace std;

CCustomAlphabet::CCustomAlphabet(const CAlphIO::AlphInfo& AlphInfo)
{
	// Set miscellaneous options
	SetOrientation(AlphInfo.Orientation);
	SetLanguage(AlphInfo.Type);
	SetTrainingFile(AlphInfo.TrainingFile);
	SetPalette(AlphInfo.PreferredColours);
	
	m_AlphInfo=&AlphInfo;

	// Add all the characters.
	for (unsigned int i=0; i<AlphInfo.Groups.size(); i++) { // loop groups
		for (unsigned int j=0; j<AlphInfo.Groups[i].Characters.size(); j++) { // loop characters
			AddChar(AlphInfo.Groups[i].Characters[j].Text, AlphInfo.Groups[i].Characters[j].Display, AlphInfo.Groups[i].Characters[j].Colour, AlphInfo.Groups[i].Characters[j].Foreground);
		}
		StartNewGroup(AlphInfo.Groups[i].Colour, AlphInfo.Groups[i].Label);
	}
	
	// Set Space character if requested

	// This line makes it a bit easier for our WindowsCE compiler
	std::string empty="";

	if (AlphInfo.ParagraphCharacter.Text != empty ) {
		AddChar(AlphInfo.ParagraphCharacter.Text, AlphInfo.ParagraphCharacter.Display, AlphInfo.ParagraphCharacter.Colour, AlphInfo.ParagraphCharacter.Foreground);
		SetParagraphSymbol();
	}


	if (AlphInfo.SpaceCharacter.Text != empty ) {
		AddChar(AlphInfo.SpaceCharacter.Text, AlphInfo.SpaceCharacter.Display, AlphInfo.SpaceCharacter.Colour, AlphInfo.SpaceCharacter.Foreground);
		SetSpaceSymbol();
	}
}

void CCustomAlphabet::AddControlSymbol() {
	if (m_AlphInfo->ControlCharacter.Display != std::string("") && GetControlSymbol()==-1) {
		AddChar(m_AlphInfo->ControlCharacter.Text, m_AlphInfo->ControlCharacter.Display, m_AlphInfo->ControlCharacter.Colour, m_AlphInfo->ControlCharacter.Foreground);
		SetControlSymbol();
	}
}

void CCustomAlphabet::DelControlSymbol() {
  if (GetControlSymbol()!=-1) {
    DelChar(GetControlSymbol());
    SetControlSymbol(-1);
  }
}
