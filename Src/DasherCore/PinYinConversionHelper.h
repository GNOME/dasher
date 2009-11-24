#ifndef __PINYIN_CONVERSION_HELPER_H__
#define __PINYIN_CONVERSION_HELPER_H__

#include "Alphabet/AlphIO.h"
#include "Alphabet/Alphabet.h"
#include "ConversionHelper.h"
#include "DasherNode.h"
#include "LanguageModelling/LanguageModel.h"
#include "LanguageModelling/PPMPYLanguageModel.h"
#include "PinyinParser.h"

#include <string>

//both of these start from 0

namespace Dasher {

class CPinYinConversionHelper : public CConversionHelper {
  friend class CMandarinAlphMgr;
 public:

  CPinYinConversionHelper(CNodeCreationManager *pNCManager, Dasher::CEventHandler *pEventHandler,  CSettingsStore *pSettingsStore, Dasher::CAlphIO *pAlphIO, const std::string strCHAlphabetPath, Dasher::CAlphabet * pAlphabet, CPPMPYLanguageModel *pLanguageModel);

  virtual void BuildTree(CConvHNode *pRoot);
  
  virtual bool Convert(const std::string &strSource, SCENode ** pRoot);

  virtual void AssignSizes(const std::vector<SCENode *> &vChildren, Dasher::CLanguageModel::Context context , long normalization, int uniform);

  virtual void GetProbs(Dasher::CLanguageModel::Context context, std::vector < unsigned int >&Probs, int norm);

  virtual unsigned int GetSumPYProbs(Dasher::CLanguageModel::Context context, std::vector <SCENode *> &pPYCandStart,int norm);

protected:
  class CPYConvNode : public CConvHNode {
  public:
    CPYConvNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo, CPinYinConversionHelper *pMgr);
    //override to blank out learn-as-write for Mandarin Dasher
    virtual void SetFlag(int iFlag, bool bValue);
    
    ///Bit of a hack here - these two need to be accessed by CMandarinAlphMgr :-(.
    ///However, by making MandarinAlphMgr a friend of PinYinConvHelper, it can see this nested class...
    virtual CLanguageModel::Context GetConvContext();
    virtual void SetConvSymbol(int iSymbol);
    
  protected:
    inline CPinYinConversionHelper *mgr() {return static_cast<CPinYinConversionHelper *>(m_pMgr);}
  };
  CPYConvNode *makeNode(CDasherNode *pParent, int iLbnd, int iHbnd, CDasherNode::SDisplayInfo *pDispInfo);
	
  virtual CPPMPYLanguageModel *GetLanguageModel() {
    return static_cast<CPPMPYLanguageModel *>(CConversionHelper::GetLanguageModel());
  }
 private:
  void TrainChPPM(CSettingsStore *pSettingsStore);
  void ProcessFile(CSettingsStore *pSettingsStore, int index);

  CAlphabet *m_pCHAlphabet;        // pointer to the Chinese Character alphabet
  CAlphabet *m_pPYAlphabet;        // pointer to the Grouped Super Pin Yin alphabet 
  CPinyinParser *pParser;

  int m_iPriorityScale;

};

}

#endif
