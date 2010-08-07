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
  class CConvertingAlphMgr : public CAlphabetManager {
  public:
    CConvertingAlphMgr(CDasherInterfaceBase *pInterface, CNodeCreationManager *pNCManager, CConversionManager *pConvMgr, const CAlphInfo *pAlphabet, const CAlphabetMap *pAlphabetMap, CLanguageModel *pLanguageModel);
    virtual ~CConvertingAlphMgr();
  protected:
    void AddExtras(CAlphNode *pParent, std::vector<unsigned int> *pCProb);
  private:
    CConversionManager *m_pConvMgr;
    
    //TODO do we need to override
    //void GetProbs(vector<unsigned int> *pProbInfo, CLanguageModel::Context context);
    //to do something for the conversion symbol, or does the LM do that for us???
  };
}

#endif
