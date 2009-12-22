
#include "../Common/Common.h"

#include "Trainer.h"
#include "DasherInterfaceBase.h"
#include "LanguageModelling/PPMPYLanguageModel.h"

using namespace Dasher;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

CTrainer::CTrainer(CLanguageModel *pLanguageModel, CAlphabet *pAlphabet)
  : CTrainingHelper(pAlphabet), m_pLanguageModel(pLanguageModel) {
}

void CTrainer::Train(CAlphabet::SymbolStream &syms) {
  CLanguageModel::Context sContext = m_pLanguageModel->CreateEmptyContext();

  for(symbol sym; (sym=syms.next())!=-1;) {
      m_pLanguageModel->LearnSymbol(sContext, sym);
  }
  m_pLanguageModel->ReleaseContext(sContext);
}

CMandarinTrainer::CMandarinTrainer(CLanguageModel *pLanguageModel, CAlphabet *pAlphabet, CAlphabet *pCHAlphabet)
: CTrainer(pLanguageModel, pAlphabet), m_pCHAlphabet(pCHAlphabet) {
}

//TrainMandarin is used to train Mandarin Dasher: PPMPYLanguageModel
//Mandarin training is distinct from normal PPM training in that it uses two separate alphabets, and trains with py-character pairs. Despite so, implementation here may seem out of structure, and it could be necessary to revise later, particularly on robustness to deal with non-unicode chars
//The training of Mandarin Dasher may evolve in to possible paths: 1.Include punctuation (more work); 2.User defined training files (not sure how); 3.Learning as one types (more work)
//As Manager is produced, training happens in AlphabetManagerFactory

void CMandarinTrainer::LoadFile(const std::string &strPath) {
  //TrainMandarin takes in the Super Pin Yin Alphabet, and uses the Mandarin Character alphabet stored in private AlphabetManagerFactory
  FILE * fpTrain = fopen(strPath.c_str(), "rb");
  
  if(!fpTrain) {
    std::cout << "Mandarin Training File: cannot open file or incorrect directory" << std::endl;
    return;
  }
  unsigned numberofchar = 0;


  const size_t charsize = 1024;
  const size_t trainBufferSize = 3*charsize*3;
  char szBuffer[trainBufferSize];
    
  std::string strChar;
  std::string strPY;
  CLanguageModel::Context trainContext = m_pLanguageModel->CreateEmptyContext();
  std::string pyID = "》";
  std::vector<symbol> Symchar;
  std::vector<symbol> Sympy;

  while(!feof(fpTrain)){
    
    strPY.clear();
    strChar.clear();
 
    size_t iNumBytes = fread(szBuffer, 1, trainBufferSize, fpTrain);
    std::string strBuffer = std::string(szBuffer, iNumBytes);

    size_t lim;
    if(iNumBytes<9*charsize)
      lim = iNumBytes/9;
    else
      lim = charsize;
    
    size_t pos =0;//position in 3's counting on 
    while(pos<lim*3){

      while(pyID.compare(strBuffer.substr(3*pos,3))!=0)
	pos++;
      
      pos++;
      //      strBuffer.copy(ctemp,3,3*pos);
      
      strPY.append(strBuffer.substr(3*pos,3));
 
      pos++;
 
      //strBuffer.copy(ctemp,3,3*pos);
      strChar.append(strBuffer.substr(3*pos,3));
      std::string strtemp = strBuffer.substr(3*(pos),3);
      Symchar.clear();
      m_pCHAlphabet->GetSymbols(Symchar, strtemp);

      pos++;
          
    }
    Symchar.clear();
    Sympy.clear();
    m_pCHAlphabet->GetSymbols(Symchar, strChar);
    m_pAlphabet->GetSymbols(Sympy, strPY);      
    
    for(unsigned int i =0; i<Symchar.size(); i++){

      if((Symchar[i]<7603)&&(Symchar[i]>-1)){//Hack here? to prevent lan model from failing
	
	static_cast<CPPMPYLanguageModel *>(m_pLanguageModel)->LearnPYSymbol(trainContext, Sympy[i]); 
	m_pLanguageModel->LearnSymbol(trainContext, Symchar[i]);
	
      }

      // if(Sym.size()>0)
      
      numberofchar = numberofchar + Symchar.size();     
    }       
    
  }
  //std::cout<<"The Length of Training file is  "<<numberofchar<<" bytes/py characters"<<std::endl;  
}
