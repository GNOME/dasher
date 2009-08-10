// AlphabetMap.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////


#ifndef __AlphabetMap_h__
#define __AlphabetMap_h__

#ifndef DASHER_WIN32
#include <sys/types.h>
#endif

#include "../DasherTypes.h"

#include <vector>
#include <string>

namespace Dasher {
  class alphabet_map;
} 

/// \ingroup Alphabet
/// \{

/// If I were just using GCC, which comes with the CGI "STL" implementation, I would
/// use hash_map (which isn't part of the ANSI/ISO standard C++ STL, but hey it's nice).
/// Using a plain map is just too slow for training on large files (or it is with certain
/// STL implementations). I'm sure training could be made much faster still, but that's
/// another matter...
/// 
/// While I could (and probably should) get a hash_map for VC++ from
/// http://www.stlport.org I thought it would be nicer if people didn't have
/// to download extra stuff and then have to get it working alongside the STL
/// with VC++, especially for just one small part of Dasher.
/// 
/// The result is this:
/// ***************************************************
/// very much thrown together to get Dasher out ASAP.
/// ***************************************************
/// It is deliberately not like an STL container.
/// However, as it has a tiny interface, it should still be easy to replace.
/// Sorry if this seems really unprofressional.
/// 
/// Replacing it might be a good idea. On the other hand it could be customised
/// to the needs of the alphabet, so that it works faster. For example,
/// currently if I have a string "asdf", it might be that "a" is checked
/// then "as" is checked then "asd" is checked. I shouldn't need to keep
/// rehashing the leading characters. I plan to fix that here. Doing so with
/// a standard hash_map would be hard.
/// 
/// Usage:
/// alphabet_map MyMap(NumberOfEntriesWeExpect); // Can omit NumberOfEntriesWeExpect
/// MyMap.add("asdf", 15);
/// symbol i = MyMap.get("asdf") // i=15
/// symbol j = MyMap.get("fdsa") // j=0
/// 
/// You can't remove items once they are added as Dasher has no need for that.
/// 
/// IAM 08/2002
class Dasher::alphabet_map {

public:
  alphabet_map(unsigned int InitialTableSize = 255);
  ~alphabet_map();
  void Add(const std::string & Key, symbol Value);

  // Return the symbol associated with Key or Undefined.
  symbol Get(const std::string & Key) const;
  symbol GetSingleChar(char key) const;

private:
  class Entry {
  public:
    Entry(std::string Key, symbol Symbol, Entry * Next)
  :  Key(Key), Symbol(Symbol), Next(Next) {
    } std::string Key;
    symbol Symbol;
    Entry *Next;
  };

  // A standard hash -- could try and research something specific.
  inline unsigned int Hash(const std::string & Input) const {
    unsigned int Result = 0;

    typedef std::string::const_iterator CI;
    CI Cur = Input.begin();
    CI end = Input.end();

    while(Cur != end)
      Result = (Result << 1) ^ *Cur++;
    Result %= HashTable.size();

    return Result;
    /*
       if (Input.size()==1) // Speedup for ASCII text
       return Input[0];

       for (int i=0; i<Input.size(); i++)
       Result = (Result<<1)^Input[i];

       return Result%HashTable.size();
     */
  } std::vector < Entry > Entries;
  std::vector < Entry * >HashTable;
  const symbol Undefined;
  symbol *m_pSingleChars;
};
/// \}

#endif /* #ifndef __AlphabetMap_h__ */
