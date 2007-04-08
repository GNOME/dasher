// main.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

// LanguageModel test application

#include "../../Common/Common.h"
#include "../../DasherCore/LanguageModelling/PPMLanguageModel.h"
#include "../../DasherCore/LanguageModelling/WordLanguageModel.h"
#include "../../DasherCore/LanguageModelling/MixtureLanguageModel.h"
#include "../../DasherCore/LanguageModelling/LanguageModelParams.h"

#include "../../DasherCore/Alphabet/AlphIO.h"
#include "../../DasherCore/Alphabet/Alphabet.h"

#include <fstream>
#include <iostream>
#include <cmath>
#include <sstream>

#include "lib_expt.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>

#include <TextHandler.hpp>
#include <TrecParser.hpp>

using namespace Dasher;
using namespace std;

double f(const gsl_vector * x, void *params);

class cCompressionExperiment:public cExperiment {
public:
  cCompressionExperiment(const std::string & oPrefix):cExperiment(oPrefix) {
  };
  double Execute();
};

class dummy_handler:public TextHandler {

  // A dummy text handler to allow us to use the trec parser
public:

  dummy_handler(std::stringstream * _ss) {
    ss = _ss;
  };

  virtual char *handleWord(char *word, const char *original, PropertyList * list) {
    (*ss) << word << " ";
    return word;
  };

  virtual char *handleEndDoc(char *token, const char *original, PropertyList * list) {
    return token;
  };

protected:
  std::stringstream * ss;
};

int main(int argc, char *argv[]) {
  int iNumDimensions(3);

  gsl_multimin_function oMinFunction;

  oMinFunction.f = f;
  oMinFunction.n = iNumDimensions;
  oMinFunction.params = NULL;

  gsl_vector *pXInit(gsl_vector_alloc(iNumDimensions));

  gsl_vector_set(pXInit, 0, -0.56636);
  gsl_vector_set(pXInit, 1, 0.60203);
  gsl_vector_set(pXInit, 2, 3.89152);

  gsl_vector *pXStep(gsl_vector_alloc(iNumDimensions));
  gsl_vector_set_all(pXStep, 1.0);

  f(pXInit, NULL);

//    gsl_multimin_fminimizer *pMinimizer( gsl_multimin_fminimizer_alloc( gsl_multimin_fminimizer_nmsimplex, iNumDimensions ));
//    gsl_multimin_fminimizer_set( pMinimizer, &oMinFunction, pXInit, pXStep );

//    std::ofstream oMinOutputFile( "fmin.op" );

//    int iNumIterations( 100 );
//    for( int i(0); i < iNumIterations; ++i ) {
//      gsl_multimin_fminimizer_iterate( pMinimizer );

//      gsl_vector *pXCurrent( gsl_multimin_fminimizer_x( pMinimizer ));
//      double dMin( gsl_multimin_fminimizer_minimum( pMinimizer ));

//      for( int j(0); j < iNumDimensions; ++j ) {
//        oMinOutputFile << gsl_vector_get( pXCurrent, j ) << " ";
//      }

//       oMinOutputFile << dMin << std::endl;

//    }

  return 0;
}

double f(const gsl_vector *x, void *params) {
  cCompressionExperiment oExpt("Experiment5");

  oExpt.SetParameterInt("LMAlpha", exp(gsl_vector_get(x, 0)) * 100);
  oExpt.SetParameterInt("LMBeta", (tanh(gsl_vector_get(x, 1)) + 1) * 50);
  oExpt.SetParameterInt("LMWordAlpha", exp(gsl_vector_get(x, 2)) * 100);

  oExpt.SetParameterInt("LetterOrder", 200);

  oExpt.SetParameterInt("ModelType", 1);
  oExpt.SetParameterInt("Dictionary", 1);
  oExpt.SetParameterInt("LetterExclusion", 1);

  return oExpt.Run();
}

double cCompressionExperiment::Execute() {

  std::cerr << "Setting up language model ... " << std::flush;

  string userlocation = "/usr/local/share/dasher/";
  string filename = "alphabet.english.xml";

  vector < string > vFileNames;
  vFileNames.push_back(filename);

  // Set up the CAlphIO
  std::auto_ptr < CAlphIO > ptrAlphIO(new CAlphIO("", userlocation, vFileNames));

  string strID = "English alphabet with lots of punctuation";
  const CAlphIO::AlphInfo & AlphInfo = ptrAlphIO->GetInfo(strID);

  // Create the Alphabet that converts plain text to symbols
  std::auto_ptr < CAlphabet > ptrAlphabet(new CAlphabet(AlphInfo));

  string strFileCompress = userlocation + "training_english_GB.txt";
  // string strFileCompress = "testfile.txt";

  std::cerr << "done." << std::endl;

  std::cerr << "Loading data file ... " << std::flush;

  //

  std::stringstream strCompress;

  TrecParser tp;
  dummy_handler dh(&strCompress);

  tp.setTextHandler(&dh);

  //  tp.parseFile( "/mnt/data2/pjc51/enron/enron_short_trec.txt" );

  tp.parseFile("/data/tiree2/pjc51/enron/enron_short_trec.txt");

  //

  std::cerr << "done." << std::endl;

  std::cerr << "Converting to symbols ... " << std::flush;

  int iLength(strCompress.str().size());
  int iTestSize(10000);

  std::vector < symbol > vSymbols;
  ptrAlphabet->GetSymbols(&vSymbols, &(strCompress.str().substr(0, iLength - iTestSize)), false /*IsMore */ );

  std::vector < symbol > vSymbolsTest;
  ptrAlphabet->GetSymbols(&vSymbolsTest, &(strCompress.str().substr(iLength - iTestSize, iTestSize)), false /*IsMore */ );

  std::cerr << "(" << vSymbolsTest.size() << " test symbols) ";

  std::cerr << "done." << std::endl;

  // Set up the language model for compression test

  CSymbolAlphabet alphabet(ptrAlphabet->GetNumberSymbols());
  alphabet.SetSpaceSymbol(ptrAlphabet->GetSpaceSymbol());
  alphabet.SetAlphabetPointer(&*ptrAlphabet);

  int order(200);
  int exclusion(0);
  int update_exclusion(1);

  CLanguageModelParams settings;

  settings.SetValue("LMMaxOrder", GetParameterInt("LetterOrder"));

  settings.SetValue("LMExclusion", exclusion);
  settings.SetValue("LMUpdateExclusion", update_exclusion);

  settings.SetValue("LMAlpha", GetParameterInt("LMAlpha"));     // 49
  settings.SetValue("LMBeta", GetParameterInt("LMBeta"));       // 77

  settings.SetValue("LMWordAlpha", GetParameterInt("LMWordAlpha"));
  settings.SetValue("LMDictionary", GetParameterInt("Dictionary"));

  settings.SetValue("LMLetterExclusion", GetParameterInt("LetterExclusion"));

  CLanguageModel *lm;

  switch (GetParameterInt("ModelType")) {
  case 0:
    lm = new CPPMLanguageModel(alphabet, &settings);
    break;
  case 1:
    lm = new CWordLanguageModel(alphabet, &settings);
    break;
  }

  std::cerr << "Calculating compression ... " << std::flush;

  CLanguageModel::Context context;
  context = lm->CreateEmptyContext();

  std::vector < unsigned int >Probs;
  int iNormTot = 1 << 16;
  int iNorm = iNormTot;

  int iASize = alphabet.GetSize();

  int iExtra = (iNormTot - iNorm) / (iASize - 1);
  double dSumLogP = 0;

  // Loop over symbols

  int iPcOld(-1);

  for(int i = 0; i < vSymbols.size(); i++) {

    int iPc(i * 100 / vSymbols.size());

    if(iPc > iPcOld) {

      // Do a test...

      CLanguageModel::Context oTestContext;
      oTestContext = lm->CreateEmptyContext();

      double dSumLogPTest(0.0);

      int iTestCount(0);

      for(int k(0); k < vSymbolsTest.size(); ++k) {

        lm->GetProbs(oTestContext, Probs, iNorm);

        ++iTestCount;

        symbol s = vSymbolsTest[k];

        //      std::cerr << s << std::endl;

        int j = Probs[s];

        double p = static_cast < double >(j + 1) / static_cast < double >(iNormTot + iASize - 1);

        int iTot(0);

        for(int l(1); l < iASize; ++l) {

          double dPTemp(static_cast < double >(Probs[l] + 1) / static_cast < double >(iNormTot + iASize - 1));

          iTot += Probs[l];

          if((dPTemp <= 0.0) || (dPTemp >= 1.0))
            std::cout << "warning - prob of " << dPTemp << std::endl;

        }
        if(iTot > iNorm)
          std::cout << "Total: " << iTot << ", " << iNorm << std::endl;

        DASHER_ASSERT(p != 0);

        // std::cout << "p: " << p << std::endl;

        dSumLogPTest += log(p);

        lm->EnterSymbol(oTestContext, s);

      }
      std::cerr << iPc << " " << iTestCount << "% " << dSumLogPTest << " " << i << " " << -dSumLogPTest / log(2.0) / static_cast < double >(vSymbolsTest.size()) << std::endl;
    }

    iPcOld = iPc;

    lm->GetProbs(context, Probs, iNorm);

    symbol sbl = vSymbols[i];

    lm->LearnSymbol(context, sbl);
  }

  std::cerr << "done." << std::endl;

  delete lm;

  return -dSumLogP / log(2.0) / vSymbols.size();
}
