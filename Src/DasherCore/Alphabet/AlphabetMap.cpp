// AlphabetMap.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray
//
/////////////////////////////////////////////////////////////////////////////


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../../Common/Common.h"

#include "AlphabetMap.h"
#include <limits>
#include <iostream>
#include <sstream>

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

class utf8_length
{
public:
  utf8_length();
  int operator[](const unsigned char) const;
  int max_length;
private:
  int utf8_count_array[0x100];
};
static utf8_length m_utf8_count_array;

utf8_length::utf8_length()
{
  int i;
  
  memset(utf8_count_array, 0, sizeof(utf8_count_array));
  for (i = 0x00; i <= 0x7f; ++i) utf8_count_array[i] = 1;
  for (i = 0xc0; i <= 0xdf; ++i) utf8_count_array[i] = 2;
  for (i = 0xe0; i <= 0xef; ++i) utf8_count_array[i] = 3;
  for (i = 0xf0; i <= 0xf7; ++i) utf8_count_array[i] = 4;
  max_length = 4;
  /* The following would be valid according to RFC 2279 which was rendered
   * obsolete by RFC 3629
   * for (i = 0xf8; i <= 0xfb; ++i) utf8_count_array[i] = 5;
   * for (i = 0xfc; i <= 0xfd; ++i) utf8_count_array[i] = 6;
   * max_length = 6;
   *
   * and from RFC 3629:
   * o  The octet values C0, C1, F5 to FF never appear.
   */
  utf8_count_array[0xc0] = utf8_count_array[0xc1] = 0;
  for (i = 0xf5; i <= 0xff; ++i) utf8_count_array[i] = 0;
}

int utf8_length::operator[](const unsigned char i) const
{
  return utf8_count_array[i];
}

////////////////////////////////////////////////////////////////////////////

CAlphabetMap::SymbolStream::SymbolStream(const CAlphabetMap &_map, std::istream &_in)
: map(_map), in(_in), pos(0), len(0) {
  readMore();
}

void CAlphabetMap::SymbolStream::readMore() {
  //len is first unfilled byte
  in.read(&buf[len], 1024-len);
  if (in.good()) {
    DASHER_ASSERT(in.gcount() == 1024-len);
    len = 1024;
  } else {
    len+=in.gcount();
    DASHER_ASSERT(len<1024);
    //next attempt to read more will fail.
  }
}

symbol CAlphabetMap::SymbolStream::next()
{
  int numChars;
  
  for (;;) {
    if (pos + m_utf8_count_array.max_length > len && len==1024) {
      //may need more bytes for next char; and input not yet exhausted.
      
      if (pos) {
        //shift remaining bytes to beginning
        len-=pos; //len of them
        memcpy(buf, &buf[pos], len);
        pos=0;
      }
      readMore();
    }
    //if still don't have any chars after attempting to read more...EOF!
    if (pos==len) return -1;
    numChars = m_utf8_count_array[buf[pos]];
    if (numChars != 0) break;
#ifdef DEBUG
    std::cerr << "Read invalid UTF-8 character 0x" << hex << buf[pos]
    << dec << std::endl;
#endif
    ++pos;
  }
  if (numChars == 1)
    return map.GetSingleChar(buf[pos++]);
  if (pos+numChars > len) {
    //no more bytes in file (would have tried to read earlier), but not enough for char
#ifdef DEBUG
    std::cerr << "Incomplete UTF-8 character beginning 0x" << hex << buf[pos] << dec;
    std::cerr << "(expecting " << numChars << " bytes but only " << (len-pos) << ")" << std::endl;
#endif
    pos=len;
    return -1;
  }
  int sym=map.Get(string(&buf[pos], numChars));
  pos+=numChars;
  return sym;
}

void CAlphabetMap::GetSymbols(std::vector<symbol>& Symbols, const std::string& Input) const
{
  std::istringstream in(Input);
  SymbolStream syms(*this, in);
  for (symbol sym; (sym=syms.next())!=-1;)
    Symbols.push_back(sym);
}


CAlphabetMap::CAlphabetMap(unsigned int InitialTableSize)
:HashTable(InitialTableSize <<1), Undefined(0) {
  Entries.reserve(InitialTableSize);

  const int numChars = numeric_limits<char>::max() + 1;
  m_pSingleChars = new symbol[numChars];
  for (int i = 0; i<numChars; i++) m_pSingleChars[i] = Undefined;
}

CAlphabetMap::~CAlphabetMap() {
  delete m_pSingleChars;
}

void CAlphabetMap::Add(const std::string &Key, symbol Value) {
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

symbol CAlphabetMap::Get(const std::string &Key) const {
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

symbol CAlphabetMap::GetSingleChar(char key) const {return m_pSingleChars[key];}
