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

using namespace Dasher;
using namespace std;

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

// 	for( int order(0); order <= 10; ++order )
// 	  for( int exclusion(0); exclusion < 2; ++exclusion )
// 	    for( int update_exclusion(0); update_exclusion < 2; ++update_exclusion )

	int order(4);
	int exclusion(1);
	int update_exclusion(1);

	double alpha(0.0);
	double beta(0.5);

	int length( 5000 );
	

		if( order == 10 )
		  settings.SetValue( "LMMaxOrder", 10000 );
		else
		   settings.SetValue( "LMMaxOrder", order );

		settings.SetValue( "LMExclusion", exclusion );
		settings.SetValue( "LMUpdateExclusion", update_exclusion );

		settings.SetValue( "LMAlpha", alpha * 100 );
		settings.SetValue( "LMBeta", beta * 100 );
				
		
		//CWordLanguageModel lm( alphabet, &settings );
		CPPMLanguageModel lm( alphabet, &settings );
		
		
		CLanguageModel::Context context;
		context = lm.CreateEmptyContext();
		
		std::vector<unsigned int> Probs;
		int iNorm = 1<<16;
		
		int iASize = alphabet.GetSize();
		double dSumLogP=0;
		
		// Loop over symbols
		
			for (int i=0;i< vSymbols.size(); i++)
		  {
		    lm.GetProbs(context, Probs, iNorm);
		    
		    symbol s = vSymbols[i];
		    
		    int j = Probs[s];
		    
		    // DJW
		    // Add one to p since PPMLanguageModel is returning 0 for some symbols because of integer 
		    // round-down
		    // Maybe we insist that p!=0 for all symbols and fix the language models
		    
		    double p = double(j+1) / double (iNorm+iASize);
		    DASHER_ASSERT(p!=0);
		    
		    
		    dSumLogP+= log(p);
		    
// 		    if( i%100 == 0 )
// 		      std::cout << i << " " << lm.GetMemory() << std::endl;
		    
		    lm.LearnSymbol(context, s);
		  }
			cout << "Read " << sz << " bytes, " << vSymbols.size() << " symbols " << endl;
			cout << "Compressed size: " << -dSumLogP/ log(2.0) << " bits" << endl;
			cout << "Compression acheivable is " << -dSumLogP/ log(2.0)/ vSymbols.size() << " bits per symbol" << endl;
		//		cout << order << " " << exclusion << " " << update_exclusion << " " << -dSumLogP/ log(2.0)/ vSymbols.size() << endl;
			//		cout << skip << " " << -dSumLogP/ log(2.0)/ length << endl;

		

	return 0;
}
