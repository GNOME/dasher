// DasherTypes.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __DasherTypes_h__
#define __DasherTypes_h__

namespace Dasher
{
	/* TODO: note by IAM 08/2002 {{{
	
	MS docs tell us the __int64 type has no ANSI equivalent
	I've checked and currently a 32bit long leads to problems.
	The code could probably be altered to get around this 64bit
	precision requirement. If not a custom class could be found
	(or implemented fairly easily). However, as GCC supports
	"long long int" I'm giving this low priority until someone
	complains...
	
	"ISO C99 supports data types for integers that are at least 64 bits wide,
	and as an extension GCC supports them in C89 mode and in C++."
	
	I've heard some compilers have a "quad int". If "long long int" does not
	work, try that.
	}}} */
	#ifdef _MSC_VER
		typedef __int64 myint;
                #define LLONG_MAX 9223372036854775807
	        #define LLONG_MIN (-LLONG_MAX - 1)
	#else
		typedef long long int myint;
	        #define LLONG_MAX 9223372036854775807LL
	        #define LLONG_MIN (-LLONG_MAX - 1LL)
	#endif
	
	// Using a signed symbol type allows "Out of band" ie negative {{{
	// values to be used to flag non-symbol data. For example commands
	// in dasher nodes.
	//typedef unsigned int symbol; // }}}
	typedef int symbol;
	
//	typedef unsigned int uint;
	//	typedef unsigned short ushort;
	
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

}

#endif /* #ifndef __DasherTypes_h__ */
