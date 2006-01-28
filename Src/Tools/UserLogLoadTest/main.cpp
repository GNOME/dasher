// Command line application that just hammers the user logging
// related objects.
//
// Copyright 2005 by Keith Vertanen
//

#include "../Common/Common.h"

#include <string>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#endif

// Declare our global file logging object (declared in DasherInterfaceBase.cpp)
#include "FileLogger.h"
extern CFileLogger* g_pLogger;

#include "../../DasherCore/Alphabet/Alphabet.h"
#include "../../DasherCore/Alphabet/AlphIO.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>

#include "UserLog.h"
#include "Utils.h"

#ifdef _WIN32
// In order to track leaks to line number, we need this at the top of every file
#include "MemoryLeak.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

using namespace std;

#ifndef VECTOR_STRING
typedef vector<string>            VECTOR_STRING;
#endif

int     GetRandomInt(int lower, int upper);
string  GetRandomString(int lowerLength, int upperLength);

int GetRandomInt(int lower, int upper)
{
  return rand() % (upper - lower + 1) + lower;
}

float GetRandomFloat(int lower, int upper)
{
  return ((float) rand() / (float) RAND_MAX) * (float) (upper - lower) + (float) lower;
}

int GetRandomBitMask(int bits)
{
  int result = 0;
  int power = 1;
  for (int i = 0; i < bits; i++)
  {
    if (rand() % 2 == 0)
      result += power;

    power = power * 2;            
  }    
  return result;
}

string GetRandomString(int lowerLength, int upperLength)
{
  string strResult = "";
  int len = GetRandomInt(lowerLength, upperLength);

  for (int i = 0; i < len; i++)
    strResult += (char) GetRandomInt(65, 90);

  return strResult;
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
#ifdef _DEBUG
  // Windows debug build memory leak detection
  EnableLeakDetection();
#endif
#endif

  // Normally this is created in DasherInterfaceBase, but we aren't instantiating 
  // that class for this test harness program.
  g_pLogger = new CFileLogger("UserLogLoadTest.log", logDEBUG, logTimeStamp | logDateStamp | logDeleteOldFile);

  { // memory leak scoping

    // Set the defaults for the switched parameters
    string	strAlphabetFilename		= "alphabet.english.xml";
    string	strAlphabetName			= "English alphabet - limited punctuation";

    /*
    if (argc <= 2)
    {
    cout << "UserLogLoadTest" << endl;
    cout << "  -alphabet       <alphabet XML>     [" << strAlphabetFilename << "]" << endl;
    cout << "  -alphabetName   <alphabet name>    [" << strAlphabetName << "]" << endl;

    cout << endl;
    return 0;
    }
    */

    // Parse the command line options
    int i = 1;
    while (i < argc)
    {
      string strParam = makeLower(argv[i]);
      string strNextParam = "";
      if (i < argc - 1)
        strNextParam = argv[i+1];

      if (strParam.compare("-alphabet") == 0)
      {
        strAlphabetFilename = strNextParam;
        i++;
      }
      else if (strParam.compare("-alphabetname") == 0)
      {
        strAlphabetName = strNextParam;
        i++;
      }
      else
      {
        // Failure to parse this parameter
        cout << "Unknown switch: " << strParam << endl;
      }

      i++;
    }
#ifdef _WIN32
    ::SetPriorityClass(GetCurrentThread(), BELOW_NORMAL_PRIORITY_CLASS);
#endif

    // We need an alphabet object in order to create the UserLog object

    // Load our alphabet from the specified XML file
    VECTOR_STRING vectorFilenames;
    vectorFilenames.push_back(strAlphabetFilename);	
    Dasher::CAlphIO* pAlphIO = new Dasher::CAlphIO("", "", vectorFilenames);
    if (pAlphIO == NULL)
    {
      g_pLogger->Log("Failed to create alphIO from '%s'", logNORMAL, strAlphabetFilename.c_str());
      return 0;
    }

    Dasher::CAlphabet* pAlphabet = new Dasher::CAlphabet(pAlphIO->GetInfo(strAlphabetName));	
    if (pAlphabet == NULL)
    {
      g_pLogger->Log("Failed to create alphabet from '%s'", logNORMAL, strAlphabetName.c_str());
      return 0;
    }

    srand((unsigned) time(NULL));

//    CDasherInterface*   pDasherInterface = NULL;
    CUserLog*           pUserLog = NULL;

    unsigned long count = 0;
    VECTOR_STRING vectorParams;
    bool bWriting = false;

#ifdef _WIN32
    cout << "Hit 'q' to quit.\n";
#endif

    while (1)
    {            
      count++;

      if (pUserLog == NULL)
      {

//        pDasherInterface  = new CDasherInterface();
//        pUserLog          = pDasherInterface->GetUserLogPtr();

        pUserLog = new CUserLog(NULL, NULL, GetRandomInt(1, 3), pAlphabet);

        // Set a random user log level for use next time we create DasherInterface
//        pDasherInterface->SetLongParameter(LP_USER_LOG_LEVEL_MASK, GetRandomInt(1, 3));
        
//        pDasherInterface->ChangeAlphabet(pDasherInterface->GetStringParameter(SP_ALPHABET_ID)); 

        // We need to create a new user log object                
//        pUserLog = new CUserLog(GetRandomInt(1, 3), pAlphabet);
        pUserLog->SetOuputFilename("loadtest.xml");
      }
      else
      {
        // Randomly choose something to do
        int action = GetRandomInt(0, 16);

        switch (action)
        {   
        case 0:
          {
            // Output our file and delete the object
            pUserLog->OutputFile();

  //          delete pDasherInterface;
//            pDasherInterface = NULL;
            delete pUserLog;

            pUserLog = NULL;
            vectorParams.clear();
            bWriting = false;
            break;
          }
        case 1:
          {
            pUserLog->AddCanvasSize(GetRandomInt(0, 100), GetRandomInt(0, 100), GetRandomInt(0, 100), GetRandomInt(0, 100));
            break;
          }
        case 2:
          {
            pUserLog->AddWindowSize(GetRandomInt(0, 100), GetRandomInt(0, 100), GetRandomInt(0, 100), GetRandomInt(0, 100));
            break;
          }
        case 3:
          {
            pUserLog->AddMouseLocation(GetRandomInt(0, 100), GetRandomInt(0, 100), GetRandomFloat(-100, 100));
            break;
          }
        case 4:
          {
            pUserLog->AddCanvasSize(GetRandomInt(0, 100), GetRandomInt(0, 100), GetRandomInt(0, 100), GetRandomInt(0, 100));
            pUserLog->AddMouseLocationNormalized(GetRandomInt(0, 100), GetRandomInt(0, 100), (bool) GetRandomInt(0, 1), GetRandomFloat(-100, 100));
            break;
          }
        case 5:
          {
            // Either use a parameter name we've already used, or create a new one
            string strParamName = "";
            if ((vectorParams.size() <= 0) || (rand() % 2 == 0))
            {
              strParamName = GetRandomString(4, 40);
              vectorParams.push_back(strParamName);
            }
            else
            {
              strParamName = (string) vectorParams[GetRandomInt(0, vectorParams.size() - 1)];    
            }

            switch (rand() % 3)
            {
            case 0:
              pUserLog->AddParam(strParamName, GetRandomInt(0, 100), GetRandomBitMask(5));
              break;
            case 1:
              pUserLog->AddParam(strParamName, GetRandomFloat(-100, 100), GetRandomBitMask(5));
              break;
            case 2:
              pUserLog->AddParam(strParamName, GetRandomString(4, 40), GetRandomBitMask(5));
              break;
            }

            break;
          }
        case 6:
          {
            if (!bWriting)
            {
              pUserLog->StartWriting();
              bWriting = true;
            }
            else
            {
              pUserLog->StopWriting();
              bWriting = false;
            }
            break;
          }
        case 7:
        case 8: 
        case 9: 
        case 10:
        case 11:
        case 12:
          {
            if (!bWriting)
            {
              pUserLog->StartWriting();
              bWriting = true;                        
            }

            Dasher::VECTOR_SYMBOL_PROB vectorAdded;
            int numToAdd = GetRandomInt(1, 5);
            for (int i = 0; i < numToAdd; i++)
            {
              Dasher::SymbolProb symProb;
              symProb.prob = GetRandomFloat(0, 1);
              symProb.sym  = rand() % pAlphabet->GetNumberSymbols();
              vectorAdded.push_back(symProb);
            }
            pUserLog->AddSymbols(&vectorAdded);
            break;
          }
        case 13:
        case 14:
          {
            if (!bWriting)
            {
              pUserLog->StartWriting();
              bWriting = true;                        
            }
            pUserLog->DeleteSymbols(GetRandomInt(1, 3));
            break;
          }
        case 15:
        case 16:
          {
            if (bWriting)
            {
              pUserLog->StopWriting();
              bWriting = false;
            }
            pUserLog->NewTrial();
            break;
          }


        };

      }

      if (count % 100 == 0)
      {
        printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bCount: %d", count);
#ifndef _WIN32
        // Look for a file that indicates we should shutdown
        FILE* fp = NULL;
        fp = fopen("test.die", "r");
        if (fp != NULL)
        {
          fclose(fp);
          break;
        }
#endif
      }
#ifdef _WIN32
      if (kbhit() > 0)
      {
        if (getch() == 'q')
          break;
      }
#endif
    }

    if (pUserLog != NULL)
    {
      delete pUserLog;
      pUserLog = NULL;
    }

    if (pAlphabet != NULL)
    {
      delete pAlphabet;
      pAlphabet = NULL;
    }

    if (pAlphIO != NULL)
    {
      delete pAlphIO;
      pAlphIO = NULL;
    }

  }

  if (g_pLogger != NULL)
  {
    delete g_pLogger;
    g_pLogger = NULL;
  }

  return 0;
}
