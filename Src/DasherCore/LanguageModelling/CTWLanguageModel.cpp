// CTWLanguageModel.cpp
//
// Copyright (c) 2008 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// For information on the CTW method visit
// http://www.sps.ele.tue.nl/members/F.M.J.Willems/RESEARCH_files/CTW/ResearchCTW.htm
//

//#include "stdafx.h"
#include "CTWLanguageModel.h"
#include <math.h> // not in use anymore? needed it for log
#include <cstring>

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


CCTWLanguageModel::CCTWLanguageModel(int iNumSyms) : CLanguageModel(iNumSyms) {

	Dasher::CHashTable HashTable;  // create hashtable
	MaxDepth = 6;   // Maximum depth of the context tree
	MaxTries = 15;	// Max. number of attempts to find a spot in the hash table
	alpha = 14;		// 2: KT-estimator, 1: Laplace estimator, 14 = found by P.A.J. Volf to be 'good' for text
	MaxNrNodes = 4194304; // Max number of CCTWNodes in the table, trade-off between compression and memory usage. 2^22 = 4M
    TotalNodes = 0; // to keep track of how many nodes are created in the table.
	MaxFill = 0.9;  // Threshold to decide when to freeze the tree
	Failed = 0;		// keep track of how many nodes couldn't be found or created //debug
	Frozen = false; // to indicate if there is still room in the array of CCTWNodes
	MaxCount = 255; // Maximum value for the counts for count-halving
	NrBits = 9;    // number of bits used for representation of probabilities
	MaxValue = (1<<NrBits) -1;

    NrPhases = (int)ceil(log((double)(GetSize()))/log(2.0)); // number of bits per input-symbol
	Tree = new CCTWNode[MaxNrNodes]; // create array with all CCTWNodes.

	// Fill RootIndex table with indices of the RootNodes <- now I round up to next power of 2, should only create for possible symbols
	// Does that make a noticable difference in memory usage? Rootnodes with no symbols associated will accumulate no counts, so they only cost 1 node each (6 bytes).
	// Does it waste codespace? Do rootnodes with no symbols associated with them still get assigned a positive probability?
	// Set NrTries to max+1, to identify a RootNode
	for (int i = 0; i<(1<<NrPhases);i++)
    {
     RootIndex[i] = HashTable.GetHashOffSet(i) & (MaxNrNodes-1); // MaxNrNodes is a power of 2, & results in a mod operation, walk 'round' through the array
	 Tree[RootIndex[i]].NrTries = MaxTries+1; // in rootnodes the character value doesn't matter, as long as Tries = unique
	 TotalNodes++;
    }
}

CCTWLanguageModel::~ CCTWLanguageModel(){ // destructor
	delete [] Tree;
}

// **** Implementation of help functions *****
// To get the 'phase' bit in byte
#define ByteBit(byte,Phase)	((byte >> ((NrPhases-1)-Phase)) & 1)

// To find the index of the RootNode for a byte and a phase.
inline int CCTWLanguageModel::MapIndex(int b, int f){
	return ((1<<f)-1 + (b>>(NrPhases-f)));  //(2^phase -1) + dec. value of most significant bits
}

inline void CCTWLanguageModel::Scale(uint64 &a, uint64 &b)
{
	// Instead of using the full 16 bits for the probabilities, use only 9,
	// that's the only relevant information the other bits are noise <- depends on the value of MaxCount,
	// should use log2(alpha) bits more than for the counts a and b

	while ((a > MaxValue) | (b > MaxValue))
	{ // scale to right
		a = a>>1;
		b = b>>1;
	}
	while ((a < (MaxValue>>1)) & (b < (MaxValue>>1)))
	{// scale to left
		a = a<<1;
		b = b<<1;
	}
	//prevent 0
	if (a == 0)
	{
		a = 1;
	}
	if (b == 0)
	{
		b = 1;
	}
}

void CCTWLanguageModel::UpdatePath(int bit, int Update, int ValidDepth, int* & index, unsigned short int & P0, unsigned short int & P1)
{ // updates the CTW data of the nodes in 'index' with value of 'bit'.
  // Update specifies yes (1) or no (0) (GetProbs). In the case 'no', the new Pws are calculated but the tree is not
  // altered in any way

	uint64 GammaZero;  		// (GammaZero / (GammaZero + GammaOne)) = Pw(0|x)
	uint64 GammaOne;   		// (GammaOne  / (GammaZero + GammaOne)) = Pw(1|x)
	unsigned short int CountZero; // Number of zeros seen so far in this node
	unsigned short int CountOne;  // Number of ones seen so far in this node
	uint64 PeBlockZero;    		// Local block probability of sequence (0,x)
	uint64 PeBlockOne; 	  	// Local block probability of sequence (1,x)
	uint64 PwCBlockZero;     	// Product of the weighted block probabilities of the childnodes of sequence (0,x)
	uint64 PwCBlockOne;      	// Product of the weighted block probabilities of the childnodes of sequence (1,x)
	uint64 PeCondZero; 		// Conditional local probability (0|x)
	uint64 PeCondOne;  		// Conditional local probability (1|x)
	uint64 PwCBlock;      		// Product of the weighted block probabilities of the childnodes of sequence (x)
	uint64 PeBlock;	  		// Local block probability of sequence (x)

	// The deepest index can be a leaf, a failed node, or a not-placed node
	const int DeepestIndex = index[ValidDepth];

	if (DeepestIndex == MaxNrNodes) // node didn't exist yet, both probs. equal
	{
		GammaZero = MaxValue;
		GammaOne  = MaxValue;
	}
	else if (DeepestIndex == MaxNrNodes+1) // node couldn't be placed
	{   // could do more fancy things here
		GammaZero = MaxValue;
		GammaOne  = MaxValue;
	}
	else
	{ // node has to be a leaf
		CountZero = Tree[DeepestIndex].a;
		CountOne  = Tree[DeepestIndex].b;

		GammaZero = alpha*CountZero +1;
		GammaOne  = alpha*CountOne  +1;

		if (Update == 1) // update tree
		{// first update counts
			if(bit)
			{
				if (CountOne == MaxCount)
				{ // half counts
					CountZero = (CountZero+1) / 2;
					CountOne = (CountOne+1) / 2;
				}
				else
					CountOne++;
			}
			else // bit = 0
			{
				if (CountZero == MaxCount)
				{ // half counts
					CountZero = (CountZero+1) / 2;
					CountOne = (CountOne+1) / 2;
				}
				else
					CountZero++;
			}
			Tree[DeepestIndex].a = CountZero;
			Tree[DeepestIndex].b = CountOne;
		}	// end if update
	} // end if/else, deepest index done
	// now all the internal nodes, including the rootnode
	for(int i=ValidDepth-1;i>=0;i--)
	{
		CountZero = Tree[index[i]].a;
		CountOne  = Tree[index[i]].b;

		PwCBlock = Tree[index[i]].PwChild;
		PeBlock  = Tree[index[i]].Pe;

		PeCondZero = (alpha*CountZero)+1;
		PeCondOne =  (alpha*CountOne) +1;
		PeBlockZero = PeBlock*PeCondZero*(GammaOne+GammaZero);
	    PeBlockOne  = PeBlock*PeCondOne*(GammaOne+GammaZero);
		PwCBlockZero = PwCBlock*GammaZero*((alpha*(CountZero+CountOne))+2);
		PwCBlockOne  = PwCBlock*GammaOne *((alpha*(CountZero+CountOne))+2);

		GammaZero = (PeBlockZero + PwCBlockZero);
		GammaOne  = (PeBlockOne  + PwCBlockOne );

		Scale(GammaZero, GammaOne);

		if (Update == 1) // update tree
		{// first update counts
			if(bit)
			{
				if (CountOne == MaxCount)
				{ // half counts
					CountZero = (CountZero+1) / 2;
					CountOne = (CountOne+1) / 2;
				}
				else
					CountOne++;

				Scale(PeBlockOne, PwCBlockOne);
				Tree[index[i]].Pe = PeBlockOne; // conversion after scaling, no problem
				Tree[index[i]].PwChild = PwCBlockOne;
			}
			else // bit = 0
			{
				if (CountZero == MaxCount)
				{ // half counts
					CountZero = (CountZero+1) / 2;
					CountOne = (CountOne+1) / 2;
				}
				else
					CountZero++;

				Scale(PeBlockZero, PwCBlockZero);
				Tree[index[i]].Pe = PeBlockZero;
				Tree[index[i]].PwChild = PwCBlockZero;
			}
			Tree[index[i]].a = CountZero;
			Tree[index[i]].b = CountOne;
		} // end if update
	}
	P0 = GammaZero; // Gammas are already scaled back to 16 bits
	P1 = GammaOne;
}

int CCTWLanguageModel::FindPath(CCTWContext & context, char NewChar, int phase, int create, int* & index)
{ // Puts the Tree-array indices of the CCTWNodes on the path of Context in index.
  // Returns the depth till which the path is found, index[] deeper than that is garbage!
  // If 'create' = 1, new nodes are created when an empty spot is found
    int Stepsize = 0;
    int curindex = RootIndex[MapIndex(NewChar,phase)];   // Find root, depending on current (newest) character in context
	index[0] = curindex;

	// From the root, find/create the nodes, corresponding to the context
    for (unsigned int i=0; i<context.Context.size();i++)
    {
	  unsigned char CurChar = context.Context.at(i);
	  Stepsize = (HashTable.GetHashOffSet(CurChar)<<1)+1; // get stepsize. Shift+1 to keep result odd, to prevent cycles
      bool found = false;
      for (int Tries = 1; Tries<MaxTries; Tries++)
      {
        curindex = (curindex + Stepsize) & (MaxNrNodes-1);

		if (Tree[curindex].NrTries == Tries) // node in use, is it the correct node?
        {// see if this is the correct node: compare tries and last (current) character
          if (Tree[curindex].Symbol == CurChar) // node found
          {
            found = true;				// to avoid 'failed'
			index[i+1] = curindex;      // tell calling function where to find the node, i+1 because index[0] = rootnode
			break;						// to escape loop and continue with next character
          }
        }
        if (Tree[curindex].NrTries == 0) // empty node found, create new node
        {
		  if (!create) // No need to create a new node, let calling function know empty spot found
		  {
			  index[i+1] = MaxNrNodes; // to indicate node could be placed but didn't exist yet
			  return (i+1); // +1 since i=0 is the rootnode, always valid
		  }
          if (!Frozen) // Still space in the tree
		  {
			  Tree[curindex].NrTries = Tries;
			  Tree[curindex].Symbol = CurChar;
			  TotalNodes++;				// new node in use
			  if ((float)(TotalNodes)/(float)(MaxNrNodes) > MaxFill) // Max fillratio of tree reached, freeze tree
				  Frozen = true;
			  found = true;					// to avoid 'failed'
			  index[i+1] = curindex;		// tell calling function where to find the node, i+1 because index[0] = rootnode
			  break;						// to escape loop and continue with next character
		  }
		  else // can't create a new node
		  {
			  found = false;
			  index[i+1] = MaxNrNodes+1;	// to indicate node could not be placed
			  return (i+1);					// +i since i=0 is the rootnode, always valid
		  }
        } // else collision, set next step
      } // for Tries
      // after MaxTries attempts:
      if (!found) // check to see if we were succesfull
      { // apparently, character could not be placed
		index[i+1] =  MaxNrNodes+1;			// to indicate node could not be placed
		return i+1;				// indicate node could not be found/created for this phase
      } //if !found
    } // for i contextsize
return context.Context.size(); // all nodes on the path found/created
} // end findpath


// **** Implementation of interface functions  *****

void CCTWLanguageModel::EnterSymbol(Context CurContext, int Symbol)
{ // add Symbol to the front of Context. If there are more than MaxDepth symbols, pop the last one
	CCTWLanguageModel::CCTWContext &Context = *(CCTWLanguageModel::CCTWContext *) (CurContext);
	if (Context.Full == true)
		Context.Context.pop_back();

	Context.Context.push_front(Symbol);

	if (Context.Context.size() == MaxDepth)
		Context.Full = true;
}

void CCTWLanguageModel::LearnSymbol(Context CurContext, int Symbol)
{
  CCTWLanguageModel::CCTWContext &Context = *(CCTWLanguageModel::CCTWContext *) (CurContext);

  if (Context.Full == true) // context is complete, update the tree
  {	// find indices of the tree nodes corresponding to the context

	int *Index = new int[Context.Context.size()+1]; // +1 for the rootnode
	int ValidDepth = 0;
	for (int phase = 0;phase<NrPhases;phase++)
	{
		ValidDepth = FindPath(Context, Symbol, phase, 1, Index); // Find indices of the nodes for this phase and context
		// nodes on the path for this phase found, update the tree
		unsigned short int stubZ =0;
		unsigned short int stubO =0;
		UpdatePath(ByteBit(Symbol,phase), 1, ValidDepth, Index, stubZ, stubO);
	}
	delete [] Index;

	Context.Context.pop_back();     // only delete last symbol if context is complete
  }
  Context.Context.push_front(Symbol); // update context with newest symbol

  if (Context.Context.size() == MaxDepth)
		Context.Full = true;
}

void CCTWLanguageModel::GetProbs(Context context, std::vector<unsigned int> &Probs, int Norm, int iUniform) const
{   	// because we reuse findpath and updatepath function, we need to de-const the object :(
	// findpath should be declared const anyway (?)

	CCTWLanguageModel* self = const_cast<CCTWLanguageModel*>(this);

	CCTWContext *CTWContext = ( CCTWContext *)(context);
	CCTWContext LocalContext(*CTWContext);

	int iNumSymbols = GetSize();
	int MinProb = iUniform / iNumSymbols; //smallest probability to assign

	Probs.resize(iNumSymbols);
	int pLeft = 0;

	// calculate probabilities of all possible symbols. Again assume all 2^NrPhases
	int *Index = new int[LocalContext.Context.size()+1]; // +1 for the rootnode

	vector <unsigned short int>Interval((1<<(NrPhases+1))-1); // number of rootnodes*2 (1 prob for bit 0 and 1 each)
	if (Norm>65535)
	{
		Interval[0]=65535; // to prevent overflow
	    pLeft = Norm-65535; // if Norm is way bigger than 2^16 - 1, uniformly distributing the 'leftover' could still cause overflow
	}
	else
		Interval[0] = Norm;

	int ValidDepth = 0;
	uint64 IntervalB = 0; // 'base' interval
	uint64 IntervalZ = 0; // divided interval for the 0-branch
	uint64 IntervalO = 0; // divided interval for the 1-branch
	unsigned int MinInterval = 0;
	unsigned short int Pw0 = 0;
	unsigned short int Pw1 = 0;

	for (int phase = 0;phase<NrPhases;phase++)
	{
		int stepsize = 1 <<(NrPhases-phase); // 2^maxphase-1 - fase
		for (int steps = 0;steps < 1<<phase;steps++)
		{ // find the path for all needed symbols
			// FIXME now I round up to next power of 2
			ValidDepth = self->FindPath(LocalContext, steps*stepsize, phase, 0, Index); // Find indices of the nodes for this phase and context

			IntervalB = Interval[(1<<phase)+ steps - 1];
			self->UpdatePath(0,0, ValidDepth, Index, Pw0, Pw1);

			IntervalZ = (IntervalB * Pw0)/(uint64)(Pw0+Pw1); // flooring, influence of flooring P0 instead of P1 is negligible
			IntervalO = IntervalB - IntervalZ;

			MinInterval = MinProb*1<<(NrPhases-1-phase); // leafs for each rootnode at the current phase, assuming a full alphabet!!

			//make sure all leafs from this point will get at least probability 1
			if(IntervalZ < MinInterval)
			{
				IntervalO = IntervalO - (MinInterval-IntervalZ);
				IntervalZ = IntervalZ + (MinInterval-IntervalZ);
			}
			else if(IntervalO < MinInterval)
			{
				IntervalZ = IntervalZ - (MinInterval-IntervalO);
				IntervalO = IntervalO + (MinInterval-IntervalO);
			}

			Interval[(1<<(phase+1))+ 2*steps - 1] = IntervalZ;
			Interval[(1<<(phase+1))+ 2*steps] = IntervalO;
		} // for steps
	} // for phase
	delete [] Index;

	// Copy the intervals associated with the actual symbols to the vector Probs.
	Probs.assign((Interval.end()-(1<<NrPhases)), (Interval.end()-(1<<NrPhases)+iNumSymbols));
	pLeft +=Probs[0]; //symbol 0 is a special dummy symbol, should get prob. 0
	Probs[0] = 0;

	// calculate how many extra symbols exist in tree, because iNumSymbols is not a power of 2
    int Extra = (1<<NrPhases) - iNumSymbols;
	// take the probabilities from non-existing symbols and re-divide it over existing symbols
	for (int j = Extra; j >0; j-- ) {
		pLeft +=Interval[Interval.size()-j];
	}

	int iLeft = iNumSymbols-1; //divide the probability that is left over the symbols
	for(int j = 1; j < iNumSymbols; ++j) {
		unsigned int p = pLeft / iLeft;
		Probs[j] += p;
		--iLeft;
		pLeft -= p;
	}

} // end function GetProbs


bool CCTWLanguageModel::WriteToFile(std::string strFilename, std::string AlphabetName){
	SLMFileHeader GenericHeader;
	// Magic number ("%DLF" in ASCII)
	GenericHeader.szMagic[0] = '%';
	GenericHeader.szMagic[1] = 'D';
	GenericHeader.szMagic[2] = 'L';
	GenericHeader.szMagic[3] = 'F';

	GenericHeader.iAlphabetSize = GetSize(); // Number of characters in the alphabet
	GenericHeader.iHeaderVersion = 1; // Version of the header
	GenericHeader.iLMID = 5; // ID of the language model, 5 for CTW
	GenericHeader.iLMMinVersion = 1; //Minimum backwards compatible version for the language model
	GenericHeader.iLMVersion = 1; // Version number of the language model, version 1 is the stored hashtable, april 2007
	GenericHeader.iHeaderSize = sizeof(SLMFileHeader) + AlphabetName.length(); // Total size of header (including variable length alphabet name)

	FILE *OutputFile;
	OutputFile = fopen(strFilename.c_str(), "wb");
	if(OutputFile)
	{
		char * buffer;
		buffer = new char[AlphabetName.length()+1];
		strcpy(buffer, AlphabetName.c_str());

		// write header
		fwrite(GenericHeader.szMagic , sizeof(GenericHeader.szMagic[0]), sizeof(GenericHeader.szMagic), OutputFile );
		fwrite(&GenericHeader.iHeaderVersion, 2,1, OutputFile);
		fwrite(&GenericHeader.iHeaderSize, 2,1, OutputFile);
		fwrite(&GenericHeader.iLMID, 2,1, OutputFile);
		fwrite(&GenericHeader.iLMVersion, 2,1, OutputFile);
		fwrite(&GenericHeader.iLMMinVersion, 2,1, OutputFile);
		fwrite(&GenericHeader.iAlphabetSize, 2,1, OutputFile);
		fwrite(buffer, 1, AlphabetName.length(), OutputFile ); // UTF-8 encoded alphabet name (variable length struct)
		delete[] buffer;

		// CTW specific, not in SLMFileHeader
		fwrite(&MaxNrNodes, 4,1, OutputFile);

		for(int i=0;i<MaxNrNodes;i++)
		{
			fwrite(&Tree[i].a, 1,1, OutputFile);
			fwrite(&Tree[i].b, 1,1, OutputFile);
			fwrite(&Tree[i].Symbol, 1,1,OutputFile);
			fwrite(&Tree[i].NrTries, 1,1,OutputFile);
			fwrite(&Tree[i].Pe, 2,1,OutputFile);
			fwrite(&Tree[i].PwChild, 2,1,OutputFile);
		}
		fclose(OutputFile);
		return true;
	}
	else
		return false;
}

bool CCTWLanguageModel::ReadFromFile(std::string strFilename, std::string AlphabetName){
	FILE *InputFile;
	InputFile = fopen(strFilename.c_str(), "rb");
	if(InputFile)
	{
		/* Read and check header, close file and return failure when header is not what we expect.
		TODO: Checking of the SLMFileHeader, which is not specific to the CTW languagemodel should be done in DasherModel,
		only CTW specific information (MaxNrNodes) should be checked here.
		The values to compare with should be parameters and not hardcoded. */

		SLMFileHeader GenericHeader;
		char * ReadAlphabetName;

		fread(&GenericHeader.szMagic , sizeof(GenericHeader.szMagic[0]), sizeof(GenericHeader.szMagic), InputFile);
		if(memcmp(GenericHeader.szMagic,"%DLF",4))
		{ // magic strings not equal
			return false;
		}
		fread(&GenericHeader.iHeaderVersion,2,1, InputFile);
		if(GenericHeader.iHeaderVersion != 1)
		{ // unknown header version
			return false;
		}
		fread(&GenericHeader.iHeaderSize,2,1, InputFile);
		fread(&GenericHeader.iLMID,2,1, InputFile);
		if(GenericHeader.iLMID != 5)
		{ // header indicates this is not a CTW model
			return false;
		}
		fread(&GenericHeader.iLMVersion,2,1, InputFile);
		fread(&GenericHeader.iLMMinVersion,2,1, InputFile);
		if(GenericHeader.iLMMinVersion > 1)
		{ // header indicates stored model newer than we can handle
			return false;
		}
		fread(&GenericHeader.iAlphabetSize,2,1, InputFile);
		if(GenericHeader.iAlphabetSize != GetSize())
		{ // header indicates stored model uses an alphabet of different size
			return false;
		}

		ReadAlphabetName = new char[GenericHeader.iHeaderSize - sizeof(SLMFileHeader)+1];
		fread(ReadAlphabetName,1,GenericHeader.iHeaderSize - sizeof(SLMFileHeader), InputFile);
		ReadAlphabetName[GenericHeader.iHeaderSize - sizeof(SLMFileHeader)] = '\0'; // write the terminating 0 and read it in as well

		if(strcmp(ReadAlphabetName,AlphabetName.c_str()))
		{ // header indicates stored model uses a different alphabet
			delete[] ReadAlphabetName;
			return false;
		}
		delete[] ReadAlphabetName;
		int ReadNrNodes;
		fread(&ReadNrNodes,4,1, InputFile);
		if(ReadNrNodes != MaxNrNodes)
		{ // header indicates different number of nodes in the hashtable
			return false;
		}

		for(int i=0;i<MaxNrNodes;i++)
		{
			fread(&Tree[i].a,1,1,InputFile);
			fread(&Tree[i].b,1,1,InputFile);
			fread(&Tree[i].Symbol, 1,1,InputFile);
			fread(&Tree[i].NrTries, 1,1,InputFile);
			fread(&Tree[i].Pe, 2,1,InputFile);
			fread(&Tree[i].PwChild, 2,1,InputFile);
		}
		fclose(InputFile);
		return true;
	}
	else
		return false;
}

inline CLanguageModel::Context CCTWLanguageModel::CreateEmptyContext() {
    CCTWContext *pCont = new CCTWContext;
	return (Context) pCont;
}

inline CLanguageModel::Context CCTWLanguageModel::CloneContext(Context Copy) {
	CCTWContext *pCont = new CCTWContext;
    CCTWContext *pCopy = (CCTWContext *) Copy;

	pCont->Full = pCopy->Full;
	pCont->Context.assign(pCopy->Context.begin( ), pCopy->Context.end( ));

	return (Context) pCont;
}

inline void CCTWLanguageModel::ReleaseContext(Context release) {
	  delete (CCTWContext *) release;
}
