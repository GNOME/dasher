
#include "XMLUtil.h"

bool XMLUtil::IsWhiteSpace(char ch)
{
	if ((ch == ' ') ||
		(ch == '\n') ||
		(ch == '\r') ||
		(ch == '\t'))
		return true;
	
	return false;
}

// See if a character is 0 - 9
bool XMLUtil::IsDigit(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return true;
	return false;
}

// Strip the leading and trailing white space off a string.
string XMLUtil::StripWhiteSpace(const string& str)
{
	string strResult = "";

	strResult.reserve(str.length());

	int start = 0;
	while ((start < (int) str.length()) && (IsWhiteSpace(str[start])))
		start++;

	int end = str.length() - 1;
	while ((end > 0) && (IsWhiteSpace(str[end])))
		end--;

	strResult = str.substr(start, end - start + 1);

	return strResult;
}

// Return a string containing the contents of a file
string XMLUtil::LoadFile(const string& filename, unsigned int sizeHint)
{
	string strResult = "";

#ifdef _WIN32
	struct __stat64 buf;
	int result;		
	result = _stat64(filename.c_str(), &buf);
	strResult.reserve((unsigned long) buf.st_size + 256);
#else
	// On unix, we default to 128,000 bytes or whatever the caller passed in as a hint
	strResult.reserve(sizeHint);
#endif

	char strBuffer[XML_UTIL_READ_BUFFER_SIZE];
	FILE* fp = NULL;
	fp = fopen(filename.c_str(), "r");
	if (fp != NULL)
	{
		while (!feof(fp))
		{
			memset(strBuffer, 0, XML_UTIL_READ_BUFFER_SIZE);
			fread(strBuffer, 1, XML_UTIL_READ_BUFFER_SIZE - 1, fp);
			strResult += strBuffer;
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

	int posStart = strXML.find(strStart);
	int posEnd = strXML.find(strEnd);

	if ((posStart != -1) && (posEnd != -1))
	{
		strResult = strXML.substr(posStart + strStart.length(), posEnd - (posStart + strStart.length()));
	}
	
	if (bStripWhiteSpace)
		strResult = StripWhiteSpace(strResult);

	return strResult;
}

// Return the integer representing an element
int XMLUtil::GetElementInt(const string& strTag, const string& strXML, bool* bFound)
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
        if (bFound != NULL)
            *bFound = true;
		return atoi(strElement.c_str());
    }
	else
    {
        if (bFound != NULL)
            *bFound = false;
		return -1;
    }
}

long long XMLUtil::GetElementLongLong(const string& strTag, const string& strXML, bool* bFound)
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
        if (bFound != NULL)
            *bFound = true;
		#ifdef _WIN32
			return _atoi64(strElement.c_str());
		#else
			return atoll(strElement.c_str());
		#endif
	}
	else
    {
        if (bFound != NULL)
            *bFound = false;
        return -1;
    }
}

// Optionally can pass back a bool that tell us if the tag was found
float XMLUtil::GetElementFloat(const string& strTag, const string& strXML, bool* bFound)
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
        if (bFound != NULL)
            *bFound = true;
		return (float) atof(strElement.c_str());
    }
	else
    {
        if (bFound != NULL)
            *bFound = false;
        return (float) 0.0;
    }
}

// Return a vector containing all the text inside all tags matching the passed one
VECTOR_STRING XMLUtil::GetElementStrings(const string& strTag, const string& strXML, bool bStripWhiteSpace)
{
	VECTOR_STRING vectorResult;
	vectorResult.reserve(XML_UTIL_DEFAULT_VECTOR_SIZE);

	string strStart = "";
	string strEnd = "";
	string strResult = "";

	strStart += "<";
	strStart += strTag;
	strStart += ">";

	strEnd += "</";
	strEnd += strTag;
	strEnd += ">";

	int posStart = strXML.find(strStart);
	int posEnd = strXML.find(strEnd);

	while ((posStart != -1) && (posEnd != -1))
	{
		strResult = strXML.substr(posStart + strStart.length(), posEnd - (posStart + strStart.length()));
	
		if (bStripWhiteSpace)
			strResult = StripWhiteSpace(strResult);

		posStart = strXML.find(strStart, posEnd + strEnd.length());

		if (posStart != -1)
			posEnd = strXML.find(strEnd, posStart);

		vectorResult.push_back(strResult);
	}

	return vectorResult;
}

VECTOR_NAME_VALUE_PAIR XMLUtil::GetNameValuePairs(const string& strXML, bool bStripWhiteSpace)
{
    VECTOR_NAME_VALUE_PAIR vectorResult;
	vectorResult.reserve(XML_UTIL_DEFAULT_VECTOR_SIZE);

    bool    bInStartTag = false;
    string  strName     = "";
    string  strValue    = "";

    unsigned int i = 0;
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

                int pos = -1;
                pos = strXML.find(strFind, i);

                if (pos != -1)
                {
                    strValue = strXML.substr(i + 1, pos - i - 1);

                    NameValuePair pair;
                    pair.strName    = strName;
                    pair.strValue   = strValue;

                    vectorResult.push_back(pair);
                    
                    i = pos + strFind.length();

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

    return vectorResult;
}



