// AlphabetMap.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////

#include "AlphabetMap.h"

using namespace Dasher;
using namespace std;

alphabet_map::alphabet_map(unsigned int InitialTableSize)
	: Undefined(0), HashTable(InitialTableSize<<1)
{
	Entries.reserve(InitialTableSize);
}


void alphabet_map::Add(const string& Key, symbol Value)
{
	RecursiveAdd(Key, Value, false);
}


void alphabet_map::RecursiveAdd(const string& Key, symbol Value, bool PrefixFlag)
{
	Entry*& HashEntry = HashTable[Hash(Key)];
	
	// Loop through Entries with the correct Hash value.
	for (Entry* i = HashEntry; i; i=i->Next) {
		if (i->Key==Key) {
			if (PrefixFlag) {
				// Just tagging - don't change symbol. Recurse if necessary
				i->KeyIsPrefix = true;
				if (Key.size()>1)
					RecursiveAdd(Key.substr(Key.size()-1), Undefined, true);
			} else {
				// Add symbol and leave
				i->Symbol = Value;
			}
			return;
		}
	}
	
	// When hash table gets 1/2 full...
	// (no I haven't optimised when to resize)
	if (Entries.size()<<1 >= HashTable.size()) {
		// Double up all the storage
		HashTable.clear();
		HashTable.resize(Entries.size()<<2);
		Entries.reserve(Entries.size()<<1);
		
		// Rehash as the pointers will all be mangled.
		for (uint j=0; j<Entries.size(); j++) {
			Entry*& HashEntry2 = HashTable[Hash(Entries[j].Key)];
			Entries[j].Next = HashEntry2;
			HashEntry2 = &Entries[j];
		}
		
		// Have to recall this function as the key's hash needs recalculating
		RecursiveAdd(Key, Value, PrefixFlag);
		return;
	}
	
	Entries.push_back(Entry(Key, Value, HashEntry));
	HashEntry = &Entries.back();
}


symbol alphabet_map::Get(const string& Key, bool* KeyIsPrefix) const
{
	// Loop through Entries with the correct Hash value.
	for (Entry* i = HashTable[Hash(Key)]; i; i=i->Next) {
		if (i->Key==Key) {
			if (KeyIsPrefix!=0)
				*KeyIsPrefix = i->KeyIsPrefix;
			return i->Symbol;
		}
	}
	
	if (KeyIsPrefix!=0)
		*KeyIsPrefix = false;
	return Undefined;
}
