// DasherSettingsInterface.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __DasherSettingsInterface_h__
#define __DasherSettingsInterface_h__

#include "DasherTypes.h"
#include "SettingsStore.h"

namespace Dasher {class CDasherSettingsInterface;}
class Dasher::CDasherSettingsInterface
{
public:
	void SettingsDefaults(CSettingsStore* Store);
	
	// These actually affect the way Dasher works
	//! Change the alphabet in use to NewAlphabetID
	virtual void ChangeAlphabet(const std::string& NewAlphabetID) {};

	//! Change the maximum bitrate (effectively the speed) of Dasher
	virtual void ChangeMaxBitRate(double NewMaxBitRate) {};

	//! Generate a new langage model. Not usually needed
	virtual void ChangeLanguageModel(unsigned int NewLanguageModelID) {};

	//! Generate a new view of the model. Call it with 0 when starting up
	virtual void ChangeView(unsigned int NewViewID) {};

	//! Change the orientation (l->r, r->l, so on) of the model
	virtual void ChangeOrientation(Opts::ScreenOrientations Orientation) {};

	//! Set the file encoding of output files to Encoding
	virtual void SetFileEncoding(Opts::FileEncodingFormats Encoding) {};

	//! Inform the core that the screen has this size
	virtual void SetScreenSize(long Width, long Height) {};
	
	//! Set the size of the font used in the Dasher canvas
	virtual void SetDasherFontSize(Dasher::Opts::FontSize fontsize) {};

	//! Set the number of dimensions of input (either 1 or 2)
	virtual void SetDasherDimensions(bool Value) {};

	// These are recommended options for the Dasher GUI. {{{ They don't actually
	// change the way Dasher works. They are part of the Dasher interface
	// anyway so that it can handle option saving for you, and to make it
	// easy for the Dasher engine to control the GUI later on. }}}

	//! True if toolbar should be shown, false otherwise
	virtual void ShowToolbar(bool Value) {};

	//! True if toolbar should show text, false otherwse
	virtual void ShowToolbarText(bool Value) {};

	//! True if toolbar should have large icons, false otherwise
	virtual void ShowToolbarLargeIcons(bool Value) {};

	//! True if the speed slider should be shown, false otherwise
	virtual void ShowSpeedSlider(bool Value) {};

	//! True if the window layout should be fixed, false otherwise
	virtual void FixLayout(bool Value) {};

	//! True if new files should be timestamped, false otherwise
	virtual void TimeStampNewFiles(bool Value) {};

	//! True if all text should be copied to clipboard when Dasher is stopped, false otherwise
	virtual void CopyAllOnStop(bool Value) {};

	//! True if a box should be drawn to represent the logical position of the mouse
	virtual void DrawMouse(bool Value) {};

	//! Set the editbox font 
	virtual void SetEditFont(std::string Name, long Size) {};

	//! Set the canvas font
	virtual void SetDasherFont(std::string Name) {};

	//! Set the height of the edit box
	virtual void SetEditHeight(long Value) {};
	
	//! Should Dasher start and stop on space bar?
	virtual void StartOnSpace(bool Value) {};

	//! Should Dasher start and stop on left mouse button?
	virtual void StartOnLeft(bool Value) {};
	
	//! Should Dasher be keyboard controlled?
	virtual void KeyControl(bool Value) {};
	
	//! Should Dasher pause when the pointer leaves the window?
	virtual void WindowPause(bool Value) {};
};


#include <string>

namespace Dasher
{
namespace Keys
{
	// Standard Option strings. You are encouraged to use these constants.
	// -------------------------------------------------------------------
	
	// bool options
	extern const std::string TIME_STAMP;
	extern const std::string SHOW_TOOLBAR;
	extern const std::string SHOW_TOOLBAR_TEXT;
	extern const std::string SHOW_LARGE_ICONS;
	extern const std::string FIX_LAYOUT;
	extern const std::string SHOW_SLIDER;
	extern const std::string COPY_ALL_ON_STOP;
	extern const std::string DRAW_MOUSE;
	extern const std::string START_SPACE;
	extern const std::string START_MOUSE;
	extern const std::string KEY_CONTROL;
	extern const std::string WINDOW_PAUSE;
	// long options
	extern const std::string FILE_ENCODING;
	extern const std::string MAX_BITRATE_TIMES100;
	extern const std::string SCREEN_ORIENTATION;
	extern const std::string VIEW_ID;
	extern const std::string LANGUAGE_MODEL_ID;
	extern const std::string EDIT_FONT_SIZE;
	extern const std::string EDIT_HEIGHT;
	extern const std::string SCREEN_WIDTH;
	extern const std::string SCREEN_HEIGHT;
	extern const std::string DASHER_FONTSIZE;
	extern const std::string DASHER_DIMENSIONS;
	
	// string options
	extern const std::string ALPHABET_ID;
	extern const std::string DASHER_FONT;
	extern const std::string EDIT_FONT;
}
} // namespace Dasher


#endif /* #ifndef __DasherSettingsInterface_h__ */




