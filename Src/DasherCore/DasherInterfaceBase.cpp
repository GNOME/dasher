// DasherInterfaceBase.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "DasherInterfaceBase.h"

#include "CustomColours.h"
#include "DasherViewSquare.h"

#include <iostream>
#include <memory>
namespace {
	#include "stdio.h"
}
using namespace Dasher;
using namespace std;

const string CDasherInterfaceBase::EmptyString = "";


CDasherInterfaceBase::CDasherInterfaceBase()
: 
m_Alphabet(0), 
m_pColours(0), 
m_pDasherModel(0), 
m_DashEditbox(0), 
m_DasherScreen(0), 
m_pDasherView(0),  
m_SettingsUI(0),
m_AlphIO(0), 
m_ColourIO(0),
m_pInput(0)
{
  m_Params = new CLanguageModelParams;

  m_pEventHandler = new CEventHandler( this );


// This should be created in the derived class (as it is platform dependent)
 // m_pSettingsStore = new CSettingsStore( m_pEventHandler );
}


CDasherInterfaceBase::~CDasherInterfaceBase()
{
	delete m_pDasherModel;   // The order of some of these deletions matters
	delete m_Alphabet;      
	delete m_pDasherView;
	delete m_ColourIO;
	delete m_AlphIO;
	delete m_pColours;
	delete m_Params;
	delete m_pEventHandler;

	// Do NOT delete Edit box or Screen. This class did not create them.
}


//void CDasherInterfaceBase::SetSettingsStore(CSettingsStore* SettingsStore)
//{
//	delete m_SettingsStore;
//	m_SettingsStore = SettingsStore;
//	this->SettingsDefaults(m_SettingsStore);
//}


void CDasherInterfaceBase::ExternalEventHandler( Dasher::CEvent *pEvent ) {

  // Pass events outside

  if(pEvent->m_iEventType == 1 ) {
    Dasher::CParameterNotificationEvent	*pEvt( static_cast<Dasher::CParameterNotificationEvent* >( pEvent));
    
    if (m_SettingsUI!=0)
      m_SettingsUI->HandleParameterNotification( pEvt->m_iParameter );
  } 
  else if(( pEvent->m_iEventType >= 2) && ( pEvent->m_iEventType <= 5 )) {
    if( m_DashEditbox != NULL )
      m_DashEditbox -> HandleEvent( pEvent );
  }

}

void CDasherInterfaceBase::InterfaceEventHandler( Dasher::CEvent *pEvent ) {

	if(	pEvent->m_iEventType == 1 ) {
			Dasher::CParameterNotificationEvent	*pEvt( static_cast<	Dasher::CParameterNotificationEvent	* >( pEvent	));

			switch(	pEvt->m_iParameter ) {

			case BP_COLOUR_MODE: // Forces us to redraw the display
				Start();
				Redraw();
				break;
	
			case LP_ORIENTATION:
				Start();
				Redraw();
				break;

			default:
				break;
			}
		}
}


void CDasherInterfaceBase::SetSettingsUI(CDasherSettingsInterface* SettingsUI)
{
	delete m_SettingsUI;
	m_SettingsUI = SettingsUI;
	//this->SettingsDefaults(m_SettingsStore);
    m_SettingsUI->SetInterface( this );
	//m_SettingsUI->SettingsDefaults(m_pSettingsStore);
}


void CDasherInterfaceBase::SetUserLocation(std::string UserLocation)
{
	// Nothing clever updates. (At the moment) it is assumed that
	// this is set before anything much happens and that it does
	// not require changing.
	//m_UserLocation = UserLocation;
    SetStringParameter(SP_USER_LOC, UserLocation);

    // BW - COMMENTED OUT - TRAIN_FILE should be FILE NAME only, leave DIR in USER_LOC
	//if (m_Alphabet!=0)
	//	SetStringParameter(SP_TRAIN_FILE, UserLocation + m_Alphabet->GetTrainingFile());
}


void CDasherInterfaceBase::SetSystemLocation(std::string SystemLocation)
{
	// Nothing clever updates. (At the moment) it is assumed that
	// this is set before anything much happens and that it does
	// not require changing.
	//m_SystemLocation = SystemLocation;
    SetStringParameter(SP_SYSTEM_LOC, SystemLocation);    
}

void CDasherInterfaceBase::AddAlphabetFilename(std::string Filename)
{
  m_AlphabetFilenames.push_back(Filename);
}

void CDasherInterfaceBase::AddColourFilename(std::string Filename)
{
  m_ColourFilenames.push_back(Filename);
}

void CDasherInterfaceBase::CreateDasherModel()
{
    int lmID = GetLongParameter(LP_LANGUAGE_MODEL_ID);

    std::cout << "Creating model - LM id is " << lmID << std::endl;

    if (m_DashEditbox!=0 && lmID != -1) 
    {

      // Delete the old model and create a new one

        if( m_pDasherModel != NULL ) {
	        delete m_pDasherModel;
        }


        m_pDasherModel = new CDasherModel( m_pEventHandler, m_pSettingsStore, this);
      
        // Train the new language model

        string T = m_Alphabet->GetTrainingFile();

        TrainFile(GetStringParameter(SP_SYSTEM_LOC)+T);
        TrainFile(GetStringParameter(SP_USER_LOC)+T);

        // Set various parameters

        //m_pDasherModel->SetControlMode(m_ControlMode);

        if(GetLongParameter( LP_VIEW_ID ) !=-1)
              ChangeView( GetLongParameter( LP_VIEW_ID) );
    }
}


void CDasherInterfaceBase::Start()
{
    //m_Paused=false;
    SetBoolParameter(BP_DASHER_PAUSED, false);
	if (m_pDasherModel!=0) {
		m_pDasherModel->Start();
    //    m_pDasherModel->Set_paused(m_Paused);
	}
    if (m_pDasherView!=0) {
        m_pDasherView->ResetSum();
        m_pDasherView->ResetSumCounter();
        m_pDasherView->ResetYAutoOffset();
    }
}


void CDasherInterfaceBase::PauseAt(int MouseX, int MouseY)
{
  if (m_DashEditbox!=0) 
    {
      m_DashEditbox->write_to_file();
      if (GetBoolParameter(BP_COPY_ALL_ON_STOP))
	m_DashEditbox->CopyAll();
    }	
  
  SetBoolParameter(BP_DASHER_PAUSED, true);
	//m_Paused=true;
    //if (m_pDasherModel!=0) {
    //    m_pDasherModel->Set_paused(m_Paused);
    //}

  std::cout << "In PauseAt" << std::endl;

  Dasher::CStopEvent oEvent;
  m_pEventHandler->InsertEvent( &oEvent );
}

void CDasherInterfaceBase::Halt()
{
  if (m_pDasherModel!=0)
    m_pDasherModel->Halt();
}

void CDasherInterfaceBase::Unpause(unsigned long Time)
{
    SetBoolParameter(BP_DASHER_PAUSED, false);

    if (m_pDasherModel!=0) {
      m_pDasherModel->Reset_framerate(Time);
      //m_pDasherModel->Set_paused(m_Paused);
    }
    if (m_pDasherView!=0) {
      m_pDasherView->ResetSum();
      m_pDasherView->ResetSumCounter();
    }

    Dasher::CStartEvent oEvent;
    m_pEventHandler->InsertEvent( &oEvent );
}


void CDasherInterfaceBase::Redraw()
{
  if (m_pDasherView!=0) 
  {
    m_pDasherView->Render();
    m_pDasherView->Display();
  }

}


void CDasherInterfaceBase::Redraw(int iMouseX,int iMouseY)
{
 
  if (m_pDasherView!=0) 
  {
    if( m_pDasherView->Render(iMouseX,iMouseY,false) ) // Only call display if something changed
      m_pDasherView->Display();
  }

}

void CDasherInterfaceBase::SetInput( CDasherInput *_pInput ) {

  m_pInput = _pInput;

  if (m_pDasherView!=0) 
    m_pDasherView->SetInput( _pInput );
}


void CDasherInterfaceBase::TapOn(int MouseX, int MouseY, unsigned long Time)
{
	if (m_pDasherView!=0) 
	{
		m_pDasherView->TapOnDisplay(MouseX, MouseY, Time);
		m_pDasherView->Render(MouseX,MouseY,true);
		m_pDasherView->Display();
	}
	
	if (m_pDasherModel!=0)
		m_pDasherModel->NewFrame(Time);
}

void CDasherInterfaceBase::DrawMousePos(int iMouseX, int iMouseY, int iWhichBox )
{
	m_pDasherView->Render( iMouseX, iMouseY,false );
	//if (iWhichBox!=-1)
	//m_pDasherView->DrawMousePosBox(iWhichBox, m_iMousePosDist);
  
	m_pDasherView->Display();
}


void CDasherInterfaceBase::GoTo(int MouseX, int MouseY)
{
	if (m_pDasherView!=0) 
	{
		m_pDasherView->GoTo(MouseX, MouseY);
		m_pDasherView->Render();
		m_pDasherView->Display();
	}
}

void CDasherInterfaceBase::DrawGoTo(int MouseX, int MouseY)
{
	if (m_pDasherView!=0) 
	{
//		m_pDasherView->Render(MouseY,MouseY);
		m_pDasherView->DrawGoTo(MouseX, MouseY);
		m_pDasherView->Display();
	}
}

void CDasherInterfaceBase::ChangeAlphabet(const std::string& NewAlphabetID)
{
      // Don't bother doing any of this if it's the same alphabet
    //  if (GetStringParameter(SP_ALPHABET_ID) != NewAlphabetID) { 
	        
   //     SetStringParameter(SP_ALPHABET_ID, NewAlphabetID); 

  // FIXME - we shouldn't rely on the first call to ChangeAlphabet to
  // construct the list of filenames - we may need to populate a list
  // dialogue before this happens - also, what happens if the list of
  // alphabet files changes at runtime?

	    if (!m_AlphIO)
	        m_AlphIO = new CAlphIO(GetStringParameter(SP_SYSTEM_LOC), 
                            GetStringParameter(SP_USER_LOC), m_AlphabetFilenames);

	    m_AlphInfo = m_AlphIO->GetInfo(NewAlphabetID);
    	  
	    //AlphabetID = m_AlphInfo.AlphID.c_str();

	    std::auto_ptr<CAlphabet> ptrOld(m_Alphabet);   // So we can delete the old alphabet later

	    m_Alphabet = new CAlphabet(m_AlphInfo);
    	
	    // Apply options from alphabet

	    SetStringParameter(SP_TRAIN_FILE, m_Alphabet->GetTrainingFile());
        SetStringParameter(SP_GAME_TEXT_FILE, m_Alphabet->GetGameModeFile());

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

    	  
	    if (m_Alphabet->GetPalette()!= std::string("") && GetBoolParameter(BP_PALETTE_CHANGE)) {
	        ChangeColours(m_Alphabet->GetPalette());
	    }
    	  
	    Start();

	//}
}

std::string CDasherInterfaceBase::GetCurrentAlphabet()
{
  return GetStringParameter(SP_ALPHABET_ID);
}

void CDasherInterfaceBase::ChangeColours(const std::string& NewColourID)
{
	if (!m_ColourIO)
		m_ColourIO = new CColourIO(GetStringParameter(SP_SYSTEM_LOC), 
                            GetStringParameter(SP_USER_LOC), m_ColourFilenames);
	m_ColourInfo = m_ColourIO->GetInfo(NewColourID);

	// delete old colours on editing function
	std::auto_ptr<CCustomColours> ptrColours(m_pColours);

	m_pColours = new CCustomColours(m_ColourInfo);

	//ColourID=m_ColourInfo.ColourID;

    SetStringParameter(SP_COLOUR_ID, NewColourID);

	if (m_DasherScreen!=0) {
	  m_DasherScreen->SetColourScheme(m_pColours);
	}
}

std::string CDasherInterfaceBase::GetCurrentColours() {
  return GetStringParameter(SP_COLOUR_ID);
}

void CDasherInterfaceBase::ChangeMaxBitRate(double NewMaxBitRate)
{

	// FIXME - make this function integer

	SetLongParameter( LP_MAX_BITRATE, (int)NewMaxBitRate*100 );


	// FIXME - get rid of the below somewhere
    // If this is really important to do right away, listen for change in setting
	if (GetBoolParameter(BP_KEYBOARD_MODE) && m_pDasherView!=NULL) {
	    m_pDasherView->DrawKeyboard();
	}
}

void CDasherInterfaceBase::ChangeLanguageModel(int NewLanguageModelID)
{

    if( NewLanguageModelID != GetLongParameter(LP_LANGUAGE_MODEL_ID) ) {
        SetLongParameter(LP_LANGUAGE_MODEL_ID, NewLanguageModelID);

        if (m_Alphabet!=0) {
          
            CreateDasherModel();
              
            // We need to call start here so that the root is recreated,
            // otherwise it will fail (this is probably something which
            // needs to be fixed in a more integrated way)
              
            Start(); 
          
        }
    }
}


void CDasherInterfaceBase::ChangeScreen()
{
	if (m_pDasherView!=0) {
		m_pDasherView->ChangeScreen(m_DasherScreen);
	} else {
      if(GetLongParameter( LP_VIEW_ID ) !=-1)
               ChangeView( GetLongParameter( LP_VIEW_ID) );
		
	}
}


void CDasherInterfaceBase::ChangeScreen(CDasherScreen* NewScreen)
{
	m_DasherScreen = NewScreen;
	m_DasherScreen->SetFont(GetStringParameter(SP_DASHER_FONT));
	m_DasherScreen->SetFontSize(static_cast<Dasher::Opts::FontSize> (GetLongParameter(LP_DASHER_FONTSIZE)));
	m_DasherScreen->SetColourScheme(m_pColours);
	m_DasherScreen->SetInterface(this);
	ChangeScreen();
	Redraw();
}


void CDasherInterfaceBase::ChangeView(unsigned int NewViewID)
{

  //  std::cout << "In ChangeView" << std::endl;

	//TODO Use DasherViewID
	SetLongParameter(LP_VIEW_ID, NewViewID);
	if (m_DasherScreen!=0 && m_pDasherModel!=0) {
	  delete m_pDasherView;
	  m_pDasherView = new CDasherViewSquare(m_pEventHandler, m_pSettingsStore, m_DasherScreen, *m_pDasherModel );
	  m_pDasherView->SetInput( m_pInput );
	}
}


void CDasherInterfaceBase::ChangeOrientation(Opts::ScreenOrientations Orientation)
{
	// FIXME - make alphabet orientation another parameter (non-stored), and make view read this itself

	if (Orientation==Opts::Alphabet)
		SetLongParameter( LP_ORIENTATION, GetAlphabetOrientation() );
	else
		SetLongParameter( LP_ORIENTATION, Orientation );

//



	   
	
}


void CDasherInterfaceBase::SetFileEncoding(Opts::FileEncodingFormats Encoding)
{
	//if (m_SettingsUI!=0)
	//	m_SettingsUI->SetFileEncoding(Encoding);
	if (m_pSettingsStore!=0)
		SetLongParameter(LP_FILE_ENCODING, Encoding);
	if (m_DashEditbox)
			m_DashEditbox->SetEncoding(Encoding);
}


void CDasherInterfaceBase::ShowToolbar(bool Value)
{
	//if (m_SettingsUI!=0)
	//	m_SettingsUI->ShowToolbar(Value);
	if (m_pSettingsStore!=0)
		SetBoolParameter(BP_SHOW_TOOLBAR, Value);
}


void CDasherInterfaceBase::ShowToolbarText(bool Value)
{
	//if (m_SettingsUI!=0)
	//	m_SettingsUI->ShowToolbarText(Value);
	if (m_pSettingsStore!=0)
		SetBoolParameter(BP_SHOW_TOOLBAR_TEXT, Value);
}


void CDasherInterfaceBase::ShowToolbarLargeIcons(bool Value)
{
    //if (m_SettingsUI!=0)
	//    m_SettingsUI->ShowToolbarLargeIcons(Value);
	//if (m_SettingsStore!=0)
		SetBoolParameter(BP_SHOW_LARGE_ICONS, Value);
}


void CDasherInterfaceBase::ShowSpeedSlider(bool Value)
{
	//if (m_SettingsUI!=0)
	//	m_SettingsUI->ShowSpeedSlider(Value);
	//if (m_SettingsStore!=0)
		SetBoolParameter(BP_SHOW_SLIDER, Value);
}


void CDasherInterfaceBase::FixLayout(bool Value)
{
	//if (m_SettingsUI!=0)
	//	m_SettingsUI->FixLayout(Value);
	//if (m_SettingsStore!=0)
		SetBoolParameter(BP_FIX_LAYOUT, Value);
}


void CDasherInterfaceBase::TimeStampNewFiles(bool Value)
{
	//if (m_SettingsUI!=0)
	//	m_SettingsUI->TimeStampNewFiles(Value);
	if (m_pSettingsStore!=0)
		SetBoolParameter(BP_TIME_STAMP, Value);
	if (m_DashEditbox!=0)
		m_DashEditbox->TimeStampNewFiles(Value);
}


void CDasherInterfaceBase::CopyAllOnStop(bool Value)
{
	//if (m_SettingsUI!=0)
	//	m_SettingsUI->CopyAllOnStop(Value);
	if (m_pSettingsStore!=0)
		SetBoolParameter(BP_COPY_ALL_ON_STOP, Value);
}

void CDasherInterfaceBase::DrawMouse(bool Value)
{
	// Obsolete method - call SetBoolParameter directly
	SetBoolParameter( BP_DRAW_MOUSE, Value );
}

void CDasherInterfaceBase::DrawMouseLine(bool Value)
{
	// Obsolete method - call SetBoolParameter directly
	SetBoolParameter( BP_DRAW_MOUSE_LINE, Value );
}

void CDasherInterfaceBase::StartOnSpace(bool Value)
{
	//if (m_SettingsUI!=0)
	//  m_SettingsUI->StartOnSpace(Value);
	if (m_pSettingsStore!=0)
	  SetBoolParameter(BP_START_SPACE, Value);
}

void CDasherInterfaceBase::StartOnLeft(bool Value)
{
	//if (m_SettingsUI!=0)
	//  m_SettingsUI->StartOnLeft(Value);
	if (m_pSettingsStore!=0)
	  SetBoolParameter(BP_START_MOUSE, Value);
}

void CDasherInterfaceBase::KeyControl(bool Value)
{
	//if (m_SettingsUI!=0)
	//  m_SettingsUI->KeyControl(Value);
	if (m_pSettingsStore!=0)
	  SetBoolParameter(BP_KEY_CONTROL, Value);
	//if (m_pDasherView!=0)
	//  m_pDasherView->SetKeyControl(Value);
}

void CDasherInterfaceBase::WindowPause(bool Value)
{
	//if (m_SettingsUI!=0)
	//  m_SettingsUI->WindowPause(Value);
	if (m_pSettingsStore!=0)
	  SetBoolParameter(BP_WINDOW_PAUSE, Value);
}

void CDasherInterfaceBase::ControlMode(bool Value)
{
	if (m_pSettingsStore!=0)
		SetBoolParameter(BP_CONTROL_MODE, Value);
	if (m_pDasherModel!=0) 
	{
	//	m_pDasherModel->SetControlMode(Value);
    // DJW_TODO - control symbol
		if (Value==true) 
		{
			//m_Alphabet->AddControlSymbol();
		} 
		else 
		{
			//m_Alphabet->DelControlSymbol();
		}
	}

	//if (m_SettingsUI!=0)
	//	m_SettingsUI->ControlMode(Value);


  Start();
  Redraw();
}

void CDasherInterfaceBase::KeyboardMode(bool Value)
{
 // if (m_SettingsUI!=0)
 //   m_SettingsUI->KeyboardMode(Value);
  if (m_pSettingsStore!=0)
    SetBoolParameter(BP_KEYBOARD_MODE, Value);
}

void CDasherInterfaceBase::SetDrawMousePosBox(int iWhich)
{
	SetLongParameter(LP_MOUSE_POS_BOX, iWhich);
	//if (m_pDasherView)
	//	m_pDasherView->SetDrawMousePosBox(iWhich);
	
}

void CDasherInterfaceBase::MouseposStart(bool Value)
{
    SetBoolParameter(BP_MOUSEPOS_MODE, Value);
}

void CDasherInterfaceBase::OutlineBoxes(bool Value)
{
    SetBoolParameter(BP_OUTLINE_MODE, Value);
}

void CDasherInterfaceBase::PaletteChange(bool Value)
{
    SetBoolParameter(BP_PALETTE_CHANGE, Value);
}

void CDasherInterfaceBase::Speech(bool Value)
{
    SetBoolParameter(BP_SPEECH_MODE, Value);
}

void CDasherInterfaceBase::SetScreenSize(long Width, long Height)
{
  if (m_pSettingsStore!=0) {
    SetLongParameter(LP_SCREEN_HEIGHT, Height);
    SetLongParameter(LP_SCREEN_WIDTH, Width);
  }
}


void CDasherInterfaceBase::SetEditHeight(long Value)
{
  if (m_pSettingsStore!=0) {
    SetLongParameter(LP_EDIT_HEIGHT, Value);
  }
}

void CDasherInterfaceBase::SetEditFont(string Name, long Size)
{
	if (m_DashEditbox)
		m_DashEditbox->SetFont(Name, Size);
	//if (m_SettingsUI!=0)
	//	m_SettingsUI->SetEditFont(Name, Size);
	if (m_pSettingsStore!=0) {
		SetStringParameter(SP_EDIT_FONT, Name);
		SetLongParameter(LP_EDIT_FONT_SIZE, Size);
	}
}

void CDasherInterfaceBase::SetUniform(int Value)
{
  //if( m_pDasherModel != NULL )
  //  m_pDasherModel->SetUniform(Value);
  if (m_pSettingsStore!=0) {
    SetLongParameter(LP_UNIFORM, Value);
  }
}

void CDasherInterfaceBase::SetYScale(int Value)
{
	if (m_pSettingsStore!=0) {
		SetLongParameter(LP_YSCALE, Value);
	}
}

void CDasherInterfaceBase::SetMousePosDist(int Value)
{
	if (m_pSettingsStore!=0) {
		SetLongParameter(LP_MOUSEPOSDIST, Value);
	}

}

void CDasherInterfaceBase::SetDasherFont(string Name)
{
	if (m_pSettingsStore!=0)
		SetStringParameter(SP_DASHER_FONT, Name);
	if (m_DasherScreen!=0)
		m_DasherScreen->SetFont(Name);
	Redraw();
}

void CDasherInterfaceBase::SetDasherFontSize(FontSize fontsize)
{
	if (m_pSettingsStore!=0)
		SetLongParameter(LP_DASHER_FONTSIZE, fontsize);
	if (m_DasherScreen!=0) {
	         m_DasherScreen->SetFontSize(fontsize);
	}
	//if (m_SettingsUI!=0) {
	//  m_SettingsUI->SetDasherFontSize(fontsize);
	//}
	Redraw();
}

void CDasherInterfaceBase::SetDasherDimensions(bool Value)
{
    // WHY IS THIS A LONG PARAMETER?
	if (m_pSettingsStore!=0)
		SetLongParameter(LP_DASHER_DIMENSIONS, Value);
	//if (m_pDasherModel!=0) {
	//         m_pDasherModel->Set_dimensions(Value);
	//}
	//if (m_SettingsUI!=0) {
	//         m_SettingsUI->SetDasherDimensions(Value);
	//}	  
}

void CDasherInterfaceBase::SetDasherEyetracker(bool Value)
{
	if (m_pSettingsStore!=0)
		SetLongParameter(LP_DASHER_EYETRACKER, Value);
	//if (m_pDasherModel!=0) {
	//         m_pDasherModel->Set_eyetracker(Value);
	//}
	//if (m_SettingsUI!=0) {
	//         m_SettingsUI->SetDasherEyetracker(Value);
	//}	  
}

void CDasherInterfaceBase::SetTruncation( int Value ) {

  //  std::cout << "In SetTruncation: " << m_pDasherView << std::endl;

  //if( m_pDasherView ) {
  //  m_pDasherView->SetTruncation( Value );
  //}
}

void CDasherInterfaceBase::SetTruncationType( int Value ) {
  //if( m_pDasherView ) {
  //  m_pDasherView->SetTruncationType( Value );
  //}
}

unsigned int CDasherInterfaceBase::GetNumberSymbols()
{
	if (m_Alphabet!=0)
		return m_Alphabet->GetNumberSymbols();
	else
		return 0;
}


const string& CDasherInterfaceBase::GetDisplayText(symbol Symbol)
{
	if (m_Alphabet!=0)
		return m_Alphabet->GetDisplayText(Symbol);
	else
		return EmptyString;
}


const string& CDasherInterfaceBase::GetEditText(symbol Symbol)
{
	if (m_Alphabet!=0)
		return m_Alphabet->GetText(Symbol);
	else
		return EmptyString;
}

int CDasherInterfaceBase::GetTextColour(symbol Symbol)
{
	if (m_Alphabet!=0)
		return m_Alphabet->GetTextColour(Symbol);
	else
	  return 4; // Default colour for text
}


Opts::ScreenOrientations CDasherInterfaceBase::GetAlphabetOrientation()
{
	return m_Alphabet->GetOrientation();
}


Opts::AlphabetTypes CDasherInterfaceBase::GetAlphabetType()
{
	return m_Alphabet->GetType();
}


const std::string CDasherInterfaceBase::GetTrainFile()
{
    // DOES NOT RETURN FULLY QUALIFIED PATH - SEPARATE SETTING FOR PATH
	return GetStringParameter(SP_TRAIN_FILE);
}


void CDasherInterfaceBase::GetAlphabets(std::vector< std::string >* AlphabetList)
{
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(GetStringParameter(SP_SYSTEM_LOC), 
                             GetStringParameter(SP_USER_LOC), m_AlphabetFilenames);	
	m_AlphIO->GetAlphabets(AlphabetList);
}


const CAlphIO::AlphInfo& CDasherInterfaceBase::GetInfo(const std::string& AlphID)
{
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(GetStringParameter(SP_SYSTEM_LOC), 
                            GetStringParameter(SP_USER_LOC), m_AlphabetFilenames);
	
	return m_AlphIO->GetInfo(AlphID);
}


void CDasherInterfaceBase::SetInfo(const CAlphIO::AlphInfo& NewInfo)
{
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(GetStringParameter(SP_SYSTEM_LOC),
                         GetStringParameter(SP_USER_LOC), m_AlphabetFilenames);
	
	m_AlphIO->SetInfo(NewInfo);
}


void CDasherInterfaceBase::DeleteAlphabet(const std::string& AlphID)
{
	if (!m_AlphIO)
		m_AlphIO = new CAlphIO(GetStringParameter(SP_SYSTEM_LOC),
                         GetStringParameter(SP_USER_LOC), m_AlphabetFilenames);
	
	m_AlphIO->Delete(AlphID);
}

void CDasherInterfaceBase::GetColours(std::vector< std::string >* ColourList)
{
	if (!m_ColourIO)
		m_ColourIO = new CColourIO(GetStringParameter(SP_SYSTEM_LOC), 
                    GetStringParameter(SP_USER_LOC), m_ColourFilenames);	
	m_ColourIO->GetColours(ColourList);
}



void CDasherInterfaceBase::ChangeEdit()
{
	CreateDasherModel();
	Start();
	Redraw();
}


void CDasherInterfaceBase::ChangeEdit(CDashEditbox* NewEdit)
{
	m_DashEditbox = NewEdit;
	m_DashEditbox->SetFont(GetStringParameter(SP_EDIT_FONT), GetLongParameter(LP_EDIT_FONT_SIZE));
	m_DashEditbox->SetInterface(this);
	//if (m_SettingsStore!=0)
	//    m_DashEditbox->TimeStampNewFiles(m_SettingsStore->GetBoolOption(Keys::TIME_STAMP));
	m_DashEditbox->New("");
	ChangeEdit();
}

void CDasherInterfaceBase::ColourMode(bool Value)
{

// Obsolete method - call SetBoolParameter directly

	SetBoolParameter( BP_COLOUR_MODE, Value );

}

void CDasherInterfaceBase::Train(string* TrainString, bool IsMore)
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
void CDasherInterfaceBase::TrainFile(string Filename)
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

void CDasherInterfaceBase::GetFontSizes(std::vector<int> *FontSizes) const
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
  

double CDasherInterfaceBase::GetCurCPM()
{
	//
	return 0;
}


double CDasherInterfaceBase::GetCurFPS()
{
	//
	return 0;
}

void CDasherInterfaceBase::AddControlTree(ControlTree *controltree)
{
  m_pDasherModel->NewControlTree(controltree);
}

void CDasherInterfaceBase::Render()
{
//  if (m_pDasherView!=0)
 //   m_pDasherView->Render();
}

int CDasherInterfaceBase::GetAutoOffset() {
  if (m_pDasherView!=0) {
    return m_pDasherView->GetAutoOffset();
  }
  return -1;
}


/////////////////////////////////////////////////////////////////////////////

double CDasherInterfaceBase::GetNats() const
{
	if( m_pDasherModel )
		return m_pDasherModel->GetNats();
	else
		return 0.0;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherInterfaceBase::ResetNats() 
{
	if( m_pDasherModel )
		m_pDasherModel->ResetNats();
}

/////////////////////////////////////////////////////////////////////////////

void CDasherInterfaceBase::ChangeLMOption( const std::string &pname, long int Value ) 
{
  m_Params->SetValue( pname, Value );

  if (m_pSettingsStore!=0)
    m_pSettingsStore->SetLongOption( pname, Value);

  //if (m_SettingsUI!=0)
  //   m_SettingsUI->ChangeLMOption( pname, Value );
}
