// DasherInterface.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////



#ifndef __DasherInterface_h__
#define __DasherInterface_h__

// TODO - there is a list of things to be configurable in my notes
// Check that everything that is not self-contained within the GUI is covered.

#include "../Common/MSVC_Unannoy.h"
#include "../Common/NoClones.h"

#include "DasherWidgetInterface.h"
#include "DasherAppInterface.h"
#include "DasherSettingsInterface.h"

#include "DasherScreen.h"
#include "Alphabet.h"
#include "AlphIO.h"
#include "CustomColours.h"
#include "ColourIO.h"
#include "LanguageModel.h"
#include "DasherModel.h"
#include "DashEdit.h"
#include "DasherView.h"

#include "../Common/MSVC_Unannoy.h"
#include <map>

namespace Dasher {class CDasherInterface;}
class Dasher::CDasherInterface : private NoClones,
	public CDasherWidgetInterface, public CDasherAppInterface, public CDasherSettingsInterface
{
public:
	CDasherInterface();
	virtual ~CDasherInterface();

	//! Tell the core which CSettingsStore should be used
	void SetSettingsStore(CSettingsStore* SettingsStore);

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

	void DrawMousePos(int MouseX, int MouseY);
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
	const std::string& GetTrainFile();
	
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
	// Settings Interface (options saved between sessions)
	// -----------------------------------------------------
	
	void ChangeAlphabet(const std::string& NewAlphabetID);
	std::string GetCurrentAlphabet();
	void ChangeColours(const std::string& NewColourID);
	std::string GetCurrentColours();
	void ChangeMaxBitRate(double NewMaxBitRate);
	void ChangeLanguageModel(unsigned int NewLanguageModelID);
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
	void MouseposStart(bool Value);
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
    int GetOneButton();
    void SetOneButton(int Value);
    int GetAutoOffset();
    void Render();

private:
	CAlphabet* m_Alphabet;
	CCustomColours* m_Colours;
	CLanguageModel* m_LanguageModel;
	CDasherModel* m_DasherModel;
	CDashEditbox* m_DashEditbox;
	CDasherScreen* m_DasherScreen;
	CDasherView* m_DasherView;
	CSettingsStore* m_SettingsStore;
	CDasherSettingsInterface* m_SettingsUI;
	CAlphIO* m_AlphIO;	
	CAlphIO::AlphInfo m_AlphInfo;
	CColourIO* m_ColourIO;
	CColourIO::ColourInfo m_ColourInfo;
	CLanguageModel::CNodeContext* TrainContext;
	
	std::string AlphabetID;
	std::string ColourID;
	int LanguageModelID;
	int ViewID;
	double m_MaxBitRate;
	bool m_CopyAllOnStop;
	bool m_DrawMouse;
	bool m_DrawMouseLine;
	bool m_DrawKeyboard;
	bool m_StartSpace;
	bool m_StartLeft;
	bool m_KeyControl;
	bool m_Dimensions;
	bool m_Eyetracker;
	bool m_WindowPause;
	bool m_ControlMode;
	bool m_ColourMode;
	bool m_KeyboardMode;
	bool m_MouseposStart;
	bool m_Paused;
	bool m_PaletteChange;
	Opts::ScreenOrientations m_Orientation;
	std::string m_UserLocation;
	std::string m_SystemLocation;
	std::string m_TrainFile;

	std::string m_DasherFont;
	Opts::FontSize m_DasherFontSize;

	std::string m_EditFont;
	std::vector<std::string> m_AlphabetFilenames;
	std::vector<std::string> m_ColourFilenames;
	int m_EditFontSize;
	static const std::string EmptyString;
	
	void CreateDasherModel();
};


#endif /* #ifndef __DasherInterface_h__ */
