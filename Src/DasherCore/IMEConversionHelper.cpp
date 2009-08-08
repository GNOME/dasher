#ifdef JAPANESE

#include "../WIn32/Common/WinUTF8.h"
#include "IMEConversionHelper.h"

#define BUFSIZE 10240

#include <iostream>             //For testing 23 June 2005

CIMEConversionHelper::CIMEConversionHelper(Dasher::CNodeCreationManager *pNCManager, Dasher::CAlphabet *pAlphabet)
 : CConversionHelper(pNCManager, pAlphabet) {
  IsInit = 0;
  hIMC = ImmCreateContext();
  IsInit = 1;
}

CIMEConversionHelper::~CIMEConversionHelper() {
  ImmDestroyContext( hIMC );
  IsInit = 0;
 }

bool CIMEConversionHelper::Convert(const std::string &strSource, std::vector<std::vector<std::string> > &vResult) {

  std::wstring strInput;
  WinUTF8::UTF8string_to_wstring(strSource, strInput);

  HKL hKL;
  DWORD dwSize;
  LPCANDIDATELIST lpCand;

  hKL = GetKeyboardLayout(0);
	const WCHAR *pQuery = strInput.c_str();
	//while( wcslen(pQuery) ){
		dwSize = ImmGetConversionList(hKL, hIMC, (LPCWSTR)pQuery, NULL, 0, GCL_CONVERSION);
    if(dwSize > 0) {
		  lpCand = (LPCANDIDATELIST)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
		  dwSize = ImmGetConversionList(hKL, hIMC, (LPCWSTR)pQuery, lpCand, dwSize, GCL_CONVERSION);
		  size_t MaxLen = 0;
      std::vector<std::string> new_phrase;
		  //	For all candidates
		  for (unsigned int i = 0; i< lpCand->dwCount; i++)
		  {
        std::wstring strOutput((WCHAR *)((char *)lpCand + lpCand->dwOffset[i]));
        std::string strUTF8Output;
        WinUTF8::wstring_to_UTF8string(strOutput, strUTF8Output);
		    new_phrase.push_back(strUTF8Output);
			  //sprintf( buf, "%s\n",(LPBYTE)lpCand + lpCand->dwOffset[i] );
			  // Find hiragana length

        MaxLen = strOutput.size();

			  /*if( wcslen( (WCHAR *)lpCand + lpCand->dwOffset[i] ) > MaxLen )
  				MaxLen = wcslen( (WCHAR *)lpCand + lpCand->dwOffset[i] );*/
		  }
		  vResult.push_back(new_phrase);
		  HeapFree(GetProcessHeap(), 0, lpCand);
		  pQuery += MaxLen;
    }
    else {
    //  break;
    }
	//}

  return true;
}

#endif
