// CTWLanguageModel.h
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

#ifndef __CTWLanguageModel_h__
#define __CTWLanguageModel_h__

#include <vector>			
#include <fstream>			
#include "LanguageModel.h"	
#include "HashTable.h"
#include <deque>			

using namespace Dasher;
using namespace std;

namespace Dasher {  
 
  // CTW language model 
  class CCTWLanguageModel: public CLanguageModel {
  public:    
	CCTWLanguageModel(int iNumSyms);
	virtual ~ CCTWLanguageModel(); 

    Context CreateEmptyContext();			
	Context CloneContext(Context context);	
	void ReleaseContext(Context context);	

    virtual void EnterSymbol(Context context, int Symbol); 
	virtual void LearnSymbol(Context context, int Symbol); 	
	virtual void GetProbs(Context context, std::vector < unsigned int >&Probs, int Norm, int iUniform) const; 
	
	Dasher::CHashTable HashTable; // Hashtable used for storing CCTWNodes in an array
      unsigned int MaxDepth;	// Maximum depth of the tree
	int MaxTries;	// Determines how many times to try to find an empty index for a new node (max number of collisions)
	int alpha;		// Parameter of the KT-estimator 
	
	int MaxNrNodes; // Max number of CCTWNodes in the table
	int TotalNodes; // keep track of how many nodes are created, and memory usage
	float MaxFill;  // Treshold to decide when to freeze the tree
	int Failed;		// keep track of how many nodes couldn't be found or created		
	bool Frozen;	// to indicate if there is still room in the array of CCTWNodes
	int MaxCount;   // The maximum number of a and b, the counts of zeros and ones in each CCTWnode, before they are halved.
	unsigned short int MaxValue; // representation of probability one
	unsigned short int NrBits;   // number of bits used to represent probabilities 
	int NrPhases;   // Number of bits per input Symbol

	class CCTWNode {
    public:         
      unsigned char a;		 // number of zeros
	  unsigned char b;	 // number of ones
	  unsigned char Symbol;  // newest context symbol, needed to check if the correct node is found by hashing
	  unsigned char NrTries; // needed to check if the correct node is found by hashing. MaxTries is bounded, 4-5 bits would suffice
	  unsigned short int Pe; // Numerator of the local block probability
	  unsigned short int PwChild; // Numerator of the product of the weighted block probabilities of the child nodes

	  CCTWNode(){ // constructor		
		a = 0;
		b = 0;
	    Pe = 511;      // Should be initialised to MaxValue, make that a static const
		PwChild = 511;
		Symbol = 0; 
		NrTries = 0; 		
	  }
	~CCTWNode(){}
	};
	
	CCTWNode *Tree;     // array of CCTWNodes
	int RootIndex[255]; // array of indices of the RootNodes. Depends on number of bits per character, assuming 8 
	//TODO only create necessary rootnodes no more. Need to dynamically create this array

	class CCTWContext {
	public:
		CCTWContext(CCTWContext const &Input) { // copy constructor
			Full = Input.Full;
			Context.assign(Input.Context.begin( ), Input.Context.end( ));			
		} 
		CCTWContext() { // default constructor
			Full = false;			
		}		
		~ CCTWContext(){};
		bool Full;
		deque<int> Context;	
	};	

	virtual bool WriteToFile(std::string strFilename, std::string AlphabetName);
	virtual bool ReadFromFile(std::string strFilename, std::string AlphabetName);	
// **** used help functions *****
    
	private:

	int MapIndex(int b, int f); 	
		
	void UpdatePath(int bit, int Update, int ValidDepth, int* & index, unsigned short int & Pw0, unsigned short int & Pw1);
	// calculates the new weighted conditional probabilities of a zero and a one (Pw0, Pw1) after an update with 'bit'
	// on the path given in 'index'. 'Update' specifies whether or not the tree is actually updated (LearnSymbol),
	// or only the probabilities are calculated (GetProbs).
	
	int FindPath(CCTWContext & context, char NewChar, int phase, int create, int* & index); 
    // Puts the Tree-array indices of the CCTWNodes on the path of Context in index. 
	// Returns depth of found path. ``Create'' specifies whether non-existing nodes need to be
	// created (LearnSymbol) or not (GetProbs).

	void Scale(uint64 & a, uint64 & b);
	// Scales both inputs to fit in NrBits

  }; // end class CCTWLanguageModel

} // end namespace 

#endif // __LanguageModelling__CTWLanguageModel_h__
