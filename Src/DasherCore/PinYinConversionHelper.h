#ifndef __PINYIN_CONVERSION_HELPER_H__
#define __PINYIN_CONVERSION_HELPER_H__

#include "Alphabet/Alphabet.h"
#include "Alphabet/AlphIO.h"
#include "LanguageModelling/LanguageModel.h"
#include "ConversionHelper.h"
#include "DasherNode.h"

//both of these start from 0
typedef int HZIDX; 
typedef int CANDIDX; 

/// \ingroup Model
/// @{
class CPinYinConversionHelper : public CConversionHelper {
 public:

  CPinYinConversionHelper(Dasher::CEventHandler *pEventHandler,  CSettingsStore *pSettingsStore, Dasher::CAlphIO *pAlphIO);
  
  virtual bool Convert(const std::string &strSource, SCENode ** pRoot, int * HZCount, int CMid);

  virtual void AssignSizes(SCENode * pStart, Dasher::CLanguageModel::Context context , long normalization, int uniform, int iNChildren);

  virtual Dasher::CLanguageModel *  GetLanguageModel(){
    return m_pLanguageModel;
  }

  virtual bool GetPhraseList(int HZIndex, SCENode ** psOutput, int CMid);

  virtual void BuildDataBase();
  virtual void ClearData(int CMid);

  virtual std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > > * GetDP(int CMid);// get data pointer

  void ProcessPhrase(HZIDX HZIndex);
  int CalculateScore(CDasherNode * pNode, CANDIDX CandIndex);
    CANDIDX HZLookup(HZIDX HZIndex, const std::string &strSource);//finds the index of a HZ candidate

 private:
     
  void TrainChPPM();
  void ProcessFile(int index);

  CLanguageModel *m_pLanguageModel;     // pointer to the language model
  CAlphabet *m_pAlphabet;        // pointer to the alphabet


  //CONTEXT DATA
  std::vector<std::vector<std::vector<std::vector<std::vector<std::vector<int> > > > > >vContextData;  
//THE DATA IS CONSTRUCTED OF UNITS OF SINGLE CELLS STORING CONTEXT SEQUENCE
  //AND A CORRESPONDING SCORE 

  //LEVEL 1 : HZ INDEX : NUMBER OF CHARACTERS CONVERTED
  //LEVEL 2 : CAND INDEX : NUMBER OF CANDIDATES WITH EACH POSITION
  //LEVEL 3 : SUB HZ INDEX :(COULD BE REDUNDANT) CORRESPONDES TO
  //          INDEX OF CHARACTERS WHICH WERE CONSISTED IN PHRASES
  //LEVEL 4 : SUB CAND INDEX : WHICH CAND WAS IN THE PHRASE 
  //LEVLE 5 : CELL: STORING PHASES IN HZ INDEX AND ASSIGNED SCORE
  //          IN THE WAY: 1.SCORE 2.Z 3.Y 4.X FOR PHRASE XYZ
  //          PREVIOUSLY PROCESSED
		 
  int m_iCMID;
  int m_iHZCount;
  
  bool m_bTraceNeeded;
  bool m_bPhrasesProcessed[MAX_HZ_NUM-1]; // flags to signal whether
  // phrases are processed
  // at a particular Chinese
  // HZ index position



  std::vector<int> vTrace; //used to store the last input string of
                           //Chinese HZ characters found in
                           //CalculateScore


};
/// @}
#endif
