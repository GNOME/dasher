// DasherInterfaceBase.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////



#ifndef __DasherInterfaceBase_h__
#define __DasherInterfaceBase_h__

// TODO - there is a list of things to be configurable in my notes
// Check that everything that is not self-contained within the GUI is covered.

#include "../Common/NoClones.h"

#include "DasherWidgetInterface.h"
#include "DasherAppInterface.h"
#include "DasherSettingsInterface.h"

#include "DasherScreen.h"
#include "Alphabet/Alphabet.h"
#include "Alphabet/AlphIO.h"
#include "CustomColours.h"
#include "ColourIO.h"
#include "LanguageModelling/LanguageModelParams.h"
#include "DashEdit.h"
#include "DasherView.h"
#include "DasherInput.h"

#include "EventHandler.h"
#include "Event.h"

#include <map>

namespace Dasher {class CDasherInterfaceBase;}
class Dasher::CDasherInterfaceBase : private NoClones,
	public CDasherWidgetInterface, public CDasherAppInterface, public CDasherSettingsInterface
{
public:
	CDasherInterfaceBase();
	virtual ~CDasherInterfaceBase();

	// New externally visible interface
    void SetBoolParameter( int iParameter, bool bValue ) {
	    m_pSettingsStore->SetBoolParameter( iParameter, bValue );
    };

    void SetLongParameter( int iParameter, long lValue ) {
	    m_pSettingsStore->SetLongParameter( iParameter, lValue );
    };

    void SetStringParameter( int iParameter, const std::string &sValue ) {
	    m_pSettingsStore->SetStringParameter( iParameter, sValue );
    };

    bool GetBoolParameter( int iParameter ) {
        return m_pSettingsStore->GetBoolParameter( iParameter );
    }

    long GetLongParameter( int iParameter ) {
        return m_pSettingsStore->GetLongParameter( iParameter );
    }

    std::string GetStringParameter( int iParameter ) {
        return m_pSettingsStore->GetStringParameter( iParameter );
    }

    void ExternalEventHandler( Dasher::CEvent *pEvent );
    void InterfaceEventHandler( Dasher::CEvent *pEvent );


	// ---

	//! Tell the core which CSettingsStore should be used
	//void SetSettingsStore(CSettingsStore* SettingsStore);

	//! Tell the core which CDasherSettingsInterface should be used
	//
	//! Provide a pointer to an instance of CDasherSettingsInterface in 
	//! order to allow for platform dependent configuration of certain 
	//! options
	void SetSettingsUI(CDasherSettingsInterface* SettingsUI);
	
	//! Set the path for user specific configuration and files
	void SetUserLocation(std::string UserLocation);

	//! Set the path for system-wide configuration and files
	void SetSystemLocation(std::string SystemLocation);
	
	//! Add an alphabet filename
	void AddAlphabetFilename(std::string Filename);

	//! Add a colour filename
	void AddColourFilename(std::string Filename);


	// Widget Interface
	// -----------------------------------------------------
	void Start();
	
	void TapOn(int MouseX, int MouseY, unsigned long Time); // Times in milliseconds
	void PauseAt(int MouseX, int MouseY);                   // are required to make
	void Halt();
	void Unpause(unsigned long Time);                       // Dasher run at the
	void Redraw();                                          // correct speed.
	void Redraw(int iMouseX,int iMouseY);

	void DrawMousePos(int MouseX, int MouseY, int iWhichBox);
	void GoTo(int MouseX, int MouseY);
	void DrawGoTo(int MouseX, int MouseY);
	
	void ChangeScreen(); // The widgets need to tell the engine when they have been
	void ChangeEdit();   // affected by external interaction
	
	unsigned int GetNumberSymbols();           // These are needed so widgets know
	const std::string& GetDisplayText(symbol Symbol); // how to render the alphabet. All
	const std::string& GetEditText(symbol Symbol);    // strings are encoded in UTF-8
	int GetTextColour(symbol Symbol);    // the foreground colour of the text
	Opts::ScreenOrientations GetAlphabetOrientation();
	Opts::AlphabetTypes GetAlphabetType();
	const std::string GetTrainFile();
	
	// App Interface
	// -----------------------------------------------------
	
	// std::map<int, std::string>& GetAlphabets(); // map<key, value> int is a UID string can change. Store UID in preferences. Display string to user.
	// std::vector<std::string>& GetAlphabets();
	// std::vector<std::string>& GetLangModels();
	// std::vector<std::string>& GetViews();
	
	void ChangeScreen(CDasherScreen* NewScreen); // We may change the widgets Dasher uses
	void ChangeEdit(CDashEditbox* NewEdit);      // at run time.
	
	void Train(std::string* TrainString, bool IsMore); // Training by string segments or file
	void TrainFile(std::string Filename);              // all training data must be in UTF-8.

	void GetFontSizes(std::vector<int> *FontSizes) const;
	
	double GetCurCPM(); // App may want to display characters per minute
	double GetCurFPS(); // or frames per second.
	
	// Customize alphabet
	void GetAlphabets(std::vector< std::string >* AlphabetList);
	const CAlphIO::AlphInfo& GetInfo(const std::string& AlphID);
	void SetInfo(const CAlphIO::AlphInfo& NewInfo);
	void DeleteAlphabet(const std::string& AlphID);

	void GetColours(std::vector< std::string >* ColourList);
	void AddControlTree(ControlTree *controltree);

	/////////////////////////////////////////////////////////////////////////////
	// Settings Interface (CDasherSettingsInterface) - options saved between sessions
	/////////////////////////////////////////////////////////////////////////////

	//! Should Dasher start based on the mouse position?
	virtual void MouseposStart(bool Value);

	void ChangeAlphabet(const std::string& NewAlphabetID);
	std::string GetCurrentAlphabet();
	void ChangeColours(const std::string& NewColourID);
	std::string GetCurrentColours();
	void ChangeMaxBitRate(double NewMaxBitRate);

	// DJW - nasty thing about this is - we dont necessarily want the LM to rebuild every
	// time a parameter is change - e.g. if we change 2 or 3 params in a row ???
	void ChangeLanguageModel(int NewLanguageModelID);
	void ChangeLMOption( const std::string &pname, long int Value );

	void ChangeView(unsigned int NewViewID);
	void ChangeOrientation(Opts::ScreenOrientations Orientation);
	void SetFileEncoding(Opts::FileEncodingFormats Encoding);
	// TODO Color customization.
	
	void ShowToolbar(bool Value);
	void ShowToolbarText(bool Value);
	void ShowToolbarLargeIcons(bool Value);
	void ShowSpeedSlider(bool Value);
	void FixLayout(bool Value);
	void TimeStampNewFiles(bool Value);
	void CopyAllOnStop(bool Value);
	void DrawMouse(bool Value);
	void DrawMouseLine(bool Value);
	void StartOnSpace(bool Value);
	void StartOnLeft(bool Value);
	void KeyControl(bool Value);
	void WindowPause(bool Value);
	void ControlMode(bool Value);
	void ColourMode(bool Value);
	void KeyboardMode(bool Value);


	void Speech(bool Value);
	void OutlineBoxes(bool Value);
	void PaletteChange(bool Value);
	void SetScreenSize(long Width, long Height);
	void SetEditHeight(long Value);
	void SetEditFont(std::string Name, long Size);
	void SetDasherFont(std::string Name);
	void SetDasherFontSize(FontSize fontsize);
	void SetDasherDimensions(bool Value);
	void SetDasherEyetracker(bool Value);
	void SetUniform(int Value);
	void SetYScale(int Value);
	void SetMousePosDist(int Value);
	void SetTruncation( int Value );
	void SetTruncationType( int Value );
	// Which mouse position box should the View draw?
	// 0 - no box, 1 - upper box, 2 - lower box
	void SetDrawMousePosBox(int iWhich);
	
	int GetAutoOffset();
	void Render();
	
	void SetInput( CDasherInput *_pInput );
	
	
	double GetNats() const;

	void ResetNats();

    CAlphabet* GetAlphabet() { return m_Alphabet; }
    CLanguageModelParams* GetLMParams() { return m_Params; }
    CDashEditbox* GetEditbox() { return m_DashEditbox; }

private:
	CAlphabet* m_Alphabet;
	CLanguageModelParams *m_Params;
	CCustomColours* m_pColours;
	CDasherModel* m_pDasherModel;
	CDashEditbox* m_DashEditbox;
	CDasherScreen* m_DasherScreen;
	CDasherView* m_pDasherView;
	
	CDasherSettingsInterface* m_SettingsUI;
	CAlphIO* m_AlphIO;	
	CAlphIO::AlphInfo m_AlphInfo;
	CColourIO* m_ColourIO;
	CColourIO::ColourInfo m_ColourInfo;

	std::vector<std::string> m_AlphabetFilenames;
	std::vector<std::string> m_ColourFilenames;

	static const std::string EmptyString;
	
	void CreateDasherModel();

protected:
	CEventHandler *m_pEventHandler;
	CSettingsStore* m_pSettingsStore;
};


#endif /* #ifndef __DasherInterfaceBase_h__ */
