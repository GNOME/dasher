#include "CustomAlphabet.h"

using namespace Dasher;
using namespace std;


CCustomAlphabet::CCustomAlphabet(const CAlphIO::AlphInfo& AlphInfo)
{
	// Set miscellaneous options
	SetOrientation(AlphInfo.Orientation);
	SetLanguage(AlphInfo.Type);
	SetTrainingFile(AlphInfo.TrainingFile);
	
	// Add all the characters.
	for (int i=0; i<AlphInfo.Groups.size(); i++) { // loop groups
		for (int j=0; j<AlphInfo.Groups[i].Characters.size(); j++) { // loop characters
			AddChar(AlphInfo.Groups[i].Characters[j].Text, AlphInfo.Groups[i].Characters[j].Display);
		}
		StartNewGroup();
	}
	
	// Set Space character if requested
	if (AlphInfo.SpaceCharacter.Text!="") {
		AddChar(AlphInfo.SpaceCharacter.Text, AlphInfo.SpaceCharacter.Display);
		SetSpaceSymbol();
	}
}