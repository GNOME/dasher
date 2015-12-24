// WinUTF8.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "WinUTF8.h"

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG_MEMLEAKS
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/*

IAM 09/2002
Warning2: TODO: Error checking is non-existent. I am very naughty. This needs
sorting out.

*/

using std::string;
using std::wstring;

/////////////////////////////////////////////////////////////////////////////

void WinUTF8::UTF8string_to_wstring(const std::string &UTF8string, std::wstring &Output) {
  // Optimization - use the stack for small strings
  const std::size_t iStackBufferSize = 32;
  wchar_t StackBuffer[iStackBufferSize];
  wchar_t *pBuffer = StackBuffer;

  bool bNewed = false;

  const std::size_t BufferSize = UTF8string.size() + 1;
  if(BufferSize > iStackBufferSize) {
    pBuffer = new wchar_t[BufferSize];
    bNewed = true;
  }

  MultiByteToWideChar(CP_UTF8, 0, UTF8string.c_str(), -1, pBuffer, BufferSize);

  Output = pBuffer;
  if(bNewed)
    delete[]pBuffer;
  return;

}

std::wstring WinUTF8::UTF8string_to_wstring(const std::string &utf8string) {
  std::wstring res;
  UTF8string_to_wstring(utf8string,res);
  return res;
}

/////////////////////////////////////////////////////////////////////////////

void WinUTF8::wstring_to_UTF8string(const wstring &Input, string &Output) {
  wstring_to_UTF8string(Input.c_str(), Output);
}

string WinUTF8::wstring_to_UTF8string(const wchar_t *Input) {
  string res;
  wstring_to_UTF8string(Input, res);
  return res;
}

void WinUTF8::wstring_to_UTF8string(const wchar_t *Input, string &Output) {
  size_t len = wcslen(Input);
  auto size_needed = WideCharToMultiByte(CP_UTF8, 0, Input, len, nullptr, 0, nullptr, nullptr);
  Output.resize(size_needed);
  WideCharToMultiByte(CP_UTF8, 0, Input, (int)Output.size(), &Output[0], size_needed, NULL, NULL);
  return;
}
