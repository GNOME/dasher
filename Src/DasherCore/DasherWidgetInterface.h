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
	virtual void Start()=0;
	
	// Times in milliseconds are required so Dasher runs at the correct speed.
	virtual void TapOn(int MouseX, int MouseY, unsigned long Time)=0;
	virtual void PauseAt(int MouseX, int MouseY)=0;
	virtual void Unpause(unsigned long Time)=0;
	virtual void Redraw()=0;
	
	// The widgets need to tell the engine when they have been affected
	// by external interaction
	virtual void ChangeScreen()=0;
	virtual void ChangeEdit()=0;
	
	// These are needed so widgets know how to render the alphabet.
	// All strings are encoded in UTF-8.
	virtual unsigned int GetNumberSymbols()=0;
	virtual const std::string& GetDisplayText(Dasher::symbol Symbol)=0;
	virtual const std::string& GetEditText(Dasher::symbol Symbol)=0;
	virtual Opts::ScreenOrientations GetAlphabetOrientation()=0;
	virtual Opts::AlphabetTypes GetAlphabetType()=0;
	virtual const std::string& GetTrainFile()=0; // Returns a fully-qualified path to file:
	                                             // UserLocation + TrainFile
};

#endif /* #ifndef __DasherWidgetInterface_h__ */