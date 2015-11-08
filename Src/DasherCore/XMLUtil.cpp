
#include "../Common/Common.h"

#include <cstring>

#include "XMLUtil.h"

#include <sys/types.h>
#include <sys/stat.h>

// Track memory leaks on Windows to the line that new'd the memory
#ifdef _WIN32
#ifdef _DEBUG
#define DEBUG_NEW new( _NORMAL_BLOCK, THIS_FILE, __LINE__ )
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

bool XMLUtil::IsWhiteSpace(char cLetter)
{
  if ((cLetter == ' ') ||
      (cLetter == '\n') ||
      (cLetter == '\r') ||
      (cLetter == '\t'))
    return true;

  return false;
}

// See if a character is 0 - 9
bool XMLUtil::IsDigit(char cLetter)
{
  if ((cLetter >= '0') && (cLetter <= '9'))
    return true;
  return false;
}

// Strip the leading and trailing white space off a string.
string XMLUtil::StripWhiteSpace(const string& strText)
{
  string strResult = "";

  strResult.reserve(strText.length());

  int iStart = 0;
  while ((iStart < (int) strText.length()) && (IsWhiteSpace(strText[iStart])))
    iStart++;

  int iEnd = strText.length() - 1;
  while ((iEnd > 0) && (IsWhiteSpace(strText[iEnd])))
    iEnd--;

  strResult = strText.substr(iStart, iEnd - iStart + 1);

  return strResult;
}

// Return a string containing the contents of a file
string XMLUtil::LoadFile(const string& strFilename, unsigned int iSizeHint)
{
  string strResult = "";

  char szBuffer[XML_UTIL_READ_BUFFER_SIZE];
  FILE* fp = NULL;
  fp = fopen(strFilename.c_str(), "r");
  if (fp != NULL)
  {
#ifdef _WIN32
    struct __stat64 buf;
    int result;
    result = _stat64(strFilename.c_str(), &buf);
    strResult.reserve((unsigned long) buf.st_size + 256);
#else
    // On unix, we default to 128,000 bytes or whatever the caller passed in as a hint
    strResult.reserve(iSizeHint);
#endif

    while (!feof(fp))
    {
      memset(szBuffer, 0, XML_UTIL_READ_BUFFER_SIZE);
      fread(szBuffer, 1, XML_UTIL_READ_BUFFER_SIZE - 1, fp);
      strResult += szBuffer;
    }

    fclose(fp);
    fp = NULL;
  }

  return strResult;
}


// Returns what is between the given tag in the passed XML.  We only return the first matching
// tag if there are multiple in the XML.  Tags are case sensitive.
string XMLUtil::GetElementString(const string& strTag, const string& strXML, bool bStripWhiteSpace)
{
  string strResult = "";
  string strStart = "";
  string strEnd = "";

  strStart += "<";
  strStart += strTag;
  strStart += ">";

  strEnd += "</";
  strEnd += strTag;
  strEnd += ">";

  int iPosStart = strXML.find(strStart);
  int iPosEnd = strXML.find(strEnd);

  if ((iPosStart != -1) && (iPosEnd != -1))
  {
    strResult = strXML.substr(iPosStart + strStart.length(), iPosEnd - (iPosStart + strStart.length()));
  }

  if (bStripWhiteSpace)
    strResult = StripWhiteSpace(strResult);

  return strResult;
}

// Return the integer representing an element
int XMLUtil::GetElementInt(const string& strTag, const string& strXML, bool* pFound)
{
  string strElement = GetElementString(strTag, strXML);

  unsigned int i = 0;
  for (i = 0; i < strElement.size(); i++)
  {
    if (i == 0)
    {
      if ((!IsDigit(strElement[i])) && ((strElement[i] != '-')))
        break;
    }
    else
      if (!IsDigit(strElement[i]))
        break;
  }

  // Only try and convert something that is all digits
  if (i == strElement.size())
  {
    if (pFound != NULL)
      *pFound = true;
    return atoi(strElement.c_str());
  }
  else
  {
    if (pFound != NULL)
      *pFound = false;
    return -1;
  }
}

int64 XMLUtil::GetElementLongLong(const string& strTag, const string& strXML, bool* pFound)
{
  string strElement = GetElementString(strTag, strXML);

  unsigned int i = 0;
  for (i = 0; i < strElement.size(); i++)
  {
    if (i == 0)
    {
      if ((!IsDigit(strElement[i])) && ((strElement[i] != '-')))
        break;
    }
    else
      if (!IsDigit(strElement[i]))
        break;
  }

  // Only try and convert something that is all digits
  if ((i > 0) && (i == strElement.size()))
  {
    if (pFound != NULL)
      *pFound = true;
#ifdef _WIN32
    return _atoi64(strElement.c_str());
#else
    return atoll(strElement.c_str());
#endif
  }
  else
  {
    if (pFound != NULL)
      *pFound = false;
    return -1;
  }
}

// Optionally can pass back a bool that tell us if the tag was found
float XMLUtil::GetElementFloat(const string& strTag, const string& strXML, bool* pFound)
{
  string strElement = GetElementString(strTag, strXML);

  bool bFoundDot = false;

  unsigned int i = 0;
  for (i = 0; i < strElement.size(); i++)
  {
    if (i == 0)
    {
      if ((!IsDigit(strElement[i])) && ((strElement[i] != '-')))
        break;
    }
    else
    {
      if (!IsDigit(strElement[i]))
      {
        if ((strElement[i] == '.') && (!bFoundDot))
          bFoundDot = true;
        else
          break;
      }
    }
  }

  // Only try and convert something that is all digits
  if ((i > 0) && (i == strElement.size()))
  {
    if (pFound != NULL)
      *pFound = true;
    return (float) atof(strElement.c_str());
  }
  else
  {
    if (pFound != NULL)
      *pFound = false;
    return (float) 0.0;
  }
}

// Return a vector containing all the text inside all tags matching the passed one
VECTOR_STRING XMLUtil::GetElementStrings(const string& strTag, const string& strXML, bool bStripWhiteSpace)
{
  VECTOR_STRING vResult;
  vResult.reserve(XML_UTIL_DEFAULT_VECTOR_SIZE);

  string strStart = "";
  string strEnd = "";
  string strResult = "";

  strStart += "<";
  strStart += strTag;
  strStart += ">";

  strEnd += "</";
  strEnd += strTag;
  strEnd += ">";

  size_t iPosStart        = strXML.find(strStart);
  size_t iPosEnd          = strXML.find(strEnd);

  while ((iPosStart != string::npos) && (iPosEnd != string::npos))
  {
    // We want to be able to handle having the same tag emedded in itself.
    // So between the start tag and the first instance of the end tag,
    // we'll count any other instances of the start tag.  If we find some
    // then we require that we continue until we get that number more of
    // close tags.
    size_t iCurrentStart    = iPosStart + strStart.length();
    size_t iEmbedCount      = 0;
    while ((iCurrentStart != string::npos) && (iCurrentStart < iPosEnd))
    {
      iCurrentStart = strXML.find(strStart, iCurrentStart);
      if ((iCurrentStart != string::npos) && (iCurrentStart < iPosEnd))
      {
        iEmbedCount++;
        iCurrentStart += strStart.length();
      }
    }
    // Now look for end tag to balance the start tags
    for (size_t i = 0; i < iEmbedCount; i++)
    {
      iPosEnd = strXML.find(strEnd, iPosEnd  + strEnd.length());

      // Check to make sure we're still matching tags
      if (iPosEnd == string::npos)
        break;
    }

    strResult = strXML.substr(iPosStart + strStart.length(), iPosEnd - (iPosStart + strStart.length()));

    if (bStripWhiteSpace)
      strResult = StripWhiteSpace(strResult);

    iPosStart = strXML.find(strStart, iPosEnd + strEnd.length());

    if (iPosStart != string::npos)
      iPosEnd = strXML.find(strEnd, iPosStart);

    vResult.push_back(strResult);
  }

  return vResult;
}

VECTOR_NAME_VALUE_PAIR XMLUtil::GetNameValuePairs(const string& strXML, bool bStripWhiteSpace)
{
  VECTOR_NAME_VALUE_PAIR vResult;
  vResult.reserve(XML_UTIL_DEFAULT_VECTOR_SIZE);

  bool    bInStartTag = false;
  string  strName     = "";
  string  strValue    = "";

  size_t i = 0;
  while (i < strXML.length())
  {
    if ((!bInStartTag) && (strXML[i] == '<'))
    {
      // Starting a new tag
      bInStartTag = true;
    }
    else if (bInStartTag)
    {
      if (strXML[i] == '>')
      {
        // Hit the end of the start tag, get everything
        // until we find the end tag.

        string strFind = "</";
        strFind += strName;
        strFind += ">";

        size_t iPos = string::npos;
        iPos = strXML.find(strFind, i);

        if (iPos != string::npos)
        {
          strValue = strXML.substr(i + 1, iPos - i - 1);

          NameValuePair sPair;
          sPair.strName    = strName;
          sPair.strValue   = strValue;

          vResult.push_back(sPair);

          i = iPos + strFind.length();

        }
        else
          break;

        bInStartTag     = false;
        strName         = "";
        strValue        = "";
      }
      else
        strName += strXML[i];
    }

    i++;
  }

  return vResult;
}
