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


namespace Dasher {class CDasherAppInterface;}
class Dasher::CDasherAppInterface
{
public:
	// We may change the widgets Dasher uses at run time
	virtual void ChangeScreen(CDasherScreen* NewScreen)=0;
	virtual void ChangeEdit(CDashEditbox* NewEdit)=0;
	
	// Training by string segments or file all training data must be in UTF-8.
	virtual void Train(std::string* TrainString, bool IsMore)=0;
	virtual void TrainFile(std::string Filename)=0;
	
	// App may want to display characters per minute or frames per second.
	virtual double GetCurCPM()=0;
	virtual double GetCurFPS()=0;

	// Customize alphabet
	virtual void GetAlphabets(std::vector< std::string >* AlphabetList)=0;
	virtual const CAlphIO::AlphInfo& GetInfo(const std::string& AlphID)=0;
	virtual void SetInfo(const CAlphIO::AlphInfo& NewInfo)=0;
	virtual void DeleteAlphabet(const std::string& AlphID)=0;
};


#endif /* #ifndef __DasherAppInterface_h__ */
