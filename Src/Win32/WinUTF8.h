// WinUTF8.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////



#ifndef __WinUTF8_h__
#define __WinUTF8_h__

#include "WinHelper.h"

/* Description {{{
	Functions in this namespace convert between Tstring and UTF-8 formats.
	If _UNICODE is defined Tstring is UTF-16, probably constrained to the BMP,
	such that all codepoints are represented in two bytes. If _UNICODE is not
	defined then a Tstring is an 8bit character format, encoded with some
	code page. If we do not specify a code page, UTF-8 is assumed and the
	conversion does nothing.
	
	These functions are just wrappers around Windows API calls or functions
	from the Unicode consortium (if using Windows 95 or CE, which do not support
	UTF-8 natively).
}}}*/

namespace WinUTF8
{
#ifdef CP_UTF8
	void UTF8string_to_Tstring(std::string UTF8string, Tstring* Output, UINT CodePage=CP_UTF8);
	void Tstring_to_UTF8string(Tstring Input, std::string* Output, UINT CodePage=CP_UTF8);
#else
	void UTF8string_to_Tstring(std::string UTF8string, Tstring* Output, UINT CodePage=65001);
	void Tstring_to_UTF8string(Tstring Input, std::string* Output, UINT CodePage=65001);
#endif
}


#endif /* #ifndef __WinUTF8_h__ */