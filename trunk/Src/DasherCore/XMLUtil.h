
// Some very simple XML utility functions that just function on STL strings.
//
// Good for recursively splitting up the job of reinstatiating objects
// that have been serialized to an XML file.
//
// Copyright 2005 by Keith Vertanen

#ifndef __XML_UTIL_H__
#define __XML_UTIL_H__

#include <cstdlib>
#include <string>
#include <vector>
#include <stdio.h>
#include "FileLogger.h"

extern CFileLogger* gLogger;

using namespace std;

#ifndef VECTOR_STRING
typedef vector<string> VECTOR_STRING;
#endif
#ifndef VECTOR_STRING_ITER
typedef vector<string>::iterator VECTOR_STRING_ITER;
#endif

// We want to be able grab all the name/value pairs in XML like:
//  <Params>
//      <Color>red</Color>
//      <Size>10</Size>
//  </Params>
struct NameValuePair
{
  string  strName;
  string  strValue;
};

typedef vector<NameValuePair>               VECTOR_NAME_VALUE_PAIR;
typedef vector<NameValuePair>::iterator     VECTOR_NAME_VALUE_PAIR_ITER;

const int XML_UTIL_READ_BUFFER_SIZE     = 4096;
const int XML_UTIL_DEFAULT_VECTOR_SIZE  = 128;
/// \ingroup Logging
/// @{
class XMLUtil
{
public:
  XMLUtil();

  static string			              LoadFile(const string& filename, unsigned int iSizeHint = 128000);
  static string			              GetElementString(const string& strTag, const string& strXML, bool bStripWhiteSpace = true);
  static int				              GetElementInt(const string& strTag, const string& strXML, bool* pFound = NULL);
  static int64		            GetElementLongLong(const string& strTag, const string& strXML, bool* pFound = NULL);
  static float			              GetElementFloat(const string& strTag, const string& strXML, bool* pFound = NULL);
  static VECTOR_STRING	          GetElementStrings(const string& strTag, const string& strXML, bool bStripWhiteSpace = true);
  static VECTOR_NAME_VALUE_PAIR   GetNameValuePairs(const string& strXML, bool bStripWhiteSpace = true);

  static bool				        IsWhiteSpace(char cLetter);
  static string			        StripWhiteSpace(const string& strText);
  static bool				        IsDigit(char cLetter);

};
/// @}

#endif

