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

#include "lib_expt.h"

#include <gsl/gsl_vector.h>
#include <gsl/gsl_multimin.h>

using namespace Dasher;
using namespace std;

double f( const gsl_vector *x, void *params );

class cCompressionExperiment : public cExperiment {
public:
  cCompressionExperiment( const std::string &oPrefix ) : cExperiment( oPrefix ) {};
  double Execute();
};

int main( int argc, char *argv[] )
{
  int iNumDimensions(3);

  gsl_multimin_function oMinFunction;

  oMinFunction.f = f;
  oMinFunction.n = iNumDimensions;
  oMinFunction.params = NULL;

  gsl_vector *pXInit( gsl_vector_alloc( iNumDimensions ) );
  gsl_vector_set_zero( pXInit );

  gsl_vector *pXStep( gsl_vector_alloc( iNumDimensions ) );
  gsl_vector_set_all( pXStep, 1.0 );

  gsl_multimin_fminimizer *pMinimizer( gsl_multimin_fminimizer_alloc( gsl_multimin_fminimizer_nmsimplex, iNumDimensions ));
  gsl_multimin_fminimizer_set( pMinimizer, &oMinFunction, pXInit, pXStep );


  std::ofstream oMinOutputFile( "fmin.op" );

  int iNumIterations( 100 );

  for( int i(0); i < iNumIterations; ++i ) {
    gsl_multimin_fminimizer_iterate( pMinimizer );

    gsl_vector *pXCurrent( gsl_multimin_fminimizer_x( pMinimizer ));
    double dMin( gsl_multimin_fminimizer_minimum( pMinimizer ));
    
    for( int j(0); j < iNumDimensions; ++j ) {
      oMinOutputFile << gsl_vector_get( pXCurrent, j ) << " ";
    }

     oMinOutputFile << dMin << std::endl;
    
  }

  return 0;
}

double f( const gsl_vector *x, void *params )
{
  cCompressionExperiment oExpt( "Foo" );

  oExpt.SetParameterInt( "LMAlpha", exp( gsl_vector_get( x, 0 ) ) * 100 );
  oExpt.SetParameterInt( "LMBeta", (tanh( gsl_vector_get( x, 1 ) ) + 1 ) * 50 );
  oExpt.SetParameterInt( "LMWordAlpha", exp( gsl_vector_get( x, 2 ) ) * 100 );

  return oExpt.Run();
}

double cCompressionExperiment::Execute() {


  std::cerr << "Setting up language model ... " << std::flush;

  string userlocation = "/usr/local/share/dasher/";
  string filename = "alphabet.english.xml";
	
  vector<string> vFileNames;
  vFileNames.push_back(filename);
  
  // Set up the CAlphIO
  std::auto_ptr<CAlphIO> ptrAlphIO ( new CAlphIO( "", userlocation, vFileNames) );

  string strID = "English alphabet with lots of punctuation";
  const CAlphIO::AlphInfo& AlphInfo = ptrAlphIO->GetInfo(strID);

  // Create the Alphabet that converts plain text to symbols
  std::auto_ptr<CAlphabet> ptrAlphabet ( new CAlphabet(AlphInfo) );
	
    string strFileCompress = userlocation + "training_english_GB.txt";
  // string strFileCompress = "testfile.txt";

  std::cerr << "done." << std::endl;

  std::cerr << "Loading data file ... " << std::flush;

  ifstream ifs(strFileCompress.c_str(), ios::in | ios::ate);

  if (!ifs)
    return 0.0;
  streampos sz = ifs.tellg();
  ifs.seekg(0, ios::beg);
  string strCompress( sz, '0');
  ifs.read(&strCompress[0], sz);

  std::cerr << "done." << std::endl;

  std::cerr << "Converting to symbols ... " << std::flush;

  std::vector<symbol> vSymbols;
  ptrAlphabet->GetSymbols(&vSymbols, &strCompress, false /*IsMore*/ );

  std::cerr << "done." << std::endl;

  // Set up the language model for compression test

  CSymbolAlphabet alphabet( ptrAlphabet->GetNumberSymbols() );
  alphabet.SetSpaceSymbol( ptrAlphabet->GetSpaceSymbol() );
  alphabet.SetAlphabetPointer( &*ptrAlphabet );


  int order(200);
  int exclusion(0);
  int update_exclusion(1);
  
  
  CLanguageModelParams settings;

  settings.SetValue( "LMMaxOrder", order );
  
  settings.SetValue( "LMExclusion", exclusion );
  settings.SetValue( "LMUpdateExclusion", update_exclusion );
  
  settings.SetValue( "LMAlpha", GetParameterInt( "LMAlpha" ) ); // 49
  settings.SetValue( "LMBeta", GetParameterInt( "LMBeta" ) ); // 77

  settings.SetValue( "LMWordAlpha", GetParameterInt( "LMWordAlpha" ) );
  
  settings.SetValue( "LMDictionary", 1 );

  CWordLanguageModel lm( alphabet, &settings );
  
  //  CPPMLanguageModel lm( alphabet, &settings );
		
  
  std::cerr << "Calculating compression ... " << std::flush;

   CLanguageModel::Context context;
   context = lm.CreateEmptyContext();
  
   std::vector<unsigned int> Probs;
   int iNormTot = 1<<16;
   int iNorm = iNormTot;
  

   int iASize = alphabet.GetSize();

   int iExtra = (iNormTot - iNorm) /( iASize-1);
   double dSumLogP=0;
  
   // Loop over symbols
  
   int iPcOld( -1 );

   for (int i=0;i< vSymbols.size(); i++)
     {

       int iPc( i*100 / vSymbols.size() );

       if( iPc > iPcOld )
	 std::cerr << iPc << "% " << dSumLogP << std::endl;

       iPcOld = iPc;

       lm.GetProbs(context, Probs, iNorm);
      
       symbol s = vSymbols[i];
      
       int j = Probs[s];

       //       std::cout <<j << std::endl;
      
//       // DJW
//       // Add one to p since PPMLanguageModel is returning 0 for some symbols because of integer 
//       // round-down
//       // Maybe we insist that p!=0 for all symbols and fix the language models
      


       double p = double(j+1) / double (iNormTot+iASize-1);
       
       DASHER_ASSERT(p!=0);
      
      
      dSumLogP+= log(p);
      
    
      
       lm.LearnSymbol(context, s);
     }

   std::cerr << "done." << std::endl;
 
   std::cout <<  -dSumLogP/ log(2.0)/ vSymbols.size() << std::endl;

   return  -dSumLogP/ log(2.0)/ vSymbols.size();
}
