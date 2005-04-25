// SymbolAlphabet.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __LanguageModelling_SymbolAlphabet_h__
#define __LanguageModelling_SymbolAlphabet_h__

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

namespace Dasher
{
	
	class CSymbolAlphabet
	{
	public:

		/////////////////////////////////////////////////////////////////////////////
		// iSize is the number of Symbols - please dont change the constructor
		// Add functions to set customizable behaviour

		CSymbolAlphabet::CSymbolAlphabet(int iSize)
			: m_iSize(iSize)
		{
			DASHER_ASSERT(iSize>0);
		}
		
		int GetSize() const
		{
			return m_iSize;
		}

	private:
		int m_iSize;
	};

}

#endif // ndef __LanguageModelling_SymbolAlphabet_h__ 

