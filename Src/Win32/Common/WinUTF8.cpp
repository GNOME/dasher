// WinUTF8.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "WinUTF8.h"
#include "WinHelper.h"

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

DJW 20050505
If CP_UTF8 is defined we use Windows native UTF-8 conversion.
Otherwise we include extra code from the Unicode consortium to do it for us.

IAM 09/2002
Warning2: TODO: Error checking is non-existent. I am very naughty. This needs
sorting out.

*/

using std::string;
using std::wstring;

/////////////////////////////////////////////////////////////////////////////

#ifndef CP_UTF8                 // MultiByteToWideChar(CP_UTF8,...) won't work
namespace {
#include "../../Common/Unicode/ConvertUTF.c"    // Yes I do mean to include .c
}
#endif /* #ifndef CP_UTF8 */

/*
ConversionResult ConvertUTF8toUTF16 (
		const UTF8** sourceStart, const UTF8* sourceEnd, 
		UTF16** targetStart, UTF16* targetEnd, ConversionFlags flags);

ConversionResult ConvertUTF16toUTF8 (
		const UTF16** sourceStart, const UTF16* sourceEnd, 
		UTF8** targetStart, UTF8* targetEnd, ConversionFlags flags);
*/

/////////////////////////////////////////////////////////////////////////////

void WinUTF8::UTF8string_to_wstring(const std::string &UTF8string, std::wstring &Output) {
  // Optimization - use the stack for small strings
  const int iStackBufferSize = 32;
  wchar_t StackBuffer[iStackBufferSize];
  wchar_t *pBuffer = StackBuffer;

  bool bNewed = false;

  const unsigned int BufferSize = UTF8string.size() + 1;
  if(BufferSize > iStackBufferSize) {
    pBuffer = new wchar_t[BufferSize];
    bNewed = true;
  }

#ifdef CP_UTF8
  MultiByteToWideChar(CP_UTF8, 0, UTF8string.c_str(), -1, pBuffer, BufferSize);
#else // Some OS's don't support this (Win95, WinCE if this code is ported there)
  const UTF8 *sourceStart = (const UTF8 *)UTF8string.c_str();   // yuck
  UTF16 *targetStart = pBuffer;
  ConvertUTF8toUTF16(&sourceStart, sourceStart + UTF8string.size(), &targetStart, targetStart + BufferSize - 1, lenientConversion);
  *targetStart = 0;             // Null-terminate the converted string
#endif

  Output = pBuffer;
  if(bNewed)
    delete[]pBuffer;
  return;

}

/////////////////////////////////////////////////////////////////////////////

void WinUTF8::wstring_to_UTF8string(const wstring &Input, string &Output) {

  const unsigned int BufferSize = Input.size() + 1;
  const wchar_t *Buffer = Input.c_str();

  const unsigned int BufferSize2 = BufferSize * 2;
  char *Buffer2 = new char[BufferSize2];

#ifdef CP_UTF8
  WideCharToMultiByte(CP_UTF8, 0, Buffer, -1, Buffer2, BufferSize2, NULL, NULL);
#else
  const UTF16 *sourceStart = Buffer;
  UTF8 *targetStart = (UTF8 *) Buffer2;
  ConvertUTF16toUTF8(&sourceStart, sourceStart + len, &targetStart, targetStart + BufferSize2 - 1, lenientConversion);
  *targetStart = 0;             // Null-terminate the converted string
#endif /* #ifdef CP_UTF8 */

  Output = Buffer2;
  delete[]Buffer2;
  return;
}
