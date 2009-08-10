// AlphabetMap.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"

#include "AlphabetMap.h"
#include <limits>

using namespace Dasher;
using namespace std;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

alphabet_map::alphabet_map(unsigned int InitialTableSize)
:HashTable(InitialTableSize <<1), Undefined(0) {
  Entries.reserve(InitialTableSize);

  const int numChars = numeric_limits<char>::max() + 1;
  m_pSingleChars = new symbol[numChars];
  for (int i = 0; i<numChars; i++) m_pSingleChars[i] = Undefined;
}

alphabet_map::~alphabet_map() {
  delete m_pSingleChars;
}

void alphabet_map::Add(const std::string &Key, symbol Value) {
  if (Key.length() == 1) {
    m_pSingleChars[Key[0]] = Value;
    return;
  }
  Entry *&HashEntry = HashTable[Hash(Key)];

  // Loop through Entries with the correct Hash value.
  for(Entry * i = HashEntry; i; i = i->Next) {
    if(i->Key == Key) {
      // Add symbol and leave
	  i->Symbol = Value;
    }
  }

  // When hash table gets 1/2 full...
  // (no I haven't optimised when to resize)
  if(Entries.size() << 1 >= HashTable.size()) {
    // Double up all the storage
    HashTable.clear();
    HashTable.resize(Entries.size() << 2);
    Entries.reserve(Entries.size() << 1);

    // Rehash as the pointers will all be mangled.
    for(unsigned int j = 0; j < Entries.size(); j++) {
      Entry *&HashEntry2 = HashTable[Hash(Entries[j].Key)];
      Entries[j].Next = HashEntry2;
      HashEntry2 = &Entries[j];
    }

    // Have to recall this function as the key's hash needs recalculating
    Add(Key, Value);
    return;
  }

  Entries.push_back(Entry(Key, Value, HashEntry));
  HashEntry = &Entries.back();
}

symbol alphabet_map::Get(const std::string &Key) const {
  if (Key.length() == 1) {
	return GetSingleChar(Key[0]);
  }
  // Loop through Entries with the correct Hash value.
  for(Entry * i = HashTable[Hash(Key)]; i; i = i->Next) {
    if(i->Key == Key) {
      return i->Symbol;
    }
  }

  return Undefined;
}

symbol alphabet_map::GetSingleChar(char key) const {return m_pSingleChars[key];}
