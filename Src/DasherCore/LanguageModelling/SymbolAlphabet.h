// SymbolAlphabet.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __LanguageModelling_SymbolAlphabet_h__
#define __LanguageModelling_SymbolAlphabet_h__

//#include "../Alphabet/Alphabet.h"

/////////////////////////////////////////////////////////////////////////////
//
// CSymbolAlphabet - represents the LanguageModel's alphabet
//
// The alphabet has a size m_iSize, and each symbol is identified by an integer [0,m_iSize-1]
//
// If future LanguageModels require more information about the alphabet, then CSymbolAlphabet 
// could be extented
// - Additions should be made such that no LanguageModel shouldn't have to interpret anything other
// than GetSize()
// - Also, no alphabet should have to set anything other than the size
//
//
// A class hierarchy could be useful if this class gets a lot more complicated
/////////////////////////////////////////////////////////////////////////////

namespace Dasher {

  class CAlphabet;
  /// \ingroup LM
  /// @{

  class CSymbolAlphabet {
  public:

    /////////////////////////////////////////////////////////////////////////////
    // iSize is the number of Symbols - please dont change the constructor
    // Add functions to set customizable behaviour

    CSymbolAlphabet(int iSize)
    : m_iSize(iSize) {
      //                    DASHER_ASSERT(iSize>0);

      SetSpaceSymbol(-1);       // Default (uniformative) value.
    } int GetSize() const {
      return m_iSize;
    } void SetSpaceSymbol(int _SpaceSymbol) {
      // Set the space symbol for the alphabet
      m_SpaceSymbol = _SpaceSymbol;
    } int GetSpaceSymbol() const {
      // Get the space symbol. -1 means that no symbol has been set.
      return m_SpaceSymbol;
    } void SetAlphabetPointer(const CAlphabet * _AlphabetPointer) {

      // NOTE - you are VERY strongly discouraged from
      // using this function. It is only here as a hack
      // until I figure out a better solution.

      AlphabetPointer = _AlphabetPointer;
    }

    const CAlphabet *GetAlphabetPointer() const {

      // NOTE - you are VERY strongly discouraged from
      // using this function. It is only here as a hack
      // until I figure out a better solution.

      return AlphabetPointer;
  } private:
    int m_iSize;
    int m_SpaceSymbol;
    const CAlphabet *AlphabetPointer;
  };

  /// @}

}

#endif // ndef __LanguageModelling_SymbolAlphabet_h__
