// DasherViewSquare.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////


// EnglishAlphabet.h: interface for the EnglishAlphabet class.
// Copyright 2002 David Ward
//////////////////////////////////////////////////////////////////////

#ifndef __EnglishAlphabet_h__
#define __EnglishAlphabet_h__

#include "Alphabet.h"

namespace Dasher {class CEnglishAlphabet;}
class Dasher::CEnglishAlphabet : public Dasher::CAlphabet
{
public:
	CEnglishAlphabet() 
	{
		//StartNewGroup();
		AddChar("a", "a");
		AddChar("b", "b");
		AddChar("c", "c");
		AddChar("d", "d");
		AddChar("e", "e");
/*
		AddChar("f", "f");
		AddChar("g", "g");
		AddChar("h", "h");
		AddChar("i", "i");
		AddChar("j", "j");
		AddChar("k", "k");
		AddChar("l", "l");
		AddChar("m", "m");
		AddChar("n", "n");
		AddChar("o", "o");
		AddChar("p", "p");
		AddChar("q", "q");
		AddChar("r", "r");
		AddChar("s", "s");
		AddChar("t", "t");
		AddChar("u", "u");
		AddChar("v", "v");
		AddChar("w", "w");
		AddChar("x", "x");
		AddChar("y", "y");
		AddChar("z", "z");
*/
		//unsigned char foo[4] = {0xec, 0xa1, 0x8f, 0x0};
		//AddChar("„ÅØ", "„ÅØ");
		StartNewGroup();
		AddChar("A", "A");
		AddChar("B", "B");
		AddChar("C", "C");
		AddChar("D", "D");
		AddChar("E", "E");
/*		AddChar("F", "F");
		AddChar("G", "G");
		AddChar("H", "H");
		AddChar("I", "I");
		AddChar("J", "J");
		AddChar("K", "K");
		AddChar("L", "L");
		AddChar("M", "M");
		AddChar("N", "N");
		AddChar("O", "O");
		AddChar("P", "P");
		AddChar("Q", "Q");
		AddChar("R", "R");
		AddChar("S", "S");
		AddChar("T", "T");
		AddChar("U", "U");
		AddChar("V", "V");
		AddChar("W", "W");
		AddChar("X", "X");
		AddChar("Y", "Y");
		AddChar("Z", "Z");
*/
		StartNewGroup();
		//AddChar("®", "®");
		//AddChar("∏", "∏");
		AddChar("¬®", "¬®");
		AddChar("¬∏", "¬∏");
		StartNewGroup();
		AddChar(" ", "_");
		SetSpaceSymbol();
	};
	~CEnglishAlphabet() {};
	
};

#endif /* #ifndef __EnglishAlphabet_h__ */
