// AlphIO.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////


#include "AlphIO.h"

using namespace Dasher;
using namespace std;
using namespace expat;


CAlphIO::CAlphIO(string SystemLocation, string UserLocation, vector<string> Filenames)
	:  BlankInfo(), SystemLocation(SystemLocation), UserLocation(UserLocation), Filenames(Filenames), LoadMutable(false), CData("")
{
	CreateDefault();
	
	typedef pair<Opts::AlphabetTypes, string> AT;
	vector<AT> Types;
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
	for (unsigned int i=0; i<Types.size(); i++) {
		StoT[Types[i].second] = Types[i].first;
		TtoS[Types[i].first] = Types[i].second;
	}
	
	LoadMutable = false;
	ParseFile(SystemLocation + "alphabet.xml");
	if (Filenames.size()>0) {
	  for (unsigned int i=0; i<Filenames.size(); i++) {
	    ParseFile(SystemLocation + Filenames[i]);
	  }
	}
	LoadMutable = true;
	ParseFile(UserLocation + "alphabet.xml");
	if (Filenames.size()>0) {
	  for (unsigned int i=0; i<Filenames.size(); i++) {
	    ParseFile(UserLocation + Filenames[i]);
	  }
	}
}


void CAlphIO::ParseFile(std::string Filename)
{
	FILE* Input;
	if ((Input = fopen (Filename.c_str(), "r")) == (FILE*)0) {
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
		if (XML_Parse(Parser, Buffer, len, Done) == XML_STATUS_ERROR) {
			break;
		}
	} while (!Done);
	
	XML_ParserFree(Parser);
	fclose(Input);
}


void CAlphIO::GetAlphabets(std::vector< std::string > * AlphabetList) const
{
	AlphabetList->clear();
	
	typedef std::map<std::string, AlphInfo>::const_iterator CI;
	CI End = Alphabets.end();
	
	for (CI Cur=Alphabets.begin(); Cur!=End; Cur++)
		AlphabetList->push_back( (*Cur).second.AlphID);
}


const CAlphIO::AlphInfo& CAlphIO::GetInfo(const std::string& AlphID)
{
  if (AlphID=="") {
    // Eww. If no alphabet is configured, default to this one...
    if (Alphabets.count("English alphabet - limited punctuation")!=0) {
      return Alphabets["English alphabet - limited punctuation"];
    } else {
      // unless it doesn't exist, in which case return default
      return Alphabets["Default"];
    }
  }  else {
    if (Alphabets.count(AlphID)!=0) {
      // if we have the alphabet they ask for, return it
      Alphabets[AlphID].AlphID = AlphID; // Ensure consistency
      return Alphabets[AlphID];
    } else {
      // otherwise, give them default - it's better than nothing
      return Alphabets["Default"];
    }
  }
}


void CAlphIO::SetInfo(const AlphInfo& NewInfo)
{
	Alphabets[NewInfo.AlphID] = NewInfo;
	Save(NewInfo.AlphID);
}


void CAlphIO::Delete(const std::string& AlphID)
{
	if (Alphabets.find(AlphID)!=Alphabets.end()) {
		Alphabets.erase(AlphID);
		Save("");
	}
}


void CAlphIO::Save(const std::string& AlphID)
{
	// Write an XML file containing all the alphabets that have been defined.
	// I am not going to indent the XML file as it will just bloat it, and it
	// is very simple. There are line breaks though as it is very hard to read
	// without. I'm going to ignore AlphID and save all alphabets as the
	// overhead doesn't seem to matter and it makes things much easier.
	char Number[sizeof(int)];
	FILE* Output;
	string Filename = UserLocation + "alphabet.xml";
	if ((Output = fopen (Filename.c_str(), "w")) == (FILE*)0) {
		// could not open file
	}
	
	fwrite("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", sizeof(char), 39, Output);
	fwrite("<!DOCTYPE alphabets SYSTEM \"alphabet.dtd\">\n", sizeof(char), 43, Output);
	fwrite("<?xml-stylesheet type=\"text/xsl\" href=\"alphabet.xsl\"?>\n", sizeof(char), 55, Output);
	fwrite("<alphabets>\n", sizeof(char), 12, Output);
	
	typedef std::map<std::string, AlphInfo>::const_iterator CI;
	CI End = Alphabets.end();
	for (CI Cur=Alphabets.begin(); Cur!=End; Cur++) {
		AlphInfo Info = (*Cur).second; // Take a copy so that special characters can be escaped
		
		if (Info.Mutable==false) // this is a system alphabet, not one we write
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

		// Write out the space character
		fwrite("<space d=\"", sizeof(char), 10, Output);
		XML_Escape(&Info.SpaceCharacter.Display, true);
		fwrite(Info.SpaceCharacter.Display.c_str(), sizeof(char), Info.SpaceCharacter.Display.size(), Output);
		fwrite("\" t=\"", sizeof(char), 5, Output);
		XML_Escape(&Info.SpaceCharacter.Text, true);
		fwrite(Info.SpaceCharacter.Text.c_str(), sizeof(char), Info.SpaceCharacter.Text.size(), Output);
		fwrite("\" b=\"", sizeof(char), 5, Output);
		sprintf(Number,"%d",Info.SpaceCharacter.Colour);
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
		sprintf(Number,"%d",Info.ParagraphCharacter.Colour);
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
		sprintf(Number,"%d",Info.ControlCharacter.Colour);
		fwrite(Number, sizeof(char), strlen(Number), Output);
		fwrite("\"/>\n", sizeof(char), 4, Output);
		
		typedef vector<AlphInfo::group>::iterator gi;
		gi LG = Info.Groups.end();
		for (gi CG = Info.Groups.begin(); CG!=LG; CG++) {
			fwrite("<group name=\"", sizeof(char), 13, Output);
			XML_Escape(&CG->Description, true);
			fwrite(CG->Description.c_str(), sizeof(char), CG->Description.size(), Output);
			fwrite("\" b=\"", sizeof(char), 5, Output);
			sprintf(Number,"%d",CG->Colour);
			fwrite(Number, sizeof(char), strlen(Number), Output);
			fwrite("\">\n", sizeof(char), 3, Output);
			
			// Iterate over CG->Characters
			typedef vector<AlphInfo::character>::iterator ci;
			ci LC = CG->Characters.end();
			for (ci CC=CG->Characters.begin(); CC!=LC; CC++) {
				fwrite("<s d=\"", sizeof(char), 6, Output);
				XML_Escape(&CC->Display, true);
				fwrite(CC->Display.c_str(), sizeof(char), CC->Display.size(), Output);
				fwrite("\" t=\"", sizeof(char), 5, Output);
				XML_Escape(&CC->Text, true);
				fwrite(CC->Text.c_str(), sizeof(char), CC->Text.size(), Output);
				fwrite("\" b=\"", sizeof(char), 5, Output);
				sprintf(Number,"%d",CC->Colour);
				fwrite(Number, sizeof(char), strlen(Number), Output);
				fwrite("\"/>\n", sizeof(char), 4, Output);
			}
			
			fwrite("</group>\n", sizeof(char), 9, Output);
		}
		
		fwrite("</alphabet>\n", sizeof(char), 12, Output);
	}
	
	fwrite("</alphabets>\n", sizeof(char), 13, Output);
	
	fclose(Output);
}


void CAlphIO::CreateDefault()
{
	// TODO I appreciate these strings should probably be in a resource file.
	// Not urgent though as this is not intended to be used. It's just a
	// last ditch effort in case file I/O totally fails.
	AlphInfo& Default = Alphabets["Default"];
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
	Default.PreferredColours = "Default";
	string Chars = "abcdefghijklmnopqrstuvwxyz";
	Default.Groups.resize(1);
	Default.Groups[0].Description = "Lower case Latin letters";
	Default.Groups[0].Characters.resize(Chars.size());
	Default.Groups[0].Colour = 0;
	for (unsigned int i=0; i<Chars.size(); i++) {
		Default.Groups[0].Characters[i].Text = Chars[i];
		Default.Groups[0].Characters[i].Display = Chars[i];
		Default.Groups[0].Characters[i].Colour = i+10;
	}
}


void CAlphIO::XML_Escape(std::string* Text, bool Attribute)
{
	// The XML "W3C Recommendation" is here: http://www.w3.org/TR/REC-xml
	
	string& Input = *Text; // Makes syntax less fiddly below
	
	for (unsigned int i=0; i<Text->size(); i++) {
		// & and < need escaping in XML. In one rare circumstance >
		// needs escaping too. I'll always do it, as I'm allowed to.
		if (Input[i]=='&') {
			Input.replace(i, 1, "&amp;");
			continue;
		}
		if (Input[i]=='<') {
			Input.replace(i, 1, "&lt;");
			continue;
		}
		if (Input[i]=='>') {
			Input.replace(i, 1, "&gt;");
			continue;
		}
		// " and ' might need escaping inside attributes, I'll do them all.
		if (Attribute==false)
			continue;
		
		if (Input[i]=='\'') {
			Input.replace(i, 1, "&apos;");
			continue;
		}
		if (Input[i]=='"') {
			Input.replace(i, 1, "&quot;");
			continue;
		}
	}
}


// Below here handlers for the Expat XML input library
////////////////////////////////////////////////////////////////////////////////////


void CAlphIO::XML_StartElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	CAlphIO* Me = (CAlphIO*) userData;
	
	Me->CData = "";
	
	if (strcmp(name, "alphabet")==0) {
		AlphInfo NewInfo;
		Me->InputInfo = NewInfo;
		Me->InputInfo.Mutable = Me->LoadMutable;
		Me->InputInfo.SpaceCharacter.Colour = -1;
		Me->InputInfo.ParagraphCharacter.Colour = -1;
		Me->InputInfo.ControlCharacter.Colour = -1;
		while (*atts!=0) {
			if (strcmp(*atts, "name")==0) {
				atts++;
				Me->InputInfo.AlphID = *atts;
				atts--;
			}
			atts += 2;
		}
		return;
	}
	
	if (strcmp(name, "orientation")==0) {
		while (*atts!=0) {
			if (strcmp(*atts, "type")) {
				atts++;
				if (strcmp(*atts, "RL")) {
					Me->InputInfo.Orientation = Opts::LeftToRight;
				} else if (strcmp(*atts, "TB")) {
					Me->InputInfo.Orientation = Opts::TopToBottom;
				} else if (strcmp(*atts, "BT")) {
					Me->InputInfo.Orientation = Opts::BottomToTop;
				} else
					Me->InputInfo.Orientation = Opts::LeftToRight;
				atts--;
			}
			atts += 2;
		}
		return;
	}
	
	if (strcmp(name, "encoding")==0) {
		while (*atts!=0) {
			if (strcmp(*atts, "type")==0) {
				atts++;
				Me->InputInfo.Type = Me->StoT[*atts];
				atts--;
			}
			atts += 2;
		}
		return;
	}
	
	if (strcmp(name, "space")==0) {
		while (*atts!=0) {
			if (strcmp(*atts, "t")==0) {
				atts++;
				Me->InputInfo.SpaceCharacter.Text = *atts;
				atts--;
			}
			if (strcmp(*atts, "d")==0) {
				atts++;
				Me->InputInfo.SpaceCharacter.Display = *atts;
				atts--;
			}
			if (strcmp(*atts, "b")==0) {
			  atts++;
			  Me->InputInfo.SpaceCharacter.Colour = atoi(*atts);
			  atts--;
			}
			if (strcmp(*atts, "f")==0) {
			  atts++;
			  Me->InputInfo.SpaceCharacter.Foreground = *atts;
			  atts--;
			}
			atts += 2;
		}
		return;
	}
	if (strcmp(name, "paragraph")==0) {
		while (*atts!=0) {
			if (strcmp(*atts, "d")==0) {
			  atts++;
			  Me->InputInfo.ParagraphCharacter.Display = *atts;
#ifdef WIN32
			  Me->InputInfo.ParagraphCharacter.Text = "\r\n";
#else
			  Me->InputInfo.ParagraphCharacter.Text = "\n";
#endif	
			  atts--;
			}
			if (strcmp(*atts, "b")==0) {
			  atts++;
			  Me->InputInfo.ParagraphCharacter.Colour = atoi(*atts);
			  atts--;
			}
			if (strcmp(*atts, "f")==0) {
			  atts++;
			  Me->InputInfo.ParagraphCharacter.Foreground = *atts;
			  atts--;
			}
			atts += 2;
		}
		return;
	}
	if (strcmp(name, "control")==0) {
		while (*atts!=0) {
			if (strcmp(*atts, "t")==0) {
				atts++;
				Me->InputInfo.ControlCharacter.Text = *atts;
				atts--;
			}
			if (strcmp(*atts, "d")==0) {
				atts++;
				Me->InputInfo.ControlCharacter.Display = *atts;
				atts--;
			}
			if (strcmp(*atts, "b")==0) {
				atts++;
				Me->InputInfo.ControlCharacter.Colour = atoi(*atts);
				atts--;
			}
			if (strcmp(*atts, "f")==0) {
				atts++;
				Me->InputInfo.ControlCharacter.Foreground = *atts;
				atts--;
			}
			atts += 2;
		}
		return;
	}
	
	if (strcmp(name, "group")==0) {
		AlphInfo::group NewGroup;
		NewGroup.Colour=-1;
		NewGroup.Label="";
		Me->InputInfo.Groups.push_back(NewGroup);
		while (*atts!=0) {
			if (strcmp(*atts, "name")==0) {
				atts++;
				Me->InputInfo.Groups.back().Description = *atts;
				atts--;
			}
			if (strcmp(*atts, "b")==0) {
				atts++;
				Me->InputInfo.Groups.back().Colour = atoi(*atts);
				atts--;
			}
            if (strcmp(*atts, "label")==0) {
                atts++;
				Me->InputInfo.Groups.back().Label = *atts; 
                atts--;
            }
			atts += 2;
		}
		return;
	}
	
	if (strcmp(name, "s")==0) {
		AlphInfo::character NewCharacter;
		NewCharacter.Colour=-1;
		Me->InputInfo.Groups.back().Characters.push_back(NewCharacter);
		AlphInfo::character& Ch = Me->InputInfo.Groups.back().Characters.back();
		while (*atts!=0) {
			if (strcmp(*atts, "t")==0) {
				atts++;
				Ch.Text = *atts; 
				atts--;
			}
			if (strcmp(*atts, "d")==0) {
				atts++;
				Ch.Display = *atts;
				atts--;
			}
			if (strcmp(*atts, "b")==0) {
			        atts++;
				Ch.Colour = atoi(*atts);
				atts--;
			}
			if (strcmp(*atts, "f")==0) {
			        atts++;
				Ch.Foreground = *atts;
				atts--;
			}
			atts += 2;
		}
		return;
	}
}


void CAlphIO::XML_EndElement(void *userData, const XML_Char *name)
{
	CAlphIO* Me = (CAlphIO*) userData;
	
	if (strcmp(name, "alphabet")==0) {
		Me->Alphabets[Me->InputInfo.AlphID] = Me->InputInfo;
		return;
	}
	
	if (strcmp(name, "train")==0) {
		Me->InputInfo.TrainingFile = Me->CData;
		return;
	}

	if (strcmp(name, "palette")==0) {
	  Me->InputInfo.PreferredColours = Me->CData;
	  return;
	}
}


void CAlphIO::XML_CharacterData(void *userData, const XML_Char *s, int len)
{
	// CAREFUL: s points to a string which is NOT null-terminated.
	
	CAlphIO* Me = (CAlphIO*) userData;
	
	Me->CData.append(s, len);
}
