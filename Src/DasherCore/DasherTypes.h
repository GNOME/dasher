// DasherTypes.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __DasherTypes_h__
#define __DasherTypes_h__

//#include "../Common/Types/int.h"
#include "../Common/Types/int32.h"
#include "../Common/Types/int64.h"
#include <string>

// Definition for hash_map, platform dependent, not part of STL
#ifdef DASHER_WIN32 
#include <hash_map>
#define HASH_MAP stdext::hash_map
#else
#include <ext/hash_map>
#define HASH_MAP __gnu_cxx::hash_map
#endif

namespace Dasher
{
	// DasherModel co-ordinates are of type myint
	typedef Cint64 myint;
	
	// All screen co-ordinates are of type screenint
	typedef Cint32 screenint;




	// Using a signed symbol type allows "Out of band" ie negative {{{
	// values to be used to flag non-symbol data. For example commands
	// in dasher nodes.
	//typedef unsigned int symbol; // }}}
	typedef int symbol;


	namespace Opts
	{
		// Numbers should be applied to elements of the following two enumerations as these preferences may be stored to file. Constancy between
		// versions is a good idea. It should *not* be assumed that the numbers map onto anything useful. Different codepages may be appropriate on different systems for different character sets.
		enum FileEncodingFormats {UserDefault=-1, AlphabetDefault=-2, UTF8=65001, UTF16LE=1200, UTF16BE=1201};
		enum AlphabetTypes {MyNone=0, Arabic=1256, Baltic=1257, CentralEurope=1250, ChineseSimplified=936, ChineseTraditional=950, Cyrillic=1251, Greek=1253, Hebrew=1255, Japanese=932, Korean=949, Thai=874, Turkish=1254, VietNam=1258, Western=1252};
		enum ScreenOrientations {Alphabet=-2, LeftToRight=0, RightToLeft=1, TopToBottom=2, BottomToTop=3};
		// TODO: Possibly make colors long and add an RGB item to this {{{
		// Would allow literal as well as semantic colors for
		// greater flexibility. }}}
		enum ColorSchemes {Nodes1=0, Nodes2=1, Special1=2, Special2=3, Groups=4, Objects=5};
		enum FontSize {Normal=1, Big=2, VBig=4};
	}

	struct ControlTree {
	  void* pointer;
	  int data;
	  int type;
	  int colour;
	  std::string text;
	  ControlTree *parent;
	  ControlTree *children;
	  ControlTree *next;
          ControlTree() : next(NULL), children(NULL), 
            parent(NULL), pointer(NULL), 
            data(0), type(0), colour(0), text("") {};
	};
}

#endif /* #ifndef __DasherTypes_h__ */
