// DasherTypes.h
//
// Copyright (c) 2001-2002 David Ward

#ifndef __DasherTypes_h__
#define __DasherTypes_h__

// We use our own version of hungarian notation to indicate
// the type of variables:
//
//    i       - integer and enumerated types 
//    c       - char
//    str     - STL string
//    sz      - char* string
//    b       - boolean
//    p       - pointer (to a primative type or to an object)
//    pp      - pointer to a pointer (and so on)
//    v       - STL vector
//    map     - STL map
//    d       - float or double
//    s       - structure 
//    o       - object
//    h       - HANDLE type in Windows
//
// Class member variables and global variables should 
// have the additional prefixes:
//
//    m_      - member variables 
//    g_      - global variablse
//    s_      - static member variables
//
// Variables names (local and member) should capitalize each 
// new word and don't use underscores (except as above).
//

#include "../Common/Types/int.h"
#include "../Common/Types/int32.h"
#include "../Common/Types/int64.h"
#include <string>
#include <vector>

// Definition for hash_map, platform dependent, not part of STL

// Commented out by pjc - I don't think we use this any more
// - Okay - maybe we do need this after all

/* #ifdef DASHER_WIN32 */
/* #include <hash_map> */
/* #define HASH_MAP stdext::hash_map */
/* #else */
/* #include <ext/hash_map> */
/* #define HASH_MAP __gnu_cxx::hash_map */
/* #endif */

namespace Dasher {
  // DasherModel co-ordinates are of type myint
  typedef Cint64 myint;
  typedef Cint64 dasherint;

  // All screen co-ordinates are of type screenint
  typedef Cint32 screenint;

  // Using a signed symbol type allows "Out of band" ie negative {{{
  // values to be used to flag non-symbol data. For example commands
  // in dasher nodes.
  //typedef unsigned int symbol; // }}}
  typedef int symbol;

  namespace Opts {
    // Numbers should be applied to elements of the following two enumerations as these preferences may be stored to file. Constancy between
    // versions is a good idea. It should *not* be assumed that the numbers map onto anything useful. Different codepages may be appropriate on different systems for different character sets.
    enum FileEncodingFormats { UserDefault = -1, AlphabetDefault = -2, UTF8 = 65001, UTF16LE = 1200, UTF16BE = 1201 };
    enum AlphabetTypes { MyNone = 0, Arabic = 1256, Baltic = 1257, CentralEurope = 1250, ChineseSimplified = 936, ChineseTraditional = 950, Cyrillic = 1251, Greek = 1253, Hebrew = 1255, Japanese = 932, Korean = 949, Thai = 874, Turkish = 1254, VietNam = 1258, Western = 1252 };
    enum ScreenOrientations { Alphabet = -2, LeftToRight = 0, RightToLeft = 1, TopToBottom = 2, BottomToTop = 3 };
    // TODO: Possibly make colors long and add an RGB item to this {{{
    // Would allow literal as well as semantic colors for
    // greater flexibility. }}}
    enum ColorSchemes { Nodes1 = 0, Nodes2 = 1, Special1 = 2, Special2 = 3, Groups = 4, Objects = 5 };
    enum FontSize { Normal = 1, Big = 2, VBig = 4 };
  }
  struct ControlTree {
    void *pointer;
    int data;
    int type;
    int colour;
    std::string text;
    ControlTree *parent;
    ControlTree *children;
    ControlTree *next;
    ControlTree():pointer(NULL), data(0), type(0), colour(0), text(""), parent(NULL), children(NULL), next(NULL) {
    };
  };

  // Types added so model can report back what it has done for
  // user logging purposes.
  struct SymbolProb
  {
    symbol          sym;
    double          prob;
  };

  // Keeps track of the display text and prob of a past symbol.
  // We need to store this as the alphabet could change in the
  // middle of a trial and we won't be able to convert symbols
  // to text once it does.
  struct SymbolProbDisplay
  {
    symbol          sym;
    std::string     strDisplay;
    double          prob;
  };

  typedef std::vector<SymbolProb>         VECTOR_SYMBOL_PROB;
  typedef std::vector<SymbolProbDisplay>  VECTOR_SYMBOL_PROB_DISPLAY;

}

#endif /* #ifndef __DasherTypes_h__ */
