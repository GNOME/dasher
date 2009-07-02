
#include "../Common/Common.h"

#include "AlphabetManagerFactory.h"
#include "DasherInterfaceBase.h"
#include "LanguageModelling/PPMLanguageModel.h"
#include "LanguageModelling/WordLanguageModel.h"
#include "LanguageModelling/DictLanguageModel.h"
#include "LanguageModelling/MixtureLanguageModel.h"
#include "LanguageModelling/CTWLanguageModel.h"
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

CAlphabetManagerFactory::CAlphabetManagerFactory(CDasherInterfaceBase *pInterface,
						 CEventHandler *pEventHandler,
						 CSettingsStore *pSettingsStore,
						 Dasher::CAlphIO *pAlphIO,
						 CNodeCreationManager *pNCManager)
{
  // -- put all this in a separate method
  // TODO: Think about having 'prefered' values here, which get
  // retrieved by DasherInterfaceBase and used to set parameters

  // TODO: We might get a different alphabet to the one we asked for -
  // if this is the case then the parameter value should be updated,
  // but not in such a way that it causes everything to be rebuilt.

  Dasher::CAlphIO::AlphInfo oAlphInfo = pAlphIO->GetInfo(pSettingsStore->GetStringParameter(SP_ALPHABET_ID));
  m_pAlphabet = new CAlphabet(oAlphInfo);
  m_pCHAlphabet = NULL;

  pSettingsStore->SetStringParameter(SP_TRAIN_FILE, m_pAlphabet->GetTrainingFile());
  pSettingsStore->SetStringParameter(SP_GAME_TEXT_FILE, m_pAlphabet->GetGameModeFile());
  
  pSettingsStore->SetStringParameter(SP_DEFAULT_COLOUR_ID, m_pAlphabet->GetPalette());

  if(pSettingsStore->GetLongParameter(LP_ORIENTATION) == Dasher::Opts::AlphabetDefault)
    pSettingsStore->SetLongParameter(LP_REAL_ORIENTATION, m_pAlphabet->GetOrientation());
  // --

  CSymbolAlphabet alphabet(m_pAlphabet->GetNumberTextSymbols());
  alphabet.SetSpaceSymbol(m_pAlphabet->GetSpaceSymbol());      // FIXME - is this right, or do we have to do some kind of translation?
  alphabet.SetAlphabetPointer(m_pAlphabet);    // Horrible hack, but ignore for now.

  // Create an appropriate language model;

  m_iConversionID = oAlphInfo.m_iConversionID;

  //WZ: Mandarin Dasher Change
  //If statement checks for the specific Super PinYin alphabet, and sets language model to PPMPY
  if((m_iConversionID==2)&&(pSettingsStore->GetStringParameter(SP_ALPHABET_ID)=="Chinese Super Pin Yin, grouped by Dictionary")){

    std::string CHAlphabet = "Chinese / 简体中文 (simplified chinese, in pin yin groups)";
    Dasher::CAlphIO::AlphInfo oCHAlphInfo = pAlphIO->GetInfo(CHAlphabet);
    m_pCHAlphabet = new CAlphabet(oCHAlphInfo);

    CSymbolAlphabet chalphabet(m_pCHAlphabet->GetNumberTextSymbols());
    chalphabet.SetSpaceSymbol(m_pCHAlphabet->GetSpaceSymbol());
    chalphabet.SetAlphabetPointer(m_pCHAlphabet);
    //std::cout<<"CHALphabet size "<<chalphabet.GetSize(); [7603]
    m_pLanguageModel = new CPPMPYLanguageModel(pEventHandler, pSettingsStore, chalphabet, alphabet);

    std::cout<<"Setting PPMPY model"<<std::endl;
  }
  else{
  //End Mandarin Dasher Change

  // FIXME - return to using enum here
    switch (pSettingsStore->GetLongParameter(LP_LANGUAGE_MODEL_ID)) {
    case 0:
      m_pLanguageModel = new CPPMLanguageModel(pEventHandler, pSettingsStore, alphabet);
      break;
    case 2:
      m_pLanguageModel = new CWordLanguageModel(pEventHandler, pSettingsStore, alphabet);
      break;
    case 3:
      m_pLanguageModel = new CMixtureLanguageModel(pEventHandler, pSettingsStore, alphabet);
      break;  
    case 4:
      m_pLanguageModel = new CCTWLanguageModel(pEventHandler, pSettingsStore, alphabet);
      break;
      
    default:
      // If there is a bogus value for the language model ID, we'll default
      // to our trusty old PPM language model.
      m_pLanguageModel = new CPPMLanguageModel(pEventHandler, pSettingsStore, alphabet);    
      break;
    }
  }

  m_iLearnContext = m_pLanguageModel->CreateEmptyContext();
  
  // TODO: Tell the alphabet manager about the alphabet here, so we
  // don't end up having to duck out to the NCM all the time
  
  m_pAlphabetManager = new CAlphabetManager(pInterface, pNCManager, m_pLanguageModel, m_iLearnContext,m_iConversionID);
}

CAlphabetManagerFactory::~CAlphabetManagerFactory() {
  m_pLanguageModel->ReleaseContext(m_iLearnContext);
  delete m_pLanguageModel;
  delete m_pAlphabetManager;
}

CDasherNode *CAlphabetManagerFactory::GetRoot(CDasherNode *pParent, int iLower, int iUpper, void *pUserData) {
  return m_pAlphabetManager->GetRoot(pParent, iLower, iUpper, pUserData);
}

CTrainer::CTrainer(CLanguageModel *pLanguageModel, CAlphabet *pAlphabet, CAlphabet *pCHAlphabet) {
  m_pLanguageModel = pLanguageModel;
  m_pAlphabet = pAlphabet;
  m_pCHAlphabet = pCHAlphabet;
  m_Context = m_pLanguageModel->CreateEmptyContext();
}

void CTrainer::Train(const std::vector<symbol> &vSymbols) {

  for(std::vector<symbol>::const_iterator it(vSymbols.begin()); it != vSymbols.end(); ++it) {
    m_pLanguageModel->LearnSymbol(m_Context, *it);
  }
}

//TrainMandarin is used to train Mandarin Dasher: PPMPYLanguageModel
//Mandarin training is distinct from normal PPM training in that it uses two separate alphabets, and trains with py-character pairs. Despite so, implementation here may seem out of structure, and it could be necessary to revise later, particularly on robustness to deal with non-unicode chars
//The training of Mandarin Dasher may evolve in to possible paths: 1.Include punctuation (more work); 2.User defined training files (not sure how); 3.Learning as one types (more work)
//As Manager is produced, training happens in AlphabetManagerFactory

void CTrainer::TrainMandarin(const std::string &strUserLoc, const std::string &strSystemLoc){

  //TrainMandarin takes in the Super Pin Yin Alphabet, and uses the Mandarin Character alphabet stored in private AlphabetManagerFactory

  std::string strTrainingFile = m_pAlphabet->GetTrainingFile();

  std::string strUserPath = strUserLoc + strTrainingFile;
  std::string strSystemPath = strSystemLoc + strTrainingFile;

  FILE * fpUser = fopen (strUserPath.c_str(), "rb");
  FILE * fpSystem = fopen(strSystemPath.c_str(), "rb");
  FILE * fpTrain = fpSystem;
  
  if(!fpTrain) {

    fpTrain = fpUser;
    if(!fpTrain){
      printf("Mandarin Training File: cannot open file or incorrect directory\n");
    return;
    }
  }
  unsigned numberofchar = 0;


  size_t charsize = 1024;
    
  size_t trainBufferSize = 3*charsize*3;
  char szBuffer[trainBufferSize];
    
  std::string strChar;
  std::string strPY;
  char ctemp[4];
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
      m_pCHAlphabet->GetSymbols(&Symchar, &strtemp, 0);

      pos++;
          
    }
    Symchar.clear();
    Sympy.clear();
    m_pCHAlphabet->GetSymbols(&Symchar, &strChar, 0);
    m_pAlphabet->GetSymbols(&Sympy, &strPY, 0);      
    
    for(int i =0; i<Symchar.size(); i++){

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


void CTrainer::Reset() {
  m_pLanguageModel->ReleaseContext(m_Context);
  m_Context = m_pLanguageModel->CreateEmptyContext();
}

CTrainer::~CTrainer() {
  m_pLanguageModel->ReleaseContext(m_Context);
}

CTrainer *CAlphabetManagerFactory::GetTrainer() {
  return new CTrainer(m_pLanguageModel, m_pAlphabet, m_pCHAlphabet);
}
