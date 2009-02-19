// TrainingHelper.cpp
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#include "AlphabetManagerFactory.h"
#include "TrainingHelper.h"

#include <cstdio>
#include <cstring>
#include <expat.h>
#include <fstream>
#include <ios>
#include <iostream>
#include <vector>

//using namespace Dasher;

static void XML_StartElement(void *pUserData, const XML_Char *szName, const XML_Char **pAtts);
static void XML_EndElement(void *pUserData, const XML_Char *szName);
static void XML_CharacterData(void *pUserData, const XML_Char *szS, int iLen);

void 
Dasher::CTrainingHelper::LoadFile(const std::string &strFileName, 
				  Dasher::CTrainer *pTrainer, 
				  const Dasher::CAlphabet *pAlphabet) {
   if(strFileName == "")
    return;

   FILE *pInputFile;
   if((pInputFile = fopen(strFileName.c_str(), "r")) == (FILE *) 0)
     return;

   char szTestBuffer[6];

   int iNumberRead = fread(szTestBuffer, 1, 5, pInputFile);
   szTestBuffer[iNumberRead] = '\0';

   fclose(pInputFile);

   if(!strcmp(szTestBuffer, "<?xml")) {
     LoadXML(strFileName, pTrainer, pAlphabet);
   }
   else {
     LoadPlain(strFileName, pTrainer, pAlphabet);
   }
}

void
Dasher::CTrainingHelper::LoadPlain(const std::string &strFileName, 
			   Dasher::CTrainer *pTrainer, 
			   const Dasher::CAlphabet *pAlphabet) {
  
  if (strFileName.empty())
    {
      std::cerr << "LoadPlain called with empty filename" << std::endl;
      return;
    }

  std::ifstream in(strFileName.c_str(), std::ios::binary);
  if (in.bad())
    {
      std::cerr << "Unable to open file \"" << strFileName << "\" for reading"
                << std::endl;
      return;
    }

  std::vector<Dasher::symbol> vSymbols;
  vSymbols.clear();
  pAlphabet->GetSymbols(vSymbols, in);
  pTrainer->Train(vSymbols);

  in.close();
}

void 
Dasher::CTrainingHelper::LoadXML(const std::string &strFileName, 
			 Dasher::CTrainer *pTrainer, 
			 const Dasher::CAlphabet *pAlphabet) {

  m_bInSegment = false;
  m_pAlphabet = pAlphabet;
  m_pTrainer = pTrainer;

  FILE *pInput;
  if((pInput = fopen(strFileName.c_str(), "r")) == (FILE *) 0) {
    // could not open file
    return;
  }

  XML_Parser Parser = XML_ParserCreate(NULL);

  // Members passed as callbacks must be static, so don't have a "this" pointer.
  // We give them one through horrible casting so they can effect changes.
  XML_SetUserData(Parser, this);

  XML_SetElementHandler(Parser, XML_StartElement, XML_EndElement);
  XML_SetCharacterDataHandler(Parser, XML_CharacterData);

  const unsigned int iBufferSize = 1024;
  char szBuffer[iBufferSize];
  bool bDone;

  do {
    size_t iLen = fread(szBuffer, 1, sizeof(szBuffer), pInput);
    bDone = iLen < sizeof(szBuffer);
    if(XML_Parse(Parser, szBuffer, iLen, bDone) == XML_STATUS_ERROR) {
      break;
    }
  } while(!bDone);

  XML_ParserFree(Parser);
  fclose(pInput);
}

void 
Dasher::CTrainingHelper::HandleStartElement(const XML_Char *szName, 
				    const XML_Char **pAtts) {
  if(!strcmp(szName, "segment")) {
    m_strCurrentText = "";
    m_bInSegment = true;
  }
}

void 
Dasher::CTrainingHelper::HandleEndElement(const XML_Char *szName) {
  if(!strcmp(szName, "segment")) {
    m_pTrainer->Reset();

    std::vector<Dasher::symbol> vSymbols;
    m_pAlphabet->GetSymbols(&vSymbols, &m_strCurrentText, false);
    m_pTrainer->Train(vSymbols);
    
    m_bInSegment = false;
  }
}

void 
Dasher::CTrainingHelper::HandleCData(const XML_Char *szS, 
			     int iLen) {
  if(m_bInSegment)
    m_strCurrentText += std::string(szS, iLen);
}

static void 
XML_StartElement(void *pUserData, 
		 const XML_Char *szName, 
		 const XML_Char **pAtts) {

  static_cast<Dasher::CTrainingHelper *>(pUserData)->HandleStartElement(szName, pAtts);
}

static void 
XML_EndElement(void *pUserData, 
	       const XML_Char *szName) {

  static_cast<Dasher::CTrainingHelper *>(pUserData)->HandleEndElement(szName);
}

static void 
XML_CharacterData(void *pUserData, 
		  const XML_Char *szS, 
		  int iLen) {
  
  static_cast<Dasher::CTrainingHelper *>(pUserData)->HandleCData(szS, iLen);
}
