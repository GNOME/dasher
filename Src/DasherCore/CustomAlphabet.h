// CustomAlphabet.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __CustomAlphabet_h__
#define __CustomAlphabet_h__


#include "Alphabet.h"
#include "AlphIO.h"


namespace Dasher {class CCustomAlphabet;}
class Dasher::CCustomAlphabet : public Dasher::CAlphabet
{
public:
	CCustomAlphabet(const CAlphIO::AlphInfo& AlphInfo);
};


#endif /* #ifndef __CustomAlphabet_h__ */
