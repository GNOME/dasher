// WinUTF8.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//                       (c) 2005 David Ward - unicode only now
/////////////////////////////////////////////////////////////////////////////

#ifndef __WinUTF8_h__
#define __WinUTF8_h__

/* Description {{{
	Functions in this namespace convert between wstring and UTF-8 formats.
	wstring is UTF-16, probably constrained to the BMP,
	such that all codepoints are represented in two bytes.
	
	These functions are just wrappers around Windows API calls or functions
	from the Unicode consortium (if using Windows 95 or CE, which do not support
	UTF-8 natively).
}}}*/

#include <string>

namespace WinUTF8 {
  void UTF8string_to_wstring(const std::string & UTF8string, std::wstring & Output);
  void wstring_to_UTF8string(const std::wstring & Input, std::string & Output);
}
#endif                          /* #ifndef __WinUTF8_h__ */
