// DasherAppInterface.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DasherAppInterface_h__
#define __DasherAppInterface_h__

#include "DasherScreen.h"
#include "DashEdit.h"
#include "AlphIO.h"
#include <string>
#include <vector>

namespace Dasher {class CDasherAppInterface;}
class Dasher::CDasherAppInterface
{
public:
	// We may change the widgets Dasher uses at run time

        //! Change the screen being used by the core to NewScreen
	virtual void ChangeScreen(CDasherScreen* NewScreen)=0;

	//! Change the editbox being used by the core to NewEdit
	virtual void ChangeEdit(CDashEditbox* NewEdit)=0;
	
	// Training by string segments or file all training data must be in UTF-8.
	//! Train the core on Trainstring
	//
	//! \param TrainString UTF-8 encoded string to train the core on
	//! \param IsMore True if more text is to follow as part of the same
	//! logical sequence, false otherwise (this allows the model to make
	//! assumptions regarding whether the final character provided is in 
	//! the middle of a word or not)
	virtual void Train(std::string* TrainString, bool IsMore)=0;

	//! Train the core on the file in Filename.
	//
	//! The file should contain UTF-8 encoded text
	virtual void TrainFile(std::string Filename)=0;
	
	// App may want to display characters per minute or frames per second.

	//! Tell the interface how many font sizes there are and what they are

	virtual void GetFontSizes(std::vector<int> *FontSizes)=0;

	//! Provides the number of characters per minute input. (Not implemented)
	virtual double GetCurCPM()=0;
	
	//! Provides the framerate that Dasher is runnin at. (Not implemented)
	virtual double GetCurFPS()=0;

	// Customize alphabet
	//! Get a vector containing a list of alphabet names available to Dasher
	virtual void GetAlphabets(std::vector< std::string >* AlphabetList)=0;
	
	//! Return an AlphInfo object containing the alphabet with name AlphID
	virtual const CAlphIO::AlphInfo& GetInfo(const std::string& AlphID)=0;

	//! Save a new or modified AlphInfo object
	virtual void SetInfo(const CAlphIO::AlphInfo& NewInfo)=0;

	//! Delete the alphabet with name AlphID
	virtual void DeleteAlphabet(const std::string& AlphID)=0;
};


#endif /* #ifndef __DasherAppInterface_h__ */
