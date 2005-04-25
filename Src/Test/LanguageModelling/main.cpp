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

#include "../../DasherCore/AlphIO.h"
#include "../../DasherCore/CustomAlphabet.h"

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

	string userlocation = "C:/Documents and Settings/dward/My Documents/dasher/Data/system.rc/";

	string filename = "alphabet.english.xml";
	
	vector<string> vFileNames;
	vFileNames.push_back(filename);

	// Set up the CAlphIO
	std::auto_ptr<CAlphIO> ptrAlphIO ( new CAlphIO( "", userlocation, vFileNames) );

//	string strID = "Default";
	string strID = "English alphabet with lots of punctuation";
	const CAlphIO::AlphInfo& AlphInfo = ptrAlphIO->GetInfo(strID);

	cout << "Alphabet " << filename << ":" << strID <<endl;

	
	// Create the Alphabet that converts plain text to symbols
	std::auto_ptr<CAlphabet> ptrAlphabet ( new CCustomAlphabet(AlphInfo) );
	
	string strFileCompress = "C:/Documents and Settings/dward/My Documents/dasher/Data/system.rc/training_english_GB.txt";

	cout << "Input file " << strFileCompress <<endl;


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
	CPPMLanguageModel lm( alphabet );

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

		lm.LearnSymbol(context, s);
	}
	cout << "Read " << sz << " bytes, " << vSymbols.size() << " symbols " << endl;
	cout << "Compression acheivable is " << -dSumLogP/ log(2.0)/ vSymbols.size() << " bits per symbol" << endl;
	return 0;
}