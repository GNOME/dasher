// DasherInterface.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////



#include "DasherInterface.h"
//#include "EnglishAlphabet.h"
#include "CustomAlphabet.h"
#include "DasherViewSquare.h"
#include "PPMLanguageModel.h"
#include <iostream>
namespace {
	#include "stdio.h"
}
using namespace Dasher;
using namespace std;


const string CDasherInterface::EmptyString = "";


CDasherInterface::CDasherInterface()
	: m_DashEditbox(0), m_DasherScreen(0), m_LanguageModel(0), TrainContext(0), m_Alphabet(0),
	  m_DasherModel(0), m_DasherView(0), AlphabetID(""), LanguageModelID(-1), ViewID(-1),
	  m_MaxBitRate(-1), m_Orientation(Opts::LeftToRight), m_SettingsStore(0), m_SettingsUI(0),
	  m_UserLocation("usr_"), m_SystemLocation("sys_"), m_AlphIO(0), m_TrainFile(""),
	  m_DasherFont(""), m_EditFont(""), m_EditFontSize(0), m_DrawKeyboard(false)
{
}


CDasherInterface::~CDasherInterface()
{
	if (m_LanguageModel)
		m_LanguageModel->ReleaseNodeContext(TrainContext);
	delete m_DasherModel;   // The order of some of these deletions matters
	delete m_LanguageModel; // eg DasherModel has a pointer to LanguageModel.
	delete m_Alphabet;      // DM baulks if LM is deleted before it is.
	delete m_DasherView;
	// Do NOT delete Edit box or Screen. This class did not create them.
}


void CDasherInterface::SetSettingsStore(CSettingsStore* SettingsStore)
{
	delete m_SettingsStore;
	m_SettingsStore = SettingsStore;
	this->SettingsDefaults(m_SettingsStore);
}


void CDasherInterface::SetSettingsUI(CDasherSettingsInterface* SettingsUI)
{
	delete m_SettingsUI;
	m_SettingsUI = SettingsUI;
	//this->SettingsDefaults(m_SettingsStore);
	m_SettingsUI->SettingsDefaults(m_SettingsStore);
}


void CDasherInterface::SetUserLocation(std::string UserLocation)
{
	// Nothing clever updates. (At the moment) it is assumed that
	// this is set before anything much happens and that it does
	// not require changing.
	m_UserLocation = UserLocation;
	if (m_Alphabet!=0)
		m_TrainFile = m_UserLocation + m_Alphabet->GetTrainingFile();
}


void CDasherInterface::SetSystemLocation(std::string SystemLocation)
{
	// Nothing clever updates. (At the moment) it is assumed that
	// this is set before anything much happens and that it does
	// not require changing.
	m_SystemLocation = SystemLocation;
}


void CDasherInterface::CreateDasherModel()
{

  if (m_DashEditbox!=0 && m_LanguageModel!=0) {
    delete m_DasherModel;
    m_DasherModel = new CDasherModel(m_DashEditbox, m_LanguageModel, m_Dimensions);
    if (m_MaxBitRate>=0)
      m_DasherModel->SetMaxBitrate(m_MaxBitRate);
    if (ViewID!=-1)
      ChangeView(ViewID);
  }


}


void CDasherInterface::Start()
{
	if (m_DasherModel!=0)
		m_DasherModel->Start();
}


void CDasherInterface::PauseAt(int MouseX, int MouseY)
{
	if (m_DasherView!=0)
		m_DasherView->FlushAt(MouseX, MouseY);
	if (m_DashEditbox!=0) {
		m_DashEditbox->write_to_file();
		if (m_CopyAllOnStop)
			m_DashEditbox->CopyAll();
	}
}


void CDasherInterface::Unpause(unsigned long Time)
{
	if (m_DashEditbox!=0)
		m_DashEditbox->unflush();
	if (m_DasherModel!=0)
		m_DasherModel->Reset_framerate(Time);
}


void CDasherInterface::Redraw()
{

  if (m_DasherView!=0) {
    m_DasherView->Render();
    m_DasherView->Display();
  }

}


void CDasherInterface::TapOn(int MouseX, int MouseY, unsigned long Time)
{
	if (m_DasherView!=0) {
		m_DasherView->TapOnDisplay(MouseX, MouseY, Time);
		m_DasherView->Render();
		if (m_DrawMouse==true) {
		  m_DasherView->DrawMouse(MouseX, MouseY);
		}
		if (m_DrawKeyboard==true) {
		  m_DasherView->DrawKeyboard();
		}
		m_DasherView->Display();
	}
	if (m_DasherModel!=0)
		m_DasherModel->NewFrame(Time);
}


void CDasherInterface::ChangeAlphabet(const std::string& NewAlphabetID)
{
	if (m_SettingsUI!=0)
		m_SettingsUI->ChangeAlphabet(NewAlphabetID);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetStringOption(Keys::ALPHABET_ID, NewAlphabetID);
	
	AlphabetID = NewAlphabetID;
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(m_SystemLocation, m_UserLocation);
	CAlphIO::AlphInfo Info = m_AlphIO->GetInfo(NewAlphabetID);

	CAlphabet* old = m_Alphabet;
	m_Alphabet = new CCustomAlphabet(Info);
	
	// Apply options from alphabet

	m_TrainFile = m_UserLocation + m_Alphabet->GetTrainingFile();
	
	// Recreate widgets and language model
	if (m_DashEditbox!=0)
		m_DashEditbox->SetInterface(this);
	if (m_DasherScreen!=0)
		m_DasherScreen->SetInterface(this);
	if (LanguageModelID!=-1 || m_LanguageModel)
		ChangeLanguageModel(LanguageModelID);

	delete old; // only delete old alphabet after telling all other objects not to use it

	Start();

	// We can only change the orientation after we have called
	// Start, as this will prompt a redraw, which will fail if the
	// model hasn't been updated for the new alphabet

	if (m_Orientation==Opts::Alphabet)
 		ChangeOrientation(Opts::Alphabet);

	Redraw();
}


void CDasherInterface::ChangeMaxBitRate(double NewMaxBitRate)
{
	m_MaxBitRate = NewMaxBitRate;
	
	if (m_DasherModel!=0)
		m_DasherModel->SetMaxBitrate(m_MaxBitRate);
	if (m_SettingsUI!=0)
		m_SettingsUI->ChangeMaxBitRate(m_MaxBitRate);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetLongOption(Keys::MAX_BITRATE_TIMES100, long(m_MaxBitRate*100) );

	if (m_DrawKeyboard==true && m_DasherView!=NULL) {
	  m_DasherView->DrawKeyboard();
	}
}


void CDasherInterface::ChangeLanguageModel(unsigned int NewLanguageModelID)
{
	LanguageModelID = NewLanguageModelID;
	if (m_Alphabet!=0) {
		if (m_LanguageModel)
			m_LanguageModel->ReleaseNodeContext(TrainContext);
		TrainContext = 0;
		delete m_DasherModel; // Have to delete DasherModel, or removing its LanguageModel will confuse it
		m_DasherModel = 0;
		delete m_LanguageModel;
		// TODO Use LanguageModelID to decide which model to use
		m_LanguageModel = new CPPMLanguageModel(m_Alphabet,1<<10);
		TrainContext = m_LanguageModel->GetRootNodeContext();
		string T = m_Alphabet->GetTrainingFile();
		TrainFile(m_SystemLocation+T);
		TrainFile(m_UserLocation+T);
		CreateDasherModel();
	}
}


void CDasherInterface::ChangeScreen()
{
	if (m_DasherView!=0) {
		m_DasherView->ChangeScreen(m_DasherScreen);
	} else {
		if (ViewID!=-1)
			ChangeView(ViewID);
	}
}


void CDasherInterface::ChangeScreen(CDasherScreen* NewScreen)
{
	m_DasherScreen = NewScreen;
	m_DasherScreen->SetFont(m_DasherFont);
	m_DasherScreen->SetInterface(this);
	ChangeScreen();
	Redraw();
}


void CDasherInterface::ChangeView(unsigned int NewViewID)
{
	//TODO Use DasherViewID
	ViewID = NewViewID;
	if (m_DasherScreen!=0 && m_DasherModel!=0) {
		delete m_DasherView;
		if (m_Orientation==Opts::Alphabet)
			m_DasherView = new CDasherViewSquare(m_DasherScreen, *m_DasherModel, GetAlphabetOrientation());
		else
			m_DasherView = new CDasherViewSquare(m_DasherScreen, *m_DasherModel, m_Orientation);
	}
}


void CDasherInterface::ChangeOrientation(Opts::ScreenOrientations Orientation)
{
	m_Orientation = Orientation;
	if (m_DasherView!=0) {
		if (Orientation==Opts::Alphabet)
			m_DasherView->ChangeOrientation(GetAlphabetOrientation());
		else
			m_DasherView->ChangeOrientation(Orientation);
	}
	if (m_SettingsUI!=0)
		m_SettingsUI->ChangeOrientation(Orientation);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetLongOption(Keys::SCREEN_ORIENTATION, Orientation);
}


void CDasherInterface::SetFileEncoding(Opts::FileEncodingFormats Encoding)
{
	if (m_SettingsUI!=0)
		m_SettingsUI->SetFileEncoding(Encoding);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetLongOption(Keys::FILE_ENCODING, Encoding);
	if (m_DashEditbox)
			m_DashEditbox->SetEncoding(Encoding);
}


void CDasherInterface::ShowToolbar(bool Value)
{
	if (m_SettingsUI!=0)
		m_SettingsUI->ShowToolbar(Value);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::SHOW_TOOLBAR, Value);
}


void CDasherInterface::ShowToolbarText(bool Value)
{
	if (m_SettingsUI!=0)
		m_SettingsUI->ShowToolbarText(Value);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::SHOW_TOOLBAR_TEXT, Value);
}


void CDasherInterface::ShowToolbarLargeIcons(bool Value)
{
	if (m_SettingsUI!=0)
		m_SettingsUI->ShowToolbarLargeIcons(Value);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::SHOW_LARGE_ICONS, Value);
}


void CDasherInterface::ShowSpeedSlider(bool Value)
{
	if (m_SettingsUI!=0)
		m_SettingsUI->ShowSpeedSlider(Value);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::SHOW_SLIDER, Value);
}


void CDasherInterface::FixLayout(bool Value)
{
	if (m_SettingsUI!=0)
		m_SettingsUI->FixLayout(Value);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::FIX_LAYOUT, Value);
}


void CDasherInterface::TimeStampNewFiles(bool Value)
{
	if (m_SettingsUI!=0)
		m_SettingsUI->TimeStampNewFiles(Value);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::TIME_STAMP, Value);
	if (m_DashEditbox!=0)
		m_DashEditbox->TimeStampNewFiles(Value);
}


void CDasherInterface::CopyAllOnStop(bool Value)
{
	m_CopyAllOnStop = Value;
	if (m_SettingsUI!=0)
		m_SettingsUI->CopyAllOnStop(Value);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::COPY_ALL_ON_STOP, Value);
}

void CDasherInterface::DrawMouse(bool Value)
{
        m_DrawMouse = Value;
	if (m_SettingsUI!=0)
                m_SettingsUI->DrawMouse(Value);
	if (m_SettingsStore!=0)
	        m_SettingsStore->SetBoolOption(Keys::DRAW_MOUSE, Value);
}

void CDasherInterface::StartOnSpace(bool Value)
{
        m_StartSpace = Value;
	if (m_SettingsUI!=0)
	  m_SettingsUI->StartOnSpace(Value);
	if (m_SettingsStore!=0)
	  m_SettingsStore->SetBoolOption(Keys::START_SPACE, Value);
}

void CDasherInterface::StartOnLeft(bool Value)
{
  m_StartLeft = Value;
	if (m_SettingsUI!=0)
	  m_SettingsUI->StartOnLeft(Value);
	if (m_SettingsStore!=0)
	  m_SettingsStore->SetBoolOption(Keys::START_MOUSE, Value);
}

void CDasherInterface::KeyControl(bool Value)
{
        m_KeyControl = Value;
	if (m_SettingsUI!=0)
	  m_SettingsUI->KeyControl(Value);
	if (m_SettingsStore!=0)
	  m_SettingsStore->SetBoolOption(Keys::KEY_CONTROL, Value);
}

void CDasherInterface::WindowPause(bool Value)
{
        m_KeyControl = Value;
	if (m_SettingsUI!=0)
	  m_SettingsUI->WindowPause(Value);
	if (m_SettingsStore!=0)
	  m_SettingsStore->SetBoolOption(Keys::WINDOW_PAUSE, Value);
}

void CDasherInterface::SetEditFont(string Name, long Size)
{
	m_EditFont = Name;
	m_EditFontSize = Size;
	if (m_DashEditbox)
		m_DashEditbox->SetFont(Name, Size);
	if (m_SettingsUI!=0)
		m_SettingsUI->SetEditFont(Name, Size);
	if (m_SettingsStore!=0) {
		m_SettingsStore->SetStringOption(Keys::EDIT_FONT, Name);
		m_SettingsStore->SetLongOption(Keys::EDIT_FONT_SIZE, Size);
	}
}


void CDasherInterface::SetDasherFont(string Name)
{
	if (m_SettingsStore!=0)
		m_SettingsStore->SetStringOption(Keys::DASHER_FONT, Name);
	m_DasherFont = Name;
	if (m_DasherScreen!=0)
		m_DasherScreen->SetFont(Name);
	Redraw();
}

void CDasherInterface::SetDasherFontSize(FontSize fontsize)
{
	if (m_SettingsStore!=0)
		m_SettingsStore->SetLongOption(Keys::DASHER_FONTSIZE, fontsize);
	if (m_DasherScreen!=0) {
	         m_DasherScreen->SetFontSize(fontsize);
	}
	Redraw();
}

void CDasherInterface::SetDasherDimensions(bool Value)
{
        m_Dimensions=Value;
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::DASHER_DIMENSIONS, Value);
	if (m_DasherModel!=0) {
	         m_DasherModel->Set_dimensions(Value);
	}
	if (m_SettingsUI!=0) {
	         m_SettingsUI->SetDasherDimensions(Value);
	}	  
}


unsigned int CDasherInterface::GetNumberSymbols()
{
	if (m_Alphabet!=0)
		return m_Alphabet->GetNumberSymbols();
	else
		return 0;
}


const string& CDasherInterface::GetDisplayText(symbol Symbol)
{
	if (m_Alphabet!=0)
		return m_Alphabet->GetDisplayText(Symbol);
	else
		return EmptyString;
}


const string& CDasherInterface::GetEditText(symbol Symbol)
{
	if (m_Alphabet!=0)
		return m_Alphabet->GetText(Symbol);
	else
		return EmptyString;
}

int CDasherInterface::GetTextColour(symbol Symbol)
{
	if (m_Alphabet!=0)
		return m_Alphabet->GetTextColour(Symbol);
	else
	        return 0;
}


Opts::ScreenOrientations CDasherInterface::GetAlphabetOrientation()
{
	return m_Alphabet->GetOrientation();
}


Opts::AlphabetTypes CDasherInterface::GetAlphabetType()
{
	return m_Alphabet->GetType();
}


const std::string& CDasherInterface::GetTrainFile()
{
	return m_TrainFile;
}


void CDasherInterface::GetAlphabets(std::vector< std::string >* AlphabetList)
{
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(m_SystemLocation, m_UserLocation);	
	m_AlphIO->GetAlphabets(AlphabetList);
}


const CAlphIO::AlphInfo& CDasherInterface::GetInfo(const std::string& AlphID)
{
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(m_SystemLocation, m_UserLocation);
	
	return m_AlphIO->GetInfo(AlphID);
}


void CDasherInterface::SetInfo(const CAlphIO::AlphInfo& NewInfo)
{
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(m_SystemLocation, m_UserLocation);
	
	m_AlphIO->SetInfo(NewInfo);
}


void CDasherInterface::DeleteAlphabet(const std::string& AlphID)
{
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(m_SystemLocation, m_UserLocation);
	
	m_AlphIO->Delete(AlphID);
}


void CDasherInterface::ChangeEdit()
{
	CreateDasherModel();
	if (m_DasherModel!=0)
		m_DasherModel->Start();
	if (m_DasherView!=0)
		m_DasherView->Render();
}


void CDasherInterface::ChangeEdit(CDashEditbox* NewEdit)
{
	m_DashEditbox = NewEdit;
	m_DashEditbox->SetFont(m_EditFont, m_EditFontSize);
	m_DashEditbox->SetInterface(this);
	if (m_SettingsStore!=0)
		m_DashEditbox->TimeStampNewFiles(m_SettingsStore->GetBoolOption(Keys::TIME_STAMP));
	m_DashEditbox->New("");
	ChangeEdit();
}


void CDasherInterface::Train(string* TrainString, bool IsMore)
{
	m_LanguageModel->LearnText(TrainContext, TrainString, IsMore);
	return;
}


/*
	I've used C style I/O because I found that C++ style I/O bloated
	the Win32 code enormously. The overhead of loading the buffer into
	the string instead of reading straight into a string seems to be
	negligible compared to huge requirements elsewhere.
*/
void CDasherInterface::TrainFile(string Filename)
{
	if (Filename=="")
		return;
	
	FILE* InputFile;
	if ( (InputFile = fopen(Filename.c_str(), "r")) == (FILE*)0)
		return;
	
	const int BufferSize = 1024;
	char InputBuffer[BufferSize];
	string StringBuffer;
	int NumberRead;
	
	do {
		NumberRead = fread(InputBuffer, 1, BufferSize-1, InputFile);
		InputBuffer[NumberRead] = '\0';
		StringBuffer += InputBuffer;
		if (NumberRead == (BufferSize-1)) {
		  m_LanguageModel->LearnText(TrainContext, &StringBuffer, true);
		} else {
		  m_LanguageModel->LearnText(TrainContext, &StringBuffer, false);
		}
	} while (NumberRead==BufferSize-1);

	fclose(InputFile);
}

void CDasherInterface::GetFontSizes(std::vector<int> *FontSizes)
{
  FontSizes->push_back(20);
  FontSizes->push_back(14);
  FontSizes->push_back(11);
  FontSizes->push_back(40);
  FontSizes->push_back(28);
  FontSizes->push_back(22);
  FontSizes->push_back(80);
  FontSizes->push_back(56);
  FontSizes->push_back(44);
}
  

double CDasherInterface::GetCurCPM()
{
	//
	return 0;
}


double CDasherInterface::GetCurFPS()
{
	//
	return 0;
}
