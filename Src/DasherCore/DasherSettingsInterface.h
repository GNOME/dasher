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
	virtual void ChangeAlphabet(const std::string& NewAlphabetID) {};
	virtual void ChangeMaxBitRate(double NewMaxBitRate) {};
	virtual void ChangeLanguageModel(unsigned int NewLanguageModelID) {};
	virtual void ChangeView(unsigned int NewViewID) {};
	virtual void ChangeOrientation(Opts::ScreenOrientations Orientation) {};
	virtual void SetFileEncoding(Opts::FileEncodingFormats Encoding) {};
	virtual void SetScreenSize(long Width, long Height) {};
	
	// These are recommended options for the Dasher GUI. {{{ They don't actually
	// change the way Dasher works. They are part of the Dasher interface
	// anyway so that it can handle option saving for you, and to make it
	// easy for the Dasher engine to control the GUI later on. }}}
	virtual void ShowToolbar(bool Value) {};
	virtual void ShowToolbarText(bool Value) {};
	virtual void ShowToolbarLargeIcons(bool Value) {};
	virtual void ShowSpeedSlider(bool Value) {};
	virtual void FixLayout(bool Value) {};
	virtual void TimeStampNewFiles(bool Value) {};
	virtual void CopyAllOnStop(bool Value) {};
	virtual void SetEditFont(std::string Name, long Size) {};
	virtual void SetDasherFont(std::string Name) {};
	virtual void SetEditHeight(long Value) {};
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
	
	// string options
	extern const std::string ALPHABET_ID;
	extern const std::string DASHER_FONT;
	extern const std::string EDIT_FONT;
}
} // namespace Dasher


#endif /* #ifndef __DasherSettingsInterface_h__ */
