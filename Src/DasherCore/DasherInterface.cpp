// DasherInterface.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "DasherInterface.h"

#include "CustomColours.h"
#include "DasherViewSquare.h"

#include <iostream>

namespace {
	#include "stdio.h"
}
using namespace Dasher;
using namespace std;

const string CDasherInterface::EmptyString = "";


CDasherInterface::CDasherInterface()
: 
m_Alphabet(0), 
m_pColours(0), 
m_pDasherModel(0), 
m_DashEditbox(0), 
m_DasherScreen(0), 
m_pDasherView(0), 
m_SettingsStore(0), 
m_SettingsUI(0),
m_AlphIO(0), 
m_ColourIO(0), 
AlphabetID(""), 
m_LanguageModelID(-1), 
m_ViewID(-1),
m_MaxBitRate(-1), 
m_DrawKeyboard(false), 
m_ColourMode(0), 
m_Paused(0), 
m_PaletteChange(0),
m_Orientation(Opts::LeftToRight), 
m_UserLocation("usr_"), 
m_SystemLocation("sys_"), 
m_TrainFile(""),
m_DasherFont(""), 
m_DasherFontSize(Opts::Normal), 
m_EditFont(""), 
m_EditFontSize(0)
{
  m_Params = new CLanguageModelParams;
}


CDasherInterface::~CDasherInterface()
{
	delete m_pDasherModel;   // The order of some of these deletions matters
	delete m_Alphabet;      
	delete m_pDasherView;
	delete m_ColourIO;
	delete m_AlphIO;
	delete m_pColours;
	delete m_Params;

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

void CDasherInterface::AddAlphabetFilename(std::string Filename)
{
  m_AlphabetFilenames.push_back(Filename);
}

void CDasherInterface::AddColourFilename(std::string Filename)
{
  m_ColourFilenames.push_back(Filename);
}

void CDasherInterface::CreateDasherModel()
{

  if (m_DashEditbox!=0 && m_LanguageModelID!=-1) 
    {

      // Delete the old model and create a new one

      if( m_pDasherModel != NULL ) {
	delete m_pDasherModel;
      }

      CDasherModel::LanguageModelID NewLanguageModelID;

      switch( m_LanguageModelID ) {
      case 0:
	NewLanguageModelID = CDasherModel::idPPM;
	break;
      case 1:
	NewLanguageModelID = CDasherModel::idWord;
	break;
      }

      m_pDasherModel = new CDasherModel(m_Alphabet, m_DashEditbox, NewLanguageModelID, m_Params, m_Dimensions, m_Eyetracker, m_Paused);
      
      // Train the new language model

      string T = m_Alphabet->GetTrainingFile();
      
      TrainFile(m_SystemLocation+T);
      TrainFile(m_UserLocation+T);

      // Set various parameters
      
      m_pDasherModel->SetControlMode(m_ControlMode);
     
      if (m_MaxBitRate>=0)
	m_pDasherModel->SetMaxBitrate(m_MaxBitRate);
      if (m_ViewID!=-1)
	ChangeView(m_ViewID);
      
    }
  
}


void CDasherInterface::Start()
{
    m_Paused=false;
	if (m_pDasherModel!=0) {
		m_pDasherModel->Start();
        m_pDasherModel->Set_paused(m_Paused);
	}
    if (m_pDasherView!=0) {
        m_pDasherView->ResetSum();
        m_pDasherView->ResetSumCounter();
        m_pDasherView->ResetYAutoOffset();
    }
}


void CDasherInterface::PauseAt(int MouseX, int MouseY)
{
	if (m_DashEditbox!=0) {
		m_DashEditbox->write_to_file();
		if (m_CopyAllOnStop)
			m_DashEditbox->CopyAll();
	}	
	m_Paused=true;
    if (m_pDasherModel!=0) {
        m_pDasherModel->Set_paused(m_Paused);
    }
}

void CDasherInterface::Halt()
{
  if (m_pDasherModel!=0)
    m_pDasherModel->Halt();
}

void CDasherInterface::Unpause(unsigned long Time)
{
	m_Paused=false;
    if (m_pDasherModel!=0) {
		m_pDasherModel->Reset_framerate(Time);
        m_pDasherModel->Set_paused(m_Paused);
    }
    if (m_pDasherView!=0) {
        m_pDasherView->ResetSum();
        m_pDasherView->ResetSumCounter();
    }
}


void CDasherInterface::Redraw()
{
  if (m_pDasherView!=0) 
  {
	m_pDasherView->Render();
    m_pDasherView->Display();
  }

}


void CDasherInterface::Redraw(int iMouseX,int iMouseY)
{
  if (m_pDasherView!=0) 
  {
	m_pDasherView->Render(iMouseX,iMouseY);
    m_pDasherView->Display();
  }

}


void CDasherInterface::TapOn(int MouseX, int MouseY, unsigned long Time)
{
	if (m_pDasherView!=0) 
	{
		m_pDasherView->TapOnDisplay(MouseX, MouseY, Time);
		m_pDasherView->Render(MouseX,MouseY);
		m_pDasherView->Display();
	}
	
	if (m_pDasherModel!=0)
		m_pDasherModel->NewFrame(Time);
}

void CDasherInterface::DrawMousePos(int iMouseX, int iMouseY, int iWhichBox )
{
	m_pDasherView->Render();
	//if (iWhichBox!=-1)
	//m_pDasherView->DrawMousePosBox(iWhichBox, m_iMousePosDist);
  
	m_pDasherView->Display();
}


void CDasherInterface::GoTo(int MouseX, int MouseY)
{
	if (m_pDasherView!=0) 
	{
		m_pDasherView->GoTo(MouseX, MouseY);
		m_pDasherView->Render();
		m_pDasherView->Display();
	}
}

void CDasherInterface::DrawGoTo(int MouseX, int MouseY)
{
	if (m_pDasherView!=0) 
	{
//		m_pDasherView->Render(MouseY,MouseY);
		m_pDasherView->DrawGoTo(MouseX, MouseY);
		m_pDasherView->Display();
	}
}

void CDasherInterface::ChangeAlphabet(const std::string& NewAlphabetID)
{
        if (AlphabetID != NewAlphabetID || NewAlphabetID=="") { // Don't bother doing any of this if
	  AlphabetID = NewAlphabetID; // it's the same alphabet

	  if (!m_AlphIO)
	    m_AlphIO = new CAlphIO(m_SystemLocation, m_UserLocation, m_AlphabetFilenames);

	  m_AlphInfo = m_AlphIO->GetInfo(NewAlphabetID);
	  
	  AlphabetID = m_AlphInfo.AlphID.c_str();

	  std::auto_ptr<CAlphabet> ptrOld(m_Alphabet);   // So we can delete the old alphabet later

	  m_Alphabet = new CAlphabet(m_AlphInfo);
	
	  // Apply options from alphabet

	  m_TrainFile = m_UserLocation + m_Alphabet->GetTrainingFile();

	  // DJW_TODO - control mode
       //   if (m_ControlMode==true) {
         //   m_Alphabet->AddControlSymbol();
        //  }
	
	  // Recreate widgets and language model
	  if (m_DashEditbox!=0)
	    m_DashEditbox->SetInterface(this);
	  if (m_DasherScreen!=0)
	    m_DasherScreen->SetInterface(this);
	  
	  delete m_pDasherModel;
	  m_pDasherModel=0;
	  CreateDasherModel();

	  
	  if (m_Alphabet->GetPalette()!= std::string("") && m_PaletteChange==true) {
	    ChangeColours(m_Alphabet->GetPalette());
	  }
	  
	  Start();
	  
	  // We can only change the orientation after we have called
	  // Start, as this will prompt a redraw, which will fail if the
	  // model hasn't been updated for the new alphabet
	  
	  if (m_Orientation==Opts::Alphabet)
	    ChangeOrientation(Opts::Alphabet);
	  
	  //FIXME - this should really be done when the new view is generated
	  //rather than fixing things up afterwards
	  if (m_pDasherView!=0) {
	    m_pDasherView->SetColourMode(m_ColourMode);
	  }
	}

	if (m_SettingsUI!=0)
	  m_SettingsUI->ChangeAlphabet(AlphabetID);
        if (m_SettingsStore!=0)
	  m_SettingsStore->SetStringOption(Keys::ALPHABET_ID, AlphabetID);
}

std::string CDasherInterface::GetCurrentAlphabet()
{
  return AlphabetID;
}

void CDasherInterface::ChangeColours(const std::string& NewColourID)
{
	if (!m_ColourIO)
		m_ColourIO = new CColourIO(m_SystemLocation, m_UserLocation, m_ColourFilenames);
	m_ColourInfo = m_ColourIO->GetInfo(NewColourID);

	// delete old colours on editing function
	std::auto_ptr<CCustomColours> ptrColours(m_pColours);

	m_pColours = new CCustomColours(m_ColourInfo);

	ColourID=m_ColourInfo.ColourID;

        if (m_SettingsUI!=0)
                m_SettingsUI->ChangeColours(ColourID);
        if (m_SettingsStore!=0)
                m_SettingsStore->SetStringOption(Keys::COLOUR_ID, ColourID);

	if (m_DasherScreen!=0) {
	  m_DasherScreen->SetColourScheme(m_pColours);
	}
}

std::string CDasherInterface::GetCurrentColours() {
  return ColourID;
}

void CDasherInterface::ChangeMaxBitRate(double NewMaxBitRate)
{
	m_MaxBitRate = NewMaxBitRate;
	
	if (m_pDasherModel!=0)
		m_pDasherModel->SetMaxBitrate(m_MaxBitRate);
	if (m_SettingsUI!=0)
		m_SettingsUI->ChangeMaxBitRate(m_MaxBitRate);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetLongOption(Keys::MAX_BITRATE_TIMES100, long(m_MaxBitRate*100) );

	if (m_DrawKeyboard==true && m_pDasherView!=NULL) {
	  m_pDasherView->DrawKeyboard();
	}
}

void CDasherInterface::ChangeLanguageModel(int NewLanguageModelID)
{

  m_LanguageModelID = NewLanguageModelID;
  if (m_Alphabet!=0) {
    
    CreateDasherModel();
    
    // We need to call start here so that the root is recreated, otherwise it will fail (this is probably something which needs to be fixed in a more integrated way)
    
    Start(); 
    
  }
}


void CDasherInterface::ChangeScreen()
{
	if (m_pDasherView!=0) {
		m_pDasherView->ChangeScreen(m_DasherScreen);
	} else {
		if (m_ViewID!=-1)
			ChangeView(m_ViewID);
	}
}


void CDasherInterface::ChangeScreen(CDasherScreen* NewScreen)
{
	m_DasherScreen = NewScreen;
	m_DasherScreen->SetFont(m_DasherFont);
	m_DasherScreen->SetFontSize(m_DasherFontSize);
	m_DasherScreen->SetColourScheme(m_pColours);
	m_DasherScreen->SetInterface(this);
	ChangeScreen();
	Redraw();
}


void CDasherInterface::ChangeView(unsigned int NewViewID)
{
	//TODO Use DasherViewID
	m_ViewID = NewViewID;
	if (m_DasherScreen!=0 && m_pDasherModel!=0) {
		delete m_pDasherView;
		if (m_Orientation==Opts::Alphabet)
			m_pDasherView = new CDasherViewSquare(m_DasherScreen, *m_pDasherModel, GetAlphabetOrientation(), m_ColourMode);
		else
			m_pDasherView = new CDasherViewSquare(m_DasherScreen, *m_pDasherModel, m_Orientation, m_ColourMode);
		m_pDasherView->SetDrawMouse(m_DrawMouse);
		m_pDasherView->SetDrawMouseLine(m_DrawMouseLine);

	}
}


void CDasherInterface::ChangeOrientation(Opts::ScreenOrientations Orientation)
{
	if (m_pDasherView!=0) {
		if (Orientation==Opts::Alphabet)
			m_pDasherView->ChangeOrientation(GetAlphabetOrientation());
		else
			m_pDasherView->ChangeOrientation(Orientation);
	}

	if (m_Orientation != Orientation) {
	  m_Orientation = Orientation;
	  if (m_SettingsUI!=0)
	    m_SettingsUI->ChangeOrientation(Orientation);
	  if (m_SettingsStore!=0)
	    m_SettingsStore->SetLongOption(Keys::SCREEN_ORIENTATION, Orientation);
	}
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
	if (m_pDasherView)
		m_pDasherView->SetDrawMouse(Value);

	if (m_SettingsUI!=0)
		m_SettingsUI->DrawMouse(Value);
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::DRAW_MOUSE, Value);
}

void CDasherInterface::DrawMouseLine(bool Value)
{
    m_DrawMouseLine = Value;
	if (m_pDasherView)
		m_pDasherView->SetDrawMouseLine(Value);
	if (m_SettingsUI!=0)
        m_SettingsUI->DrawMouseLine(Value);
	if (m_SettingsStore!=0)
	    m_SettingsStore->SetBoolOption(Keys::DRAW_MOUSELINE, Value);
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
	if (m_pDasherView!=0)
	  m_pDasherView->SetKeyControl(Value);
}

void CDasherInterface::WindowPause(bool Value)
{
	if (m_SettingsUI!=0)
	  m_SettingsUI->WindowPause(Value);
	if (m_SettingsStore!=0)
	  m_SettingsStore->SetBoolOption(Keys::WINDOW_PAUSE, Value);
}

void CDasherInterface::ControlMode(bool Value)
{
	m_ControlMode=Value;
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::CONTROL_MODE, Value);
	if (m_pDasherModel!=0) 
	{
		m_pDasherModel->SetControlMode(Value);
		// DJW_TODO - control symbol
//		if (Value==true) 
//		{
//			m_Alphabet->AddControlSymbol();
//		} 
//		else 
//		{
//			m_Alphabet->DelControlSymbol();
//		}
	}

	if (m_SettingsUI!=0)
		m_SettingsUI->ControlMode(Value);

  Start();
  Redraw();
}

void CDasherInterface::KeyboardMode(bool Value)
{
  m_KeyboardMode=Value;
  if (m_SettingsUI!=0)
    m_SettingsUI->KeyboardMode(Value);
  if (m_SettingsStore!=0)
    m_SettingsStore->SetBoolOption(Keys::KEYBOARD_MODE, Value);
}

void CDasherInterface::SetDrawMousePosBox(int iWhich)
{
	m_iMousePosBox = iWhich;
	if (m_pDasherView)
		m_pDasherView->SetDrawMousePosBox(iWhich);
	
}

void CDasherInterface::MouseposStart(bool Value)
{
  m_MouseposStart=Value;
  if (!Value && m_pDasherView)
	  m_pDasherView->SetDrawMousePosBox(0);
  
  if (m_SettingsUI!=0)
    m_SettingsUI->MouseposStart(Value);
  if (m_SettingsStore!=0)
    m_SettingsStore->SetBoolOption(Keys::MOUSEPOS_START, Value);

}

void CDasherInterface::OutlineBoxes(bool Value)
{
  if (m_SettingsUI!=0)
    m_SettingsUI->OutlineBoxes(Value);
  if (m_SettingsStore!=0)
    m_SettingsStore->SetBoolOption(Keys::OUTLINE_MODE, Value);
}

void CDasherInterface::PaletteChange(bool Value)
{
  m_PaletteChange=Value;
  if (m_SettingsUI!=0)
    m_SettingsUI->PaletteChange(Value);
  if (m_SettingsStore!=0)
    m_SettingsStore->SetBoolOption(Keys::PALETTE_CHANGE, Value);
}

void CDasherInterface::Speech(bool Value)
{
  if (m_SettingsUI!=0)
    m_SettingsUI->Speech(Value);
  if (m_SettingsStore!=0)
    m_SettingsStore->SetBoolOption(Keys::SPEECH_MODE, Value);
}

void CDasherInterface::SetScreenSize(long Width, long Height)
{
  if (m_SettingsStore!=0) {
    m_SettingsStore->SetLongOption(Keys::SCREEN_HEIGHT, Height);
    m_SettingsStore->SetLongOption(Keys::SCREEN_WIDTH, Width);
  }
}


void CDasherInterface::SetEditHeight(long Value)
{
  if (m_SettingsStore!=0) {
    m_SettingsStore->SetLongOption(Keys::EDIT_HEIGHT, Value);
  }
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

void CDasherInterface::SetUniform(int Value)
{
  if( m_pDasherModel != NULL )
    m_pDasherModel->SetUniform(Value);
  if (m_SettingsStore!=0) {
    m_SettingsStore->SetLongOption(Keys::UNIFORM, Value);
  }
}

void CDasherInterface::SetYScale(int Value)
{
	if (m_SettingsStore!=0) {
		m_SettingsStore->SetLongOption(Keys::YSCALE, Value);
	}
}

void CDasherInterface::SetMousePosDist(int Value)
{
	m_iMousePosDist = Value;
	if (m_SettingsStore!=0) {
		m_SettingsStore->SetLongOption(Keys::MOUSEPOSDIST, Value);
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
	m_DasherFontSize = fontsize;
	if (m_DasherScreen!=0) {
	         m_DasherScreen->SetFontSize(fontsize);
	}
	if (m_SettingsUI!=0) {
	  m_SettingsUI->SetDasherFontSize(fontsize);
	}
	Redraw();
}

void CDasherInterface::SetDasherDimensions(bool Value)
{
        m_Dimensions=Value;
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::DASHER_DIMENSIONS, Value);
	if (m_pDasherModel!=0) {
	         m_pDasherModel->Set_dimensions(Value);
	}
	if (m_SettingsUI!=0) {
	         m_SettingsUI->SetDasherDimensions(Value);
	}	  
}

void CDasherInterface::SetDasherEyetracker(bool Value)
{
        m_Eyetracker=Value;
	if (m_SettingsStore!=0)
		m_SettingsStore->SetBoolOption(Keys::DASHER_EYETRACKER, Value);
	if (m_pDasherModel!=0) {
	         m_pDasherModel->Set_eyetracker(Value);
	}
	if (m_SettingsUI!=0) {
	         m_SettingsUI->SetDasherEyetracker(Value);
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
	  return 4; // Default colour for text
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
		m_AlphIO = new CAlphIO(m_SystemLocation, m_UserLocation, m_AlphabetFilenames);	
	m_AlphIO->GetAlphabets(AlphabetList);
}


const CAlphIO::AlphInfo& CDasherInterface::GetInfo(const std::string& AlphID)
{
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(m_SystemLocation, m_UserLocation, m_AlphabetFilenames);
	
	return m_AlphIO->GetInfo(AlphID);
}


void CDasherInterface::SetInfo(const CAlphIO::AlphInfo& NewInfo)
{
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(m_SystemLocation, m_UserLocation, m_AlphabetFilenames);
	
	m_AlphIO->SetInfo(NewInfo);
}


void CDasherInterface::DeleteAlphabet(const std::string& AlphID)
{
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(m_SystemLocation, m_UserLocation, m_AlphabetFilenames);
	
	m_AlphIO->Delete(AlphID);
}

void CDasherInterface::GetColours(std::vector< std::string >* ColourList)
{
	if (!m_ColourIO)
		m_ColourIO = new CColourIO(m_SystemLocation, m_UserLocation, m_ColourFilenames);	
	m_ColourIO->GetColours(ColourList);
}



void CDasherInterface::ChangeEdit()
{
	CreateDasherModel();
	Start();
	Redraw();
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

void CDasherInterface::ColourMode(bool Value)
{
  if (m_pDasherView!=0) {
    m_pDasherView->SetColourMode(Value);
  }
  m_ColourMode=Value;
  Start();
  Redraw();
}

void CDasherInterface::Train(string* TrainString, bool IsMore)
{
//	m_pDasherModel->LearnText(TrainContext, TrainString, IsMore);
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
	
	vector<symbol> Symbols;
	
	CDasherModel::CTrainer* pTrainer = m_pDasherModel->GetTrainer();
	do {
		NumberRead = fread(InputBuffer, 1, BufferSize-1, InputFile);
		InputBuffer[NumberRead] = '\0';
		StringBuffer += InputBuffer;
		bool bIsMore = false;
		if (NumberRead == (BufferSize-1)) 
			bIsMore = true;

		Symbols.clear();
		m_Alphabet->GetSymbols(&Symbols, &StringBuffer, bIsMore);
		
		pTrainer->Train( Symbols );
		
		
	} while (NumberRead==BufferSize-1);

	delete pTrainer;

	fclose(InputFile);

}

void CDasherInterface::GetFontSizes(std::vector<int> *FontSizes) const
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

void CDasherInterface::AddControlTree(ControlTree *controltree)
{
  m_pDasherModel->NewControlTree(controltree);
}

void CDasherInterface::Render()
{
//  if (m_pDasherView!=0)
 //   m_pDasherView->Render();
}

int CDasherInterface::GetOneButton() {
  if (m_pDasherView!=0) {
    return m_pDasherView->GetOneButton();
  }
  return -1;
}

int CDasherInterface::GetAutoOffset() {
  if (m_pDasherView!=0) {
    return m_pDasherView->GetAutoOffset();
  }
  return -1;
}


void CDasherInterface::SetOneButton(int Value) {
  if (m_pDasherView!=0) {
    m_pDasherView->SetOneButton(Value);
  }
}

/////////////////////////////////////////////////////////////////////////////

double CDasherInterface::GetNats() const
{
	if( m_pDasherModel )
		return m_pDasherModel->GetNats();
	else
		return 0.0;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherInterface::ResetNats() 
{
	if( m_pDasherModel )
		m_pDasherModel->ResetNats();
}

/////////////////////////////////////////////////////////////////////////////

void CDasherInterface::ChangeLMOption( const std::string &pname, long int Value ) 
{
  m_Params->SetValue( pname, Value );

  if (m_SettingsStore!=0)
    m_SettingsStore->SetLongOption( pname, Value);

  if (m_SettingsUI!=0)
     m_SettingsUI->ChangeLMOption( pname, Value );
}
