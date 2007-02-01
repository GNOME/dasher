// LanguageModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __LanguageModelling_LanguageModel_h__
#define __LanguageModelling_LanguageModel_h__

#include "SymbolAlphabet.h"
#include "../DasherComponent.h"
#include <vector>

/////////////////////////////////////////////////////////////////////////////

namespace Dasher {
  class CLanguageModel;
}

///
/// \brief Language model base class
/// Base class for all language model components
///

class Dasher::CLanguageModel:public Dasher::CDasherComponent
{
public:

  /////////////////////////////////////////////////////////////////////////////

  CLanguageModel(Dasher::CEventHandler *pEventHandler, CSettingsStore *pSettingsStore, const CSymbolAlphabet &Alphabet);

  virtual ~CLanguageModel() {};

  virtual void HandleEvent(Dasher::CEvent * pEvent);
  
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
  /// Functions for creating, destroying and alteringn contexts
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

  virtual void GetProbs(Context Context, std::vector < unsigned int >&Probs, int iNorm) const = 0;

  /// @}

  /// @name Status reporting
  /// Return information usful for debgging purposes
  /// @{

  /// Get some measure of the memory usage for diagnostic
  /// purposes. No need to implement this if you're not comparing
  /// language models. The exact meaning of the result will
  /// depend on the implementation (for example, could be the
  /// number of nodes in a trie, or the physical memory usage).

  virtual int GetMemory() = 0;

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

  /// @name Internal member access
  /// Access to internal member classes. This is dangerous and should
  /// be considered obsolete.
  /// @{

  const CSymbolAlphabet &SymbolAlphabet() const {
    return m_Alphabet;
  };

  /// @}

  ///
  /// Get the maximum useful context length for this language model

  virtual int GetContextLength() const {
    // TODO: Fix hard coded value
    return 5;
  };

 protected:
  int GetSize() const {
    return m_Alphabet.GetSize();
  }


 private:

  const CSymbolAlphabet m_Alphabet;

};

/////////////////////////////////////////////////////////////////////////////

#endif // ndef __LanguageModelling_LanguageModel_h__
