// DasherWidgetInterface.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DasherWidgetInterface_h__
#define __DasherWidgetInterface_h__


#include "DasherTypes.h"
#include <string>

namespace Dasher {class CDasherWidgetInterface;}
class Dasher::CDasherWidgetInterface
{
public:
        //! Signal to the core that the model should be started
        //
        //! Call this function before providing input coordinates and whenever
        //! reinitialising Dasher
	virtual void Start()=0;
	
	// Times in milliseconds are required so Dasher runs at the correct speed.
	//! Signal an input location
	//
	//! Signal an input event. This may be the current location of the 
	//! mouse or a stylus tap, for example
	//! \param MouseX The mouse X coordinate, in screen coordinates
	//! \param MouseY The mouse Y coordinate, in screen coordinates
	//! \param Time Time in milliseconds, required to keep Dasher running
	//! at the correct speed
	virtual void TapOn(int MouseX, int MouseY, unsigned long Time)=0;
	
	//! Signal an input event and pause the simulation
	virtual void PauseAt(int MouseX, int MouseY)=0;
	
	//! Unpause the simulation.
	//
	//! \param Time should be in milliscones and should be consistent with
	//! previous time values
	virtual void Unpause(unsigned long Time)=0;

	//! Signal that the core should redraw the screen
	virtual void Redraw()=0;
	
	// The widgets need to tell the engine when they have been affected
	// by external interaction
	
	//! Signal the core that a change has occured to the screen. (Unneeded)
	virtual void ChangeScreen()=0;

	//! Signal the core that a change has occued to the editbox. (Unneeded)
	virtual void ChangeEdit()=0;
	
	// These are needed so widgets know how to render the alphabet.
	// All strings are encoded in UTF-8.

	//! Request the number of symbols in the current alphabet
	virtual unsigned int GetNumberSymbols()=0;
	
	//! Request the text that should be displayed on the Dasher canvas
	//
	//! Note - the returned string is in UTF-8 encoding.
	//! \param Symbol the symbol that is to be displayed
	virtual const std::string& GetDisplayText(Dasher::symbol Symbol)=0;

	//! Request the text that should be entered into the edit box
	//
	//! Note - the returned string is in UTF-8 encoding.
	//! \param Symbol the symbol that is to be displayed
	virtual const std::string& GetEditText(Dasher::symbol Symbol)=0;

	//! Request the foreground colour for the text to be drawn on the canvas
	//! \param Symbol the symbol that is to be displayed
	virtual int GetTextColour(Dasher::symbol Symbol)=0;

	//! Request the default screen orientation for the current alphabet
	//
	//! (Eg, left to right for English, right to left for Hebrew)
	virtual Opts::ScreenOrientations GetAlphabetOrientation()=0;

	//! Returns the codepage for the current alphabet
	virtual Opts::AlphabetTypes GetAlphabetType()=0;

	//! Provides a fully qualified path to the training file for the alphabet
	virtual const std::string& GetTrainFile()=0; // Returns a fully-qualified path to file:
	                                             // UserLocation + TrainFile
};

#endif /* #ifndef __DasherWidgetInterface_h__ */
