// ColourIO.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////



#include "ColourIO.h"
using namespace Dasher;
using namespace std;
using namespace expat;


CColourIO::CColourIO(string SystemLocation, string UserLocation, vector<string> Filenames)
	: SystemLocation(SystemLocation), UserLocation(UserLocation),
	  Filenames(Filenames), BlankInfo(), LoadMutable(false), CData("")
{
	CreateDefault();
	
	LoadMutable = false;
	ParseFile(SystemLocation + "colour.xml");
        if (Filenames.size()>0) {
          for (int i=0; i<Filenames.size(); i++) {
            ParseFile(SystemLocation + Filenames[i]);
          }
        }
	LoadMutable = true;
	ParseFile(UserLocation + "colour.xml");
        if (Filenames.size()>0) {
          for (int i=0; i<Filenames.size(); i++) {
            ParseFile(UserLocation + Filenames[i]);
          }
        }
}


void CColourIO::ParseFile(std::string Filename)
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


void CColourIO::GetColours(std::vector< std::string > * ColourList) const
{
	ColourList->clear();
	
	typedef std::map<std::string, ColourInfo>::const_iterator CI;
	CI End = Colours.end();
	
	for (CI Cur=Colours.begin(); Cur!=End; Cur++)
		ColourList->push_back( (*Cur).second.ColourID);
}


const CColourIO::ColourInfo& CColourIO::GetInfo(const std::string& ColourID)
{
  if (ColourID=="") // return Default if no colour scheme is specified
    return Colours["Default"];
  else {
    if (Colours.count(ColourID)!=0) {
      ColourInfo& CurInfo = Colours[ColourID];
      Colours[ColourID].ColourID = ColourID; // Ensure consistency
      return Colours[ColourID];
    } else {
      // if we don't have the colour scheme they asked for, return default
      return Colours["Default"];
    }
  }
}


void CColourIO::SetInfo(const ColourInfo& NewInfo)
{
	Colours[NewInfo.ColourID] = NewInfo;
	Save(NewInfo.ColourID);
}


void CColourIO::Delete(const std::string& ColourID)
{
	if (Colours.find(ColourID)!=Colours.end()) {
		Colours.erase(ColourID);
		Save("");
	}
}


void CColourIO::Save(const std::string& ColourID)
{
	// Write an XML file containing all the colours that have been defined.
	// I am not going to indent the XML file as it will just bloat it, and it
	// is very simple. There are line breaks though as it is very hard to read
	// without. I'm going to ignore ColourID and save all alphabets as the
	// overhead doesn't seem to matter and it makes things much easier.
	
	FILE* Output;
	string Filename = UserLocation + "colours.xml";
	if ((Output = fopen (Filename.c_str(), "w")) == (FILE*)0) {
		// could not open file
	}
	
	fwrite("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", sizeof(char), 39, Output);
	fwrite("<!DOCTYPE alphabets SYSTEM \"colours.dtd\">\n", sizeof(char), 43, Output);
	fwrite("<?xml-stylesheet type=\"text/xsl\" href=\"colours.xsl\"?>\n", sizeof(char), 55, Output);
	fwrite("<colours>\n", sizeof(char), 12, Output);
	
	//	typedef std::map<std::string, AlphInfo>::const_iterator CI;
	//	CI End = Alphabets.end();
	//	for (CI Cur=Alphabets.begin(); Cur!=End; Cur++) {
	//		AlphInfo Info = (*Cur).second; // Take a copy so that special characters can be escaped
	//		
	//		if (Info.Mutable==false) // this is a system alphabet, not one we write
	//			continue;
	//		
	//		fwrite("<alphabet name=\"", sizeof(char), 16, Output);
	//		XML_Escape(&Info.AlphID, true);
	//		fwrite(Info.AlphID.c_str(), sizeof(char), Info.AlphID.size(), Output);
	//		fwrite("\">\n", sizeof(char), 3, Output);
		
	//		fwrite("<orientation type=\"", sizeof(char), 19, Output);
	//		switch (Info.Orientation) {
	//		case Opts::RightToLeft:
	//			fwrite("RL", sizeof(char), 2, Output);
	//			break;
	//		case Opts::TopToBottom:
	//			fwrite("TB", sizeof(char), 2, Output);
	//			break;
	//		case Opts::BottomToTop:
	//			fwrite("BT", sizeof(char), 2, Output);
	//			break;
	//		case Opts::LeftToRight:
	//			// deliberate fall through
	//		default:
	//			fwrite("LR", sizeof(char), 2, Output);
	//			break;
	//		}
	//		fwrite("\"/>\n", sizeof(char), 4, Output);
	//		
	//		// TODO Encoding properly
	//		fwrite("<encoding type=\"", sizeof(char), 16, Output);
	//		fwrite(TtoS[Info.Type].c_str(), sizeof(char), TtoS[Info.Type].size(), Output);
	//		fwrite("\"/>\n", sizeof(char), 4, Output);
	//		
	//		fwrite("<train>", sizeof(char), 7, Output);
	//		XML_Escape(&Info.TrainingFile, false);
	//		fwrite(Info.TrainingFile.c_str(), sizeof(char), Info.TrainingFile.size(), Output);
	//		fwrite("</train>\n", sizeof(char), 9, Output);
	//		
	//		fwrite("<space d=\"", sizeof(char), 10, Output);
	//		XML_Escape(&Info.SpaceCharacter.Display, true);
	//		fwrite(Info.SpaceCharacter.Display.c_str(), sizeof(char), Info.SpaceCharacter.Display.size(), Output);
	//		fwrite("\" t=\"", sizeof(char), 5, Output);
	//		XML_Escape(&Info.SpaceCharacter.Text, true);
	//fwrite(Info.SpaceCharacter.Text.c_str(), sizeof(char), Info.SpaceCharacter.Text.size(), Output);
	//		fwrite("\"/>\n", sizeof(char), 4, Output);
	//		
	//		typedef vector<AlphInfo::group>::iterator gi;
	//		gi LG = Info.Groups.end();
	//		for (gi CG = Info.Groups.begin(); CG!=LG; CG++) {
	//			fwrite("<group name=\"", sizeof(char), 13, Output);
	//			XML_Escape(&CG->Description, true);
	//			fwrite(CG->Description.c_str(), sizeof(char), CG->Description.size(), Output);
	//			fwrite("\">\n", sizeof(char), 3, Output);
	//			
	//			// Iterate over CG->Characters
	//	typedef vector<AlphInfo::character>::iterator ci;
	//			ci LC = CG->Characters.end();
	//			for (ci CC=CG->Characters.begin(); CC!=LC; CC++) {
	//				fwrite("<s d=\"", sizeof(char), 6, Output);
	//				XML_Escape(&CC->Display, true);
	//				fwrite(CC->Display.c_str(), sizeof(char), CC->Display.size(), Output);
	//				fwrite("\" t=\"", sizeof(char), 5, Output);
	//				XML_Escape(&CC->Text, true);
	//				fwrite(CC->Text.c_str(), sizeof(char), CC->Text.size(), Output);
	//				fwrite("\"/>\n", sizeof(char), 4, Output);
	//			}
	//			
	//			fwrite("</group>\n", sizeof(char), 9, Output);
	//		}
	//		
	//		fwrite("</alphabet>\n", sizeof(char), 12, Output);
	//	}
	//	
	//	fwrite("</alphabets>\n", sizeof(char), 13, Output);
	//	
	fclose(Output);
}


void CColourIO::CreateDefault()
{
	ColourInfo& Default = Colours["Default"];
	Default.ColourID = "Default";
	Default.Mutable = false;
	Default.Reds.push_back(255);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(255);
	Default.Greens.push_back(0);
	Default.Blues.push_back(0);
	Default.Reds.push_back(0);
	Default.Greens.push_back(0);
	Default.Blues.push_back(0);
	Default.Reds.push_back(218);
	Default.Greens.push_back(218);
	Default.Blues.push_back(218);
	Default.Reds.push_back(0);
	Default.Greens.push_back(0);
	Default.Blues.push_back(0);
	Default.Reds.push_back(0);
	Default.Greens.push_back(0);
	Default.Blues.push_back(0);
	Default.Reds.push_back(0);
	Default.Greens.push_back(0);
	Default.Blues.push_back(0);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(80);
	Default.Greens.push_back(80);
	Default.Blues.push_back(80);
	Default.Reds.push_back(235);
	Default.Greens.push_back(235);
	Default.Blues.push_back(235);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
	Default.Blues.push_back(255);
	Default.Reds.push_back(180);
	Default.Greens.push_back(238);
	Default.Blues.push_back(180);
	Default.Reds.push_back(155);
	Default.Greens.push_back(205);
	Default.Blues.push_back(155);
	Default.Reds.push_back(0);
	Default.Greens.push_back(255);
Default.Blues.push_back(255);
 Default.Reds.push_back(255);
 Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(255);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(200);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(255);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(200);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(255);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(200);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(255);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(200);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(255);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(200);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(255);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(200);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(255);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(0);
Default.Greens.push_back(0);
Default.Blues.push_back(0);
Default.Reds.push_back(80);
Default.Greens.push_back(80);
Default.Blues.push_back(80);
Default.Reds.push_back(255);
Default.Greens.push_back(255);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(255);
Default.Greens.push_back(174);
Default.Blues.push_back(185);
Default.Reds.push_back(255);
Default.Greens.push_back(187);
Default.Blues.push_back(255);
Default.Reds.push_back(135);
Default.Greens.push_back(206);
Default.Blues.push_back(255);
Default.Reds.push_back(0);
Default.Greens.push_back(255);
Default.Blues.push_back(0);
Default.Reds.push_back(240);
Default.Greens.push_back(240);
Default.Blues.push_back(0);
Default.Reds.push_back(255);
Default.Greens.push_back(0);
Default.Blues.push_back(0);

}


void CColourIO::XML_Escape(std::string* Text, bool Attribute)
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


void CColourIO::XML_StartElement(void *userData, const XML_Char *name, const XML_Char **atts)
{
	CColourIO* Me = (CColourIO*) userData;
	
	Me->CData = "";

	if (strcmp(name, "palette")==0) {
		ColourInfo NewInfo;
		Me->InputInfo = NewInfo;
		Me->InputInfo.Mutable = Me->LoadMutable;
		while (*atts!=0) {
			if (strcmp(*atts, "name")==0) {
				atts++;
				Me->InputInfo.ColourID = *atts;
				atts--;
			}
			atts += 2;
		}
		return;
	}
	if (strcmp(name, "colour")==0) {
            while (*atts!=0) {
	      if (strcmp(*atts, "r")==0) {
		atts++;
		Me->InputInfo.Reds.push_back(atoi(*atts));
		atts--;
	      }
	      if (strcmp(*atts, "g")==0) {
		atts++;
		Me->InputInfo.Greens.push_back(atoi(*atts));
		atts--;
	      }
	      if (strcmp(*atts, "b")==0) {
		atts++;
		Me->InputInfo.Blues.push_back(atoi(*atts));
		atts--;
	      }
	      atts += 2;
	    }
	    return;
        }
}
void CColourIO::XML_EndElement(void *userData, const XML_Char *name)
{
	CColourIO* Me = (CColourIO*) userData;
	
	if (strcmp(name, "palette")==0) {
		Me->Colours[Me->InputInfo.ColourID] = Me->InputInfo;
		return;
	}
}


void CColourIO::XML_CharacterData(void *userData, const XML_Char *s, int len)
{
	// CAREFUL: s points to a string which is NOT null-terminated.
	
	CColourIO* Me = (CColourIO*) userData;
	
	Me->CData.append(s, len);
}
