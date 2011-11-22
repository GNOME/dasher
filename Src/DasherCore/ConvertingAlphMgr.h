/*
 *  ConvertingAlphMgr.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 06/08/2010.
 *  Copyright 2010 Cavendish Laboratory. All rights reserved.
 *
 */

#ifndef __CONVERTING_ALPH_MGR_H__
#define __CONVERTING_ALPH_MGR_H__

#include "AlphabetManager.h"
#include "ConversionManager.h"

namespace Dasher {
  //TODO Need to override CreateLanguageModel to use something appropriate for conversion.
  // The created model, needs to have a GetSize() _including_ the conversion node
  // (as this is not included in the Alphabet's GetNumberTextSymbols).
  //TODO in fact IterateChildGroups will not include the conversion symbol (expected by
  // CreateSymbolNode below) either, as it stops at Alphabet GetNumberTextSymbols too...
  //TODO do we also need to override GetProbs? Existing impl will add uniformity onto the conversion root too.
  class CConvertingAlphMgr : public CAlphabetManager {
  public:
    CConvertingAlphMgr(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CConversionManager *pConvMgr, const CAlphInfo *pAlphabet);
    ///Override to also tell the ConversionManager that the screen has changed.
    void MakeLabels(CDasherScreen *pScreen);
    virtual ~CConvertingAlphMgr();
  protected:
    ///Override to return a conversion root for iSymbol==(one beyond last alphabet symbol)
    virtual CDasherNode *CreateSymbolNode(CAlphNode *pParent, symbol iSymbol);
  private:
    CConversionManager *m_pConvMgr;
    
    //TODO do we need to override
    //void GetProbs(vector<unsigned int> *pProbInfo, CLanguageModel::Context context);
    //to do something for the conversion symbol, or does the LM do that for us???
  };
}

#endif
