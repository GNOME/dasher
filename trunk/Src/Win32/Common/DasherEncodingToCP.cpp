// DasherEncodingToCP.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "DasherEncodingToCP.h"
#include "WinLocalisation.h"
using WinLocalisation::GetUserCodePage;

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

UINT EncodingToCP(Dasher::Opts::AlphabetTypes Encoding) {
  using namespace Dasher;
  using namespace Opts;

  UINT CodePage;

  switch (Encoding) {
/*
	case UTF8:                         // Just return Unicode Codepages
		return 65001;
		break;
	case UTF16LE:
		return 1200;
		break;
	case UTF16BE:
		return 1201;
		break;
*/
  case MyNone:
    CodePage = GetUserCodePage();
    break;
  case Arabic:                 // The rest need checking. Return
    CodePage = 1256;            // user codepage if not installed.
    break;
  case Baltic:
    CodePage = 1257;
    break;
  case CentralEurope:
    CodePage = 1250;
    break;
  case ChineseSimplified:
    CodePage = 936;
    break;
  case ChineseTraditional:
    CodePage = 950;
    break;
  case Cyrillic:
    CodePage = 1251;
    break;
  case Greek:
    CodePage = 1253;
    break;
  case Hebrew:
    CodePage = 1255;
    break;
  case Japanese:
    CodePage = 932;
    break;
  case Korean:
    CodePage = 949;
    break;
  case Thai:
    CodePage = 874;
    break;
  case Turkish:
    CodePage = 1254;
    break;
  case VietNam:
    CodePage = 1258;
    break;
  case Western:
    CodePage = 1252;
    break;
  default:
    CodePage = GetUserCodePage();
    break;
  }

  if(!IsValidCodePage(CodePage))
    CodePage = GetUserCodePage();

  return CodePage;
}
