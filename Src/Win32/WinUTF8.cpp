#include "WinUTF8.h"

/*
Warning "macro-magic" is working overtime. I don't really like to use macros,
but here it seems the neatest way to deal with portability to multiple Windows
platforms. If CP_UTF8 is defined we use Windows native UTF-8 conversion.
Otherwise we include extra code from the Unicode consortium to do it for us.
We also have to use macros to decide whether TCHAR is wide or not.

Warning2: TODO: Error checking is non-existent. I am very naughty. This needs
sorting out.

IAM 09/2002
*/

using std::string;

#ifndef CP_UTF8 // MultiByteToWideChar(CP_UTF8,...) won't work
namespace {
#include "../Common/Unicode/ConvertUTF.c" // Yes I do mean to include .c
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

void WinUTF8::UTF8string_to_Tstring(std::string UTF8string, Tstring* Output, UINT CodePage)
{
	// Quick check for trivial cases:
#ifndef _UNICODE
	if (CodePage==65001) { // didn't want an #ifdef CP_UTF8, just for that number, I'm sure the definition won't change :)
		*Output = UTF8string;
		return;
	}
#endif
	if (UTF8string=="") {
		*Output = TEXT("");
		return;
	}
	
	// Otherwise do conversion:
	const unsigned int BufferSize = UTF8string.size()+1;
	wchar_t* Buffer = new wchar_t[BufferSize];
#ifdef CP_UTF8
	MultiByteToWideChar(CP_UTF8, 0, UTF8string.c_str(), -1, Buffer, BufferSize);
#else // Some OS's don't support this (Win95, WinCE if this code is ported there)
	const UTF8* sourceStart = (const UTF8*) UTF8string.c_str(); // yuck
	UTF16* targetStart = Buffer;
	ConvertUTF8toUTF16 (&sourceStart, sourceStart+UTF8string.size(),
		&targetStart, targetStart+BufferSize-1, lenientConversion);
	*targetStart = 0; // Null-terminate the converted string
#endif
	
#ifdef _UNICODE
	*Output = Buffer;
	delete[] Buffer;
	return;
#else
	const unsigned int BufferSize2 = BufferSize*2;
	char* Buffer2 = new char[BufferSize2];
	WideCharToMultiByte(CodePage, 0, Buffer, -1, Buffer2, BufferSize2, NULL, NULL);
	delete[] Buffer;
	*Output = Buffer2;
	delete[] Buffer2;
	return;
#endif
}


void WinUTF8::Tstring_to_UTF8string(Tstring Input, std::string* Output, UINT CodePage)
{
	// Quick check for trivial cases:
#ifndef _UNICODE
	if (CodePage==65001) { // didn't want an #ifdef CP_UTF8, just for that number, I'm sure the definition won't change :)
		*Output = Input;
		return;
	}
#endif /* #ifndef _UNICODE */
	if (Input==TEXT("")) {
		*Output = "";
		return;
	}
	
	const unsigned int BufferSize = Input.size()+1;
#ifdef _UNICODE
	const wchar_t* Buffer = Input.c_str();
#else
	wchar_t* Buffer = new wchar_t[BufferSize];
	int len = MultiByteToWideChar(CodePage, 0, Input.c_str(), -1, Buffer, BufferSize);
#endif /* #ifdef _UNICODE */
	const unsigned int BufferSize2 = BufferSize*2;
	char* Buffer2 = new char[BufferSize2];
	
#ifdef CP_UTF8
	WideCharToMultiByte(CP_UTF8, 0, Buffer, -1, Buffer2, BufferSize2, NULL, NULL);
#else
	const UTF16* sourceStart = Buffer;
	UTF8* targetStart = (UTF8*) Buffer2;
	ConvertUTF16toUTF8 (&sourceStart, sourceStart+len, &targetStart, targetStart+BufferSize2-1, lenientConversion);
	*targetStart = 0; // Null-terminate the converted string
#endif /* #ifdef CP_UTF8 */
	
#ifndef _UNICODE
	delete[] Buffer;
#endif /* #ifndef _UNICODE */
	*Output = Buffer2;
	delete[] Buffer2;
	return;
}