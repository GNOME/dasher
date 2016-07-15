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
#include <cstring>

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

#define UNKNOWN_SYMBOL 0

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

CAlphabetMap::SymbolStream::SymbolStream(std::istream &_in, CMessageDisplay *pMsgs)
: pos(0), len(0), in(_in), m_pMsgs(pMsgs) {
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

inline int CAlphabetMap::SymbolStream::findNext() {
  for (;;) {
    if (pos + m_utf8_count_array.max_length > len) {
      //may need more bytes for next char
      if (pos) {
        //shift remaining bytes to beginning
        len-=pos; //len of them
        memmove(buf, &buf[pos], len);
        bytesRead(pos);
        pos=0;
      }
      //and look for more
      readMore();
    }
    //if still don't have any chars after attempting to read more...EOF!
    if (pos==len) return 0; //EOF
    if (int numChars = m_utf8_count_array[buf[pos]]) {
      if (pos+numChars > len) {
        //no more bytes in file (would have tried to read earlier), but not enough for char
        if (m_pMsgs) {
          const char *msg(_("File ends with incomplete UTF-8 character beginning 0x%x (expecting %i bytes but only %i)"));
          char *mbuf(new char[strlen(msg) + 4]);
          sprintf(mbuf, msg, static_cast<unsigned int>(buf[pos] & 0xff), numChars, len-pos);
          m_pMsgs->Message(mbuf,false);
          delete[] mbuf;
        }
        return 0;
      }
      return numChars;
    }
    if (m_pMsgs) {
      const char *msg(_("Read invalid UTF-8 character 0x%x"));
      char *mbuf(new char[strlen(msg) + 2]);
      sprintf(mbuf, msg, static_cast<unsigned int>(buf[pos] & 0xff));
      m_pMsgs->Message(mbuf,false);
      delete[] mbuf;
    }
    ++pos;
  }
}

string CAlphabetMap::SymbolStream::peekAhead() {
  int numChars=findNext();
  return string(&buf[pos],numChars);
}

string CAlphabetMap::SymbolStream::peekBack() {
  bool bSeenHighBit=false;
  for(int i=pos-1; i>=0; i--) {
    if (buf[i] & 0x80) {
      //multibyte character...
      bSeenHighBit=true;
      if (buf[i] & 0x40) {
        //START of multibyte character
        int numChars = m_utf8_count_array[buf[i]];
        if (i+numChars>pos) {
          //last (attempt to read a) symbol was an incomplete UTF8 character (!).
          // We'll have reported an error already when we saw it the first time, so for now just:
          return "";
        }
        DASHER_ASSERT(i+numChars==pos);
        return string(&buf[i],numChars);
      }
      //in middle of multibyte, keep going back...
    } else {
      //high bit not set -> single-byte char
      if (bSeenHighBit) return ""; //followed by a "continuation of multibyte char" without a "first byte of multibyte char" before it. (Malformed!)
      return string(&buf[i],1);
    }
  }
  //fail...relatively gracefully ;-)
  return "";
}

symbol CAlphabetMap::SymbolStream::next(const CAlphabetMap *map)
{
  int numChars=findNext();
  if (numChars==0) return -1; //EOF
  if (numChars == 1) {
    if (map->m_ParagraphSymbol!=UNKNOWN_SYMBOL && buf[pos]=='\r') {
      DASHER_ASSERT(pos+1<len || len<1024); //there are more characters (we should have read utf8...max_length), or else input is exhausted
      if (pos+1<len && buf[pos+1]=='\n') {
        pos+=2;
        return map->m_ParagraphSymbol;
      }
    }
    return map->GetSingleChar(buf[pos++]);
  }
  int sym=map->Get(string(&buf[pos], numChars));
  pos+=numChars;
  return sym;
}

void CAlphabetMap::GetSymbols(std::vector<symbol>& Symbols, const std::string& Input) const
{
  std::istringstream in(Input);
  SymbolStream syms(in);
  for (symbol sym; (sym=syms.next(this))!=-1;)
    Symbols.push_back(sym);
}


CAlphabetMap::CAlphabetMap(unsigned int InitialTableSize)
:HashTable(InitialTableSize <<1), m_ParagraphSymbol(UNKNOWN_SYMBOL) {
  Entries.reserve(InitialTableSize);
  // TODO: fix the code so it works if char is signed.
  const int numChars = numeric_limits<char>::max() + 1;
  m_pSingleChars = new symbol[numChars];
  for (int i = 0; i<numChars; i++) m_pSingleChars[i] = UNKNOWN_SYMBOL;
}

CAlphabetMap::~CAlphabetMap() {
  delete[] m_pSingleChars;
}

void CAlphabetMap::AddParagraphSymbol(symbol Value) {
  DASHER_ASSERT (m_ParagraphSymbol==UNKNOWN_SYMBOL);
  DASHER_ASSERT (m_pSingleChars['\r'] == UNKNOWN_SYMBOL);
  DASHER_ASSERT (m_pSingleChars['\n'] == UNKNOWN_SYMBOL);
  m_pSingleChars['\n'] = m_ParagraphSymbol = Value;
}

void CAlphabetMap::Add(const std::string &Key, symbol Value) {
  //Only single unicode-characters should be added...
  DASHER_ASSERT(m_utf8_count_array[Key[0]]==Key.length());
  if (Key.length() == 1) {
    DASHER_ASSERT(m_pSingleChars[Key[0]]==UNKNOWN_SYMBOL);
    DASHER_ASSERT(Key[0]!='\r' || m_ParagraphSymbol==UNKNOWN_SYMBOL);
    m_pSingleChars[Key[0]] = Value;
    return;
  }
  Entry *&HashEntry = HashTable[Hash(Key)];

  //Loop through Entries with the correct Hash value,
  // to check the key is not already present
  for(Entry * i = HashEntry; i; i = i->Next) {
    DASHER_ASSERT(i->Key != Key);
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
  if (m_ParagraphSymbol!=UNKNOWN_SYMBOL && Key=="\r\n")
    return m_ParagraphSymbol;
  DASHER_ASSERT(m_utf8_count_array[Key[0]]==Key.length());
  if (Key.length() == 1) {
	return GetSingleChar(Key[0]);
  }
  // Loop through Entries with the correct Hash value.
  for(Entry * i = HashTable[Hash(Key)]; i; i = i->Next) {
    if(i->Key == Key) {
      return i->Symbol;
    }
  }

  return UNKNOWN_SYMBOL;
}

symbol CAlphabetMap::GetSingleChar(char key) const {return m_pSingleChars[key];}
