// HashTable.h
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

#ifndef __Hashtable_h__
#define __Hashtable_h__

#include "LanguageModel.h"	

using namespace Dasher;
//using namespace std;

namespace Dasher {

  class CHashTable { //class to store the hashtable used to find indices of nodes	 
	public:
		CHashTable(){}		
		int GetHashOffSet(int c){
			return Tperm[c];
		}	 
		private: 
			static const unsigned int Tperm[256];				
  };

} // end namespace 

#endif // __Hashtable_h__
