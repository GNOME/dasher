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
	~CDasherInterface();

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
	
	// Widget Interface
	// -----------------------------------------------------
	void Start();
	
	void TapOn(int MouseX, int MouseY, unsigned long Time); // Times in milliseconds
	void PauseAt(int MouseX, int MouseY);                   // are required to make
	void Unpause(unsigned long Time);                       // Dasher run at the
	void Redraw();                                          // correct speed.
	
	void ChangeScreen(); // The widgets need to tell the engine when they have been
	void ChangeEdit();   // affected by external interaction
	
	unsigned int GetNumberSymbols();           // These are needed so widgets know
	const std::string& GetDisplayText(symbol Symbol); // how to render the alphabet. All
	const std::string& GetEditText(symbol Symbol);    // strings are encoded in UTF-8
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

	void GetFontSizes(std::vector<int> *FontSizes);
	
	double GetCurCPM(); // App may want to display characters per minute
	double GetCurFPS(); // or frames per second.
	
	// Customize alphabet
	void GetAlphabets(std::vector< std::string >* AlphabetList);
	const CAlphIO::AlphInfo& GetInfo(const std::string& AlphID);
	void SetInfo(const CAlphIO::AlphInfo& NewInfo);
	void DeleteAlphabet(const std::string& AlphID);
	
	// Settings Interface (options saved between sessions)
	// -----------------------------------------------------
	
	void ChangeAlphabet(const std::string& NewAlphabetID);
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
	void StartOnSpace(bool Value);
	void StartOnLeft(bool Value);
	void SetEditFont(std::string Name, long Size);
	void SetDasherFont(std::string Name);
	void SetDasherFontSize(FontSize fontsize);
	void SetDasherDimensions(bool Value);

private:
	CAlphabet* m_Alphabet;
	CLanguageModel* m_LanguageModel;
	CDasherModel* m_DasherModel;
	CDashEditbox* m_DashEditbox;
	CDasherScreen* m_DasherScreen;
	CDasherView* m_DasherView;
	CSettingsStore* m_SettingsStore;
	CDasherSettingsInterface* m_SettingsUI;
	CAlphIO* m_AlphIO;
	
	CLanguageModel::CNodeContext* TrainContext;
	
	std::string AlphabetID;
	int LanguageModelID;
	int ViewID;
	double m_MaxBitRate;
	bool m_CopyAllOnStop;
	bool m_DrawMouse;
	bool m_StartSpace;
	bool m_StartLeft;
	bool m_Dimensions;
	Opts::ScreenOrientations m_Orientation;
	std::string m_UserLocation;
	std::string m_SystemLocation;
	std::string m_TrainFile;
	std::string m_DasherFont;
	std::string m_EditFont;
	int m_EditFontSize;
	static const std::string EmptyString;
	
	void CreateDasherModel();
};


#endif /* #ifndef __DasherInterface_h__ */
