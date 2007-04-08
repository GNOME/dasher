// KanjiConversionIME.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2005 Takashi Kaburagi
//
/////////////////////////////////////////////////////////////////////////////

#include "../../Common/Common.h"

#include "KanjiConversionIME.h"

#include <iostream>             //For testing 23 June 2005

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

CKanjiConversionIME::CKanjiConversionIME() {
  IsInit = 0;
  hIMC = ImmCreateContext();
  IsInit = 1;
}

CKanjiConversionIME::~CKanjiConversionIME() {
  ImmDestroyContext( hIMC );
  IsInit = 0;
}

int CKanjiConversionIME::ConvertKanji(std::string str) {
  HKL hKL;
  DWORD dwSize;
  LPCANDIDATELIST lpCand;

  hKL = GetKeyboardLayout(0);
	char *pQuery = (char *)str.c_str();
	while( strlen(pQuery) ){
		dwSize = ImmGetConversionList(hKL, hIMC, (LPCWSTR)pQuery, NULL, 0, GCL_CONVERSION);
		lpCand = (LPCANDIDATELIST)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
		dwSize = ImmGetConversionList(hKL, hIMC, (LPCWSTR)pQuery, lpCand, dwSize, GCL_CONVERSION);
		size_t MaxLen = 0;
		CPhrase new_phrase;
		//	For all candidates
		for (unsigned int i = 0; i< lpCand->dwCount; i++)
		{
		  new_phrase.candidate_list.push_back( (char *)(lpCand + lpCand->dwOffset[i]) );
			//sprintf( buf, "%s\n",(LPBYTE)lpCand + lpCand->dwOffset[i] );
			// Find hiragana length
			if( strlen( (char *)lpCand + lpCand->dwOffset[i] ) > MaxLen )
				MaxLen = strlen( (char *)lpCand + lpCand->dwOffset[i] );
		}
		phrase.push_back(new_phrase);
		HeapFree(GetProcessHeap(), 0, lpCand);
		pQuery += MaxLen;
	}
  return 0;
}
