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
#include "../../DasherCore/LanguageModelling/LanguageModelParams.h"

#include "../../DasherCore/Alphabet/AlphIO.h"
#include "../../DasherCore/Alphabet/Alphabet.h"

#include <fstream>
#include <iostream>
#include <cmath>

#include <gsl/gsl_multimin.h>
#include <gsl/gsl_vector.h>

using namespace Dasher;
using namespace std;

double f( const gsl_vector *x, void *p );

struct params {
  CLanguageModelParams *settings;
  CSymbolAlphabet *alphabet;
  std::vector<symbol> *vSymbols;
};

int main( int argc, char *argv[] )
{

	/////////////////////////////////////////////////////////////////////////////
	// Load the alphabet 

	// This is currently a bit awkward - we cant just pass in a filename, it has to be a directory
	// and an list of filenames

  //	string userlocation = "C:/Documents and Settings/dward/My Documents/dasher/Data/system.rc/";
  string userlocation = "/usr/local/share/dasher/";
  
	string filename = "alphabet.english.xml";
	
	vector<string> vFileNames;
	vFileNames.push_back(filename);

	// Set up the CAlphIO
	std::auto_ptr<CAlphIO> ptrAlphIO ( new CAlphIO( "", userlocation, vFileNames) );

	// string strID = "Default";
	string strID = "English alphabet with lots of punctuation";
	const CAlphIO::AlphInfo& AlphInfo = ptrAlphIO->GetInfo(strID);

	//	cout << "Alphabet " << filename << ":" << strID <<endl;

	
	// Create the Alphabet that converts plain text to symbols
	std::auto_ptr<CAlphabet> ptrAlphabet ( new CAlphabet(AlphInfo) );
	
	//string strFileCompress = "C:/Documents and Settings/dward/My Documents/dasher/Data/system.rc/training_english_GB.txt";
	string strFileCompress = userlocation + "training_english_GB.txt";

	//	cout << "Input file " << strFileCompress <<endl;


	ifstream ifs(strFileCompress.c_str(), ios::in | ios::ate);
	if (!ifs)
		return 1;
	streampos sz = ifs.tellg();
	ifs.seekg(0, ios::beg);
	string strCompress( sz, '0');
	ifs.read(&strCompress[0], sz);

	std::vector<symbol> vSymbols;
	ptrAlphabet->GetSymbols(&vSymbols, &strCompress, false /*IsMore*/ );


	// Set up the language model for compression test

	// DJW - add some functionality to CAlphabet to get the CSymbolAlphabet
	CSymbolAlphabet alphabet( ptrAlphabet->GetNumberSymbols() );
	alphabet.SetSpaceSymbol( ptrAlphabet->GetSpaceSymbol() );

	CLanguageModelParams settings;

	gsl_multimin_fminimizer *min;
	gsl_vector *x;
	gsl_vector *xmin;
	gsl_vector *xstep;
	
	double minval;
	
	x = gsl_vector_alloc( 2 );
	
	gsl_vector_set( x, 0, 0 );
	gsl_vector_set( x, 1, 0 );
	
	xstep = gsl_vector_alloc( 2 );
	
	gsl_vector_set( xstep, 0, 1.0 );
	gsl_vector_set( xstep, 1, 1.0 );
	
	params p;

	p.settings = &settings;
	p.alphabet = &alphabet;
	p.vSymbols = &vSymbols;

	gsl_multimin_function minf;
	
	minf.f = f;
	minf.n = 2;
	minf.params = &p;
	
	min = gsl_multimin_fminimizer_alloc( gsl_multimin_fminimizer_nmsimplex , 2);
	gsl_multimin_fminimizer_set( min, &minf, x, xstep );

	for( int i(0); i < 1000; ++i ) {
	  gsl_multimin_fminimizer_iterate( min );
	  
	  xmin = gsl_multimin_fminimizer_x( min );
	  minval = gsl_multimin_fminimizer_minimum( min );
	  
	  std::cout << gsl_vector_get( xmin, 0 ) << " " << gsl_vector_get( xmin, 1 ) << " "  << minval << std::endl;
    }

    gsl_multimin_fminimizer_free( min );
    gsl_vector_free( x );
    gsl_vector_free( xstep );


	return 0;
}


double f( const gsl_vector *x, void *p )
{

  params *prm( static_cast< params * >( p ) );

  CLanguageModelParams *settings( prm -> settings );
  CSymbolAlphabet *alphabet( prm -> alphabet );
  std::vector<symbol> *vSymbols( prm -> vSymbols );
  
  int order(4);
  int exclusion(0);
  int update_exclusion(1);
  
  double alpha( exp( gsl_vector_get( x, 0 ) ) ); // max 0.431 - 0.434
  double beta( (tanh( gsl_vector_get( x, 1 ) ) * 0.49) + 0.5 ); //0.338 - 0.337

  settings->SetValue( "LMMaxOrder", order );
  
  settings->SetValue( "LMExclusion", exclusion );
  settings->SetValue( "LMUpdateExclusion", update_exclusion );
  
  settings->SetValue( "LMAlpha", alpha * 100 );
  settings->SetValue( "LMBeta", beta * 100 );
    
  //CWordLanguageModel lm( alphabet, &settings );
  CPPMLanguageModel lm( *alphabet, settings );
		
  
  CLanguageModel::Context context;
  context = lm.CreateEmptyContext();
  
  std::vector<unsigned int> Probs;
  int iNorm = 1<<16;
  
  int iASize = alphabet->GetSize();
  double dSumLogP=0;
  
  // Loop over symbols
  
  for (int i=0;i< vSymbols->size(); i++)
    {
      lm.GetProbs(context, Probs, iNorm);
      
      symbol s = (*vSymbols)[i];
      
      int j = Probs[s];
      
      // DJW
      // Add one to p since PPMLanguageModel is returning 0 for some symbols because of integer 
      // round-down
      // Maybe we insist that p!=0 for all symbols and fix the language models
      
      double p = double(j+1) / double (iNorm+iASize);
      DASHER_ASSERT(p!=0);
      
      
      dSumLogP+= log(p);
      
    
      
      lm.LearnSymbol(context, s);
    }
  
  return -dSumLogP/ log(2.0)/ vSymbols->size();
}
