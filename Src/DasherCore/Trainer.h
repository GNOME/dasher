#ifndef __trainer_h__
#define __trainer_h__

#include "LanguageModelling/PPMPYLanguageModel.h"
#include "Alphabet/AlphInfo.h"
#include "AbstractXMLParser.h"

namespace Dasher {
  class CTrainer : public AbstractParser {
            
  public:
    CTrainer(CMessageDisplay *pMsgs, CLanguageModel *pLanguageModel, const CAlphInfo *pInfo, const CAlphabetMap *pAlphabet);
    virtual ~CTrainer();
    class ProgressIndicator {
    public:
      virtual void bytesRead(off_t)=0;
    };
    
    void SetProgressIndicator(ProgressIndicator *pProg) {m_pProg = pProg;}

    ///Parses a text file; bUser ignored.
    bool Parse(const std::string &strDesc, std::istream &in, bool bUser);
  
  protected:

    virtual void Train(CAlphabetMap::SymbolStream &syms);
    
    ///Try to read a context-switch escape sequence from the symbolstream.
    /// \param sContext context to be reinitialized if a context-switch command is found
    /// \param sym symbol that was most recently read from stream, i.e. that
    /// might be the start of the context-switch command
    /// \syms symbolstream to read body of command from, if necessary; should be positioned
    /// just after the first occurrence of the escape character.
    /// \return true if a context-switch command was found (=> sContext reinitialized);
    ///  false if the character was not a context-switch; specifically, if there was a
    ///  double context-switch character, meaning that an actual occurrence of that
    ///  character is desired to be fed into the LanguageModel, this method returns false
    ///  with the stream positioned just after the second ctx-switch character
    ///  (ready to continue reading as per normal)
    bool readEscape(CLanguageModel::Context &sContext, symbol sym, CAlphabetMap::SymbolStream &syms);

    ///Returns the description of the file as passed to Parse()
    /// (usually a filename)
    const std::string &GetDesc() {return m_strDesc;}
    const CAlphabetMap * const m_pAlphabet;
    CLanguageModel * const m_pLanguageModel;
    const CAlphInfo * const m_pInfo;
    // symbol number in alphabet of the context-switch character (maybe 0 if not in alphabet!)
    int m_iCtxEsc;
  private:
    ProgressIndicator *m_pProg;
    std::string m_strDesc;
  };

}

#endif
