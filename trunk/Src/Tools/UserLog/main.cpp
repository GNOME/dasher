// Command line application that can read a UserLog XML file
// and rebuilds the C++ objects that represented it.  
//
// Copyright 2005 by Keith Vertanen
//

#include "../../Common/Common.h"

#include <string>
#include <vector>

#include "../../DasherCore/DasherTypes.h"

// Declare our global file logging object
#include "FileLogger.h"
#ifdef _DEBUG
const eLogLevel gLogLevel   = logDEBUG;
const int       gLogOptions = logTimeStamp | logDateStamp | logDeleteOldFile;    
#else
const eLogLevel gLogLevel = logNORMAL;
const int       gLogOptions = logTimeStamp | logDateStamp;
#endif
CFileLogger* gLogger = NULL;

#include <iostream>

#include "../../DasherCore/UserLog.h"

// Can't add Utils.h back to CVS so I'm relative linking to another fricken directory
#include "../UserLogLoadTest/Utils.h"

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

#ifndef VECTOR_VECTOR_VECTOR_STRING
typedef vector<VECTOR_VECTOR_STRING>            VECTOR_VECTOR_VECTOR_STRING;
#endif
#ifndef VECTOR_VECTOR_VECTOR_STRING_ITER
typedef vector<VECTOR_VECTOR_STRING>::iterator  VECTOR_VECTOR_VECTOR_STRING_ITER;
#endif

string GetOutputName(const string& strBase, int numFile, int numTrial, int numNav);
void OutputToFile(const string& strData, const string& strBase, int numFile, int numTrial, int numNav);
void OutputToFile(DENSITY_GRID data, int gridSize, const string& strBase, int numFile, int numTrial, int numNav);
void OutputData(const string& strBase, VECTOR_VECTOR_VECTOR_STRING* vectorData, int numFile, int numTrial, int numNav);

// Create the output filename based on the current counts
string GetOutputName(const string& strBase, int numFile, int numTrial, int numNav)
{
  string strResult = strBase;
  char strNum[256];

  if (numFile >= 0)
  {
    sprintf(strNum, "_file%d", numFile);
    strResult += strNum;
  }
  if (numTrial >= 0)
  {
    sprintf(strNum, "_trial%d", numTrial);
    strResult += strNum;
  }
  if (numNav >= 0)
  {
    sprintf(strNum, "_nav%d", numNav);
    strResult += strNum;
  }

  return strResult;
}

void OutputToFile(const string& strData, const string& strBase, int numFile, int numTrial, int numNav)
{
  string strFilename = GetOutputName(strBase, numFile, numTrial, numNav);

  FILE* fp = NULL;
  fp = fopen(strFilename.c_str(), "w");

  if (fp != NULL)
  {
    fputs(strData.c_str(), fp);
    fclose(fp);
    fp = NULL;
  }
}

// Output a two dimensional grid of floating point values.
// Free up the memory as well.
void OutputToFile(DENSITY_GRID data, int gridSize, const string& strBase, int numFile, int numTrial, int numNav)
{
  if (data == NULL)
    return;

  string strFilename = GetOutputName(strBase, numFile, numTrial, numNav);

  FILE* fp = NULL;
  fp = fopen(strFilename.c_str(), "w");

  char strNum[256];

  if (fp != NULL)
  {
    for (int i = 0; i < gridSize; i++)
    {
      string strLine = "";

      for (int j = 0; j < gridSize; j++)
      {
        sprintf(strNum, "%0.4f", data[i][j]);        
        strLine += strNum;
        if (j < (gridSize - 1))
          strLine += "\t";
      }

      strLine += "\n";
      fputs(strLine.c_str(), fp);
    }

    fclose(fp);
    fp = NULL;
  }

  if (data != NULL)
  {
    for (int i = 0; i < gridSize; i++)
    {
      if (data[i] != NULL)
      {
        delete data[i];
        data[i] = NULL;
      }
    }
    delete data;
    data = NULL;
  }

}

// Output the passed in string data that is in a vector vector vector.
// Outer vector is for the data beloning to each file.
// Middle vector has data for each trial in a file.
// Inner vector is data for each navigation cycle in a trial.
void OutputData(const string& strBase, VECTOR_VECTOR_VECTOR_STRING* vectorData, int numFile, int numTrial, int numNav)
{
  if (vectorData == NULL)
    return;

  string strOutput = "";

  // Loop over each file
  for (VECTOR_VECTOR_VECTOR_STRING_ITER iter = vectorData->begin(); 
    iter < vectorData->end();
    iter++)
  {
    // Loop over each trial
    for (VECTOR_VECTOR_STRING_ITER iter2 = iter->begin();
      iter2 < iter->end();
      iter2++)
    {
      // Loop over each navigation cycle
      for (VECTOR_STRING_ITER iter3 = iter2->begin();
        iter3 < iter2->end();
        iter3++)
      {
        strOutput += (*iter3);

        // Output if we are suppose to separate each navigation cycle
        if ((numNav >= 0) && (strOutput.length() > 0))
        {
          OutputToFile(strOutput, strBase, numFile, numTrial, numNav);
          strOutput = "";
          numNav++;
        }

      } // end for each nav cycle

      // Output to the trial file
      if (numTrial >= 0) 
      {
        if (strOutput.length() > 0)
        {
          OutputToFile(strOutput, strBase, numFile, numTrial, numNav);
          strOutput = "";
        }
        if (numNav > 0)
          numNav = 0;
        numTrial++;
      }

    } // end for each trial

    // Output to for a file
    if (numFile >= 0) 
    {
      if (strOutput.length() > 0)
      {
        OutputToFile(strOutput, strBase, numFile, numTrial, numNav);
        strOutput = "";
      }

      if (numTrial > 0)
        numTrial = 0;
      if (numNav > 0)
        numNav = 0;
      numFile++;            
    }

  } // end for each file

  // See if we need to output everything to one big file
  if (strOutput.length() > 0)
  {
    OutputToFile(strOutput, strBase, numFile, numTrial, numNav);
  }

}

// Output the passed in density grid data that is in a vector vector vector.
// Outer vector is for the data beloning to each file.
// Middle vector has data for each trial in a file.
// Inner vector is data for each navigation cycle in a trial.
void OutputData(const string& strBase, VECTOR_VECTOR_VECTOR_DENSITY_GRIDS* vectorData, int gridSize, int numFile, int numTrial, int numNav)
{
  if (vectorData == NULL)
    return;

  DENSITY_GRID output = NULL;

  // Loop over each file
  for (VECTOR_VECTOR_VECTOR_DENSITY_GRIDS_ITER iter = vectorData->begin(); 
    iter < vectorData->end();
    iter++)
  {
    // Loop over each trial
    for (VECTOR_VECTOR_DENSITY_GRIDS_ITER iter2 = iter->begin();
      iter2 < iter->end();
      iter2++)
    {
      // Loop over each navigation cycle
      for (VECTOR_DENSITY_GRIDS_ITER iter3 = iter2->begin();
        iter3 < iter2->end();
        iter3++)
      {
        DENSITY_GRID grid = *iter3;
        output = CUserLogTrial::MergeGrids(gridSize, output, grid);

        // Output if we are suppose to separate each navigation cycle
        if ((numNav >= 0) && (output != NULL))
        {
          OutputToFile(output, gridSize, strBase, numFile, numTrial, numNav);
          output = NULL;
          numNav++;
        }

      } // end for each nav cycle

      // Output to the trial file
      if (numTrial >= 0) 
      {
        if (output != NULL)
          OutputToFile(output, gridSize, strBase, numFile, numTrial, numNav);
        output = NULL;
        if (numNav > 0)
          numNav = 0;
        numTrial++;
      }

    } // end for each trial

    // Output to for a file
    if (numFile >= 0) 
    {
      if (output != NULL)
        OutputToFile(output, gridSize, strBase, numFile, numTrial, numNav);
      output = NULL;
      if (numTrial > 0)
        numTrial = 0;
      if (numNav > 0)
        numNav = 0;
      numFile++;            
    }

  } // end for each file

  // See if we need to output everything to one big file
  if (output != NULL)
    OutputToFile(output, gridSize, strBase, numFile, numTrial, numNav);
  output = NULL;
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
#ifdef _DEBUG
  // Windows debug build memory leak detection
  EnableLeakDetection();
#endif
#endif

  // Global logging object we can use from anywhere
  gLogger = new CFileLogger("UserLog.log",
    gLogLevel,		
    gLogOptions);

  { // memory leak scoping

    string	strInListFilename		= "";
    string  strOutputBase           = "out";
    bool    bNormMouse              = false;
    bool    bDensityMouse           = false;
    int     gridSize                = 20;

    // These track the number index we use on output files.
    // -1 if we aren't separating on this item
    int     numOutputFile           = -1;
    int     numOutputTrial          = -1;
    int     numOutputNav            = -1;

    if (argc <= 2)
    {
      cout << "UserLog" << endl;
      cout << "  -in              <XML log filename>" << endl;
      cout << "  -inList          <file with list of log filenames>" << endl;
      cout << "  -out             <output base name>" << endl;
      cout << "  -separateFiles" << endl;
      cout << "  -separateTrials" << endl;
      cout << "  -separateNavs" << endl;
      cout << "  -normMouse" << endl;
      cout << "  -densityMouse" << endl;
      cout << "  -densityGrid     <grid size>" << endl;

      cout << endl;
      return 0;
    }

    VECTOR_STRING vectorInputFiles;

    // Parse the command line options
    int i = 1;
    while (i < argc)
    {
      string strParam     = makeLower(argv[i]);            
      string strNextParam = "";

      if (i < argc - 1)
        strNextParam = argv[i+1];

      if (strParam.compare("-in") == 0)
      {
        vectorInputFiles.push_back(strNextParam);
        i++;
      }
      else if (strParam.compare("-inlist") == 0)
      {
        strInListFilename = strNextParam;				
        i++;
      }
      else if (strParam.compare("-out") == 0)
      {
        strOutputBase = strNextParam;				
        i++;
      }
      else if (strParam.compare("-separatefiles") == 0)
      {
        numOutputFile   = 0;
      }
      else if (strParam.compare("-separatetrials") == 0)
      {
        numOutputTrial  = 0;
      }
      else if (strParam.compare("-separatenavs") == 0)
      {
        numOutputNav    = 0;
      }
      else if (strParam.compare("-normmouse") == 0)
      {
        bNormMouse = true;		
      }
      else if (strParam.compare("-densitymouse") == 0)
      {
        bDensityMouse = true;		
      }
      else if (strParam.compare("-densitygrid") == 0)
      {
        gridSize = atoi(strNextParam.c_str());
        i++;
      }
      else
      {
        // Failure to parse this parameter
        cout << "Unknown switch: " << strParam << endl;
      }

      i++;
    }

    // Read in the list of files we are suppose to process
    if (strInListFilename.length() > 0)
    {
      FILE* fp = NULL;
      fp = fopen(strInListFilename.c_str(), "r");
      if (fp != NULL)
      {
        char buffer[1024];

        while (!feof(fp))
        {
          fscanf(fp, "%s\n", buffer);

          if (strlen(buffer) > 0)
            vectorInputFiles.push_back(buffer);
        }

        fclose(fp);
        fp = NULL;
      }
      else
        cout << "Failed to open test list file: " << strInListFilename.c_str() << endl;        
    }

    // Process each of our input files
    VECTOR_VECTOR_VECTOR_STRING         vectorStrResult;
    VECTOR_VECTOR_VECTOR_DENSITY_GRIDS  vectorGridResult;

    for (VECTOR_STRING_ITER iter = vectorInputFiles.begin(); iter < vectorInputFiles.end(); iter++)
    {
      string strFile = (string) *iter;

      CUserLog objUserLog(strFile);

      if (bNormMouse)
      {
        VECTOR_VECTOR_STRING vectorMouse = objUserLog.GetTabMouseXY(true);
        vectorStrResult.push_back(vectorMouse);
      } else if (bDensityMouse)
      {
        VECTOR_VECTOR_DENSITY_GRIDS vectorGrid = objUserLog.GetMouseDensity(gridSize);
        vectorGridResult.push_back(vectorGrid);
      }
    }        

    if (vectorStrResult.size() > 0)
      OutputData(strOutputBase, &vectorStrResult, numOutputFile, numOutputTrial, numOutputNav);
    else if (vectorGridResult.size() > 0)
      OutputData(strOutputBase, &vectorGridResult, gridSize, numOutputFile, numOutputTrial, numOutputNav);
  }

  if (gLogger != NULL)
  {
    delete gLogger;
    gLogger  = NULL;
  }

  return 0;
}
