// Various little utility function can go in here
//
// Copyright 2004 by Keith Vertanen

#ifndef __UTIL_H__
#define __UTIL_H__

#include <string>
#include <vector>
#include <time.h>

#ifdef _WIN32
	#include <windows.h>
#endif

using namespace std;

typedef vector<string> VECTOR_STRING;

const int MAX_PATH_LENGTH = 1024;

static string makeLower(const string& strA);

static bool isWhiteSpace(char ch)
{
	if ((ch == ' ') ||
		(ch == '\n') ||
		(ch == '\r') ||
		(ch == '\t'))
		return true;
	
	return false;
}

// Splits a line of words up into each of its tokens
static VECTOR_STRING splitLine(const string& str)
{
	VECTOR_STRING vectorResult;
	bool bInWord = false;
	unsigned int i = 0;
	string strWord = "";

	while (i < str.length())
	{
		if (isWhiteSpace(str[i]))
		{
			// See if we have a word to add
			if (strWord.length() > 0)
				vectorResult.push_back(strWord);
			strWord = "";
			bInWord = false;
		}
		else
		{
			strWord += str[i];
			bInWord = true;
		}
		i++;
	}
	if (strWord.length() > 0)
		vectorResult.push_back(strWord);

	return vectorResult;
}

// Splits a line of words up into each of its tokens
// This version takes in a vector pointer to avoid copying expense.
static void splitLine(const string& str, VECTOR_STRING* vectorResult, bool bMakeLower = false)
{
	if (vectorResult == NULL)
		return;
	
//	vectorResult->clear();
	vectorResult->erase(vectorResult->begin(), vectorResult->end());

	bool bInWord = false;
	unsigned int i = 0;
	string strWord = "";

	while (i < str.length())
	{
		if (isWhiteSpace(str[i]))
		{
			// See if we have a word to add
			if (strWord.length() > 0)
			{
				if (bMakeLower)
					vectorResult->push_back(makeLower(strWord));
				else
					vectorResult->push_back(strWord);
			}
			strWord = "";
			bInWord = false;
		}
		else
		{
			strWord += str[i];
			bInWord = true;
		}
		i++;
	}
	if (strWord.length() > 0)
	{
		if (bMakeLower)
			vectorResult->push_back(makeLower(strWord));
		else
			vectorResult->push_back(strWord);
	}
}

// Case insensitive comparison of two STL stirngs
static long compareNoCase(const string& strA, const string& strB)
{
	string strLowerA = "";
	string strLowerB = "";

	unsigned int i = 0;

	for (i = 0; i < strA.length(); i++)
	{
	  strLowerA += tolower(strA[i]);
	}

	for (i = 0; i < strB.length(); i++)
	{
	  strLowerB += tolower(strB[i]);
	}

	return strLowerA.compare(strLowerB);
}

// Change one character in a string to some other string sequence
static string convertChar(const string& strA, const char ch, const string& strReplace)
{
	string strResult = "";
	unsigned int i = 0;

	for (i = 0; i < strA.length(); i++)
	{
		if (strA[i] == ch)
			strResult += strReplace;
		else
			strResult += strA[i];
	}

	return strResult;
}


// Change a STL string to lower case
static string makeLower(const string& strA)
{
	string strLowerA = "";
	unsigned int i = 0;

	for (i = 0; i < strA.length(); i++)
	{
	  strLowerA += tolower(strA[i]);
	}

	return strLowerA;
}

// Change a STL string to upper case
static string makeUpper(const string& strA)
{
	string strUpperA = "";
	unsigned int i = 0;

	for (i = 0; i < strA.length(); i++)
	{
	  strUpperA += toupper(strA[i]);
	}

	return strUpperA;
}

// Returns a random integer from [low, high]
static long getRandomInt(int low, int high)
{
	return low + rand() % (high - low + 1);
}

static float getRandomFloat(float low, float high)
{
	return (rand() / (float) RAND_MAX) * ((float) high - (float) low);
}

static string getDateStamp()
{
	string strResult = "";
	char strBuffer[512];

	struct tm *newtime;
	time_t aclock;

	time( &aclock );   // Get time in seconds
	newtime = localtime( &aclock );   // Convert time to struct tm form

	strftime(strBuffer, 512, "%x", newtime);

	strResult = strBuffer;
	return strResult;
}

static string getTimeStamp()
{
	string strTimeStamp = "";
	struct timeb timebuffer;
	char* timeline = NULL;

	ftime( &timebuffer );

	timeline = ctime( & ( timebuffer.time ) );

	if ((timeline != NULL) && (strlen(timeline) > 18))
	{
		for (int i = 11; i < 19; i++)
			strTimeStamp += timeline[i];
		strTimeStamp += ".";
		char strMs[16];
		sprintf(strMs, "%d", timebuffer.millitm);
		if (strlen(strMs) == 1)
			strTimeStamp += "00";
		else if (strlen(strMs) == 2)
			strTimeStamp += "0";
		strTimeStamp += strMs;
	}

	return strTimeStamp;
}

static string getDateTimeStamp()
{
	string strResult = "";
	
	strResult += getDateStamp();
	strResult += " ";
	strResult += getTimeStamp();

	return strResult;
}


// Return everything but the extension from a filename
static string getBaseFilename(const string& filename)
{
	string strResult = "";

	int i = filename.size() - 1;
	while (i >= 0)
	{
		if (filename[i] == '.')
			break;
		i--;
	}
	
	if (i >=0 )
		strResult = filename.substr(0, i);
	else
		strResult = filename;
	return strResult;
}

// Recover the full path of a filename
static string getFullPath(const string& filename)
{
#ifdef _WIN32
	// Don't know how to make this work for unicode
#ifndef _UNICODE
	char		szPath[MAX_PATH_LENGTH];
    char*		pszFilePart = NULL;

	// Recover the fully qualified path of the filename we are working on
	GetFullPathName(filename.c_str(), MAX_PATH_LENGTH, szPath, &pszFilePart);
	string strResult = "";
	strResult = szPath;
	return strResult;
#else
	return filename;
#endif
#else
	return filename;
#endif

}

#ifdef _WIN32
static void UTF8string_to_wstring(const std::string& UTF8string, std::wstring& Output)
{
	// Optimization - use the stack for small strings
	const int iStackBufferSize = 32;
	wchar_t StackBuffer[iStackBufferSize];
	wchar_t* pBuffer = StackBuffer;

	bool bNewed = false;

	const unsigned int BufferSize = UTF8string.size()+1;
	if ( BufferSize > iStackBufferSize)
	{
		pBuffer = new wchar_t[BufferSize];
		bNewed = true;
	}

#ifdef CP_UTF8
	MultiByteToWideChar(CP_UTF8, 0, UTF8string.c_str(), -1, pBuffer, BufferSize);
#else // Some OS's don't support this (Win95, WinCE if this code is ported there)
	const UTF8* sourceStart = (const UTF8*) UTF8string.c_str(); // yuck
	UTF16* targetStart = pBuffer;
	ConvertUTF8toUTF16 (&sourceStart, sourceStart+UTF8string.size(),
		&targetStart, targetStart+BufferSize-1, lenientConversion);
	*targetStart = 0; // Null-terminate the converted string
#endif

	Output = pBuffer;
	if (bNewed)
		delete[] pBuffer;
	return;
}
#endif

#endif

