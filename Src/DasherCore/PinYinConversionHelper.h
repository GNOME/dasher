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
 public:

  CPinYinConversionHelper(CNodeCreationManager *pNCManager, Dasher::CEventHandler *pEventHandler,  CSettingsStore *pSettingsStore, Dasher::CAlphIO *pAlphIO, const std::string strCHAlphabetPath, Dasher::CAlphabet * pAlphabet);

  virtual void BuildTree(CDasherNode *pRoot);
  
  virtual bool Convert(const std::string &strSource, SCENode ** pRoot);

  virtual void AssignSizes(SCENode **pStart, Dasher::CLanguageModel::Context context , long normalization, int uniform, int iNChildren);

  virtual void GetProbs(Dasher::CLanguageModel::Context context, std::vector < unsigned int >&Probs, int norm);

  virtual unsigned int GetSumPYProbs(Dasher::CLanguageModel::Context context, SCENode * pPYCandStart,int norm);

  virtual Dasher::CLanguageModel *  GetLanguageModel(){
    return m_pLanguageModel;
  }

  //override to blank out learn-as-write for Mandarin Dasher
  virtual void SetFlag(CDasherNode *pNode, int iFlag, bool bValue);
	
 private:
     
  void TrainChPPM(CSettingsStore *pSettingsStore);
  void ProcessFile(CSettingsStore *pSettingsStore, int index);

  CPPMPYLanguageModel *m_pLanguageModel;     // pointer to the language model
  CAlphabet *m_pCHAlphabet;        // pointer to the Chinese Character alphabet
  CAlphabet *m_pPYAlphabet;        // pointer to the Grouped Super Pin Yin alphabet 
  CPinyinParser *pParser;

  int m_iPriorityScale;

};

}

#endif
