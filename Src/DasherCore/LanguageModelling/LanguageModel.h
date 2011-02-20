// LanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __LanguageModelling_LanguageModel_h__
#define __LanguageModelling_LanguageModel_h__

#include "../DasherTypes.h"


#include <vector>

/////////////////////////////////////////////////////////////////////////////

namespace Dasher {
  class CLanguageModel;
}

///
/// \defgroup LM Language modelling
/// @{

///
/// \brief Language model base class
/// Base class for all language model components
///

class Dasher::CLanguageModel
{
public:

  /////////////////////////////////////////////////////////////////////////////

  CLanguageModel(int iNumSyms) : m_iNumSyms(iNumSyms) {};

  virtual ~CLanguageModel() {};
  
  /// 
  /// Index of registered context 
  ///

  typedef size_t Context;

  ///
  /// Representation of an invalid context
  ///

  static const size_t nullContext = 0;

  ///
  /// @name Context manipulation
  /// Functions for creating, destroying and altering contexts
  /// @{

  ///
  /// Create an empty context
  ///

  virtual Context CreateEmptyContext() = 0;

  ///
  /// Create a copy of an existing context
  ///

  virtual Context CloneContext(Context Context) = 0;

  ///
  /// Free resources associated with a context
  ///

  virtual void ReleaseContext(Context Context) = 0;

  ///
  /// Update context with a character - only modifies context
  ///

  virtual void EnterSymbol(Context context, int Symbol) = 0;

  ///
  /// Add character to the language model at the current context and update the context 
  /// - modifies both the context and the LanguageModel
  ///

  virtual void LearnSymbol(Context context, int Symbol) = 0;

  /// @}

  /// @name Prediction
  /// Determination of probabilities in a given context
  /// @{
  
  ///
  /// Get symbol probability distribution
  /// 

  virtual void GetProbs(Context Context, std::vector < unsigned int >&Probs, int iNorm, int iUniform) const = 0;

  /// @}

  /// @name Persistant storage
  /// Binary representation of language model state
  /// @{

  virtual bool WriteToFile(std::string strFilename) {
    return false;
  };

  virtual bool ReadFromFile(std::string strFilename) {
    return false;
  };

  /// @}

  ///
  /// Get the maximum useful context length for this language model

  virtual int GetContextLength() const {
    // TODO: Fix hard coded value
    return 5;
  };

 protected:
  struct SLMFileHeader {
    // Magic number ("%DLF" in ASCII)
    char szMagic[4];
    // Version of the header
    unsigned short int iHeaderVersion;
    // Total size of header (including variable length alphabet name)
    unsigned short int iHeaderSize;
    // ID of the language model
    unsigned short int iLMID;
    // Version number of the language model
    unsigned short int iLMVersion;
    // Minimum compatible version for the language model
    unsigned short int iLMMinVersion;
    // Number of characters in the alphabet
    unsigned short int iAlphabetSize;
    // UTF-8 encoded alphabet name follows (variable length struct)
  };

  ///Return the number of symbols over which we are making predictions, plus one
  /// (to leave space for an initial 0).
  int GetSize() const {
    return m_iNumSyms+1;
  }

  const int m_iNumSyms;

};

/// @}

/////////////////////////////////////////////////////////////////////////////

#endif // ndef __LanguageModelling_LanguageModel_h__
