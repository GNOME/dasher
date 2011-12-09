/*
 *  AbstractXMLParser.cpp
 *  Dasher
 *
 *  Created by Alan Lawrence on 17/03/2011.
 *  Copyright 2011 Cavendish Laboratory. All rights reserved.
 *
 */

#include "AbstractXMLParser.h"

#include <fstream>
#include <stdio.h>
#include <cstring>

using namespace std;

bool AbstractParser::ParseFile(const string &strPath, bool bUser) {
  std::ifstream in(strPath.c_str(), ios::binary);
  bool res=Parse("file://"+strPath, in, bUser);
  in.close();
  return res;
}

AbstractXMLParser::AbstractXMLParser(CMessageDisplay *pMsgs) : AbstractParser(pMsgs) {
}

bool AbstractXMLParser::isUser() {
  return m_bUser;
}

bool AbstractXMLParser::Parse(const std::string &strDesc, istream &in, bool bUser) {
  if (!in.good()) return false;
  
  //we'll be re-entrant (i.e. allow nested calls), as it's not difficult here...
  const bool bOldUser = m_bUser;
  const string strOldDesc = m_strDesc;
  m_bUser = bUser;
  m_strDesc = strDesc;
  
  XML_Parser Parser = XML_ParserCreate(NULL);

  // Members passed as callbacks must be static, so don't have a "this" pointer.
  // We give them one through horrible casting so they can effect changes.
  XML_SetUserData(Parser, this);

  XML_SetElementHandler(Parser, XML_StartElement, XML_EndElement);
  XML_SetCharacterDataHandler(Parser, XML_CharacterData);
  bool bRes(true);
  char Buffer[1024];
  int Done;
  do {
    in.read(Buffer, sizeof(Buffer));
    size_t len = in.gcount();
    Done = len < sizeof(Buffer);
    if(XML_Parse(Parser, Buffer, len, Done) == XML_STATUS_ERROR) {
      bRes=false;
      if (m_pMsgs) {
        const XML_LChar *xmle=XML_ErrorString(XML_GetErrorCode(Parser)); //think XML_LChar==char, depends on preprocessor variables...
        
        ///TRANSLATORS: the first string is the error message from the XML Parser;
        /// the second is the URL of the file we're trying to read.
        m_pMsgs->FormatMessageWith2Strings(_("XML Error %s in file %s "), xmle, m_strDesc.c_str());
#ifdef DEBUG
        std::cout << "Error in: " << string(Buffer,len) << std::endl;
#endif
      }
      break;
    }
  } while (!Done);

  XML_ParserFree(Parser);
  m_bUser = bOldUser; m_strDesc = strOldDesc;
  return bRes;
}

void AbstractXMLParser::XmlCData(const XML_Char *str, int len) {

}

void AbstractXMLParser::XML_Escape(std::string &Input, bool Attribute) {
  // The XML "W3C Recommendation" is here: http://www.w3.org/TR/REC-xml
  
  for(unsigned int i = 0; i < Input.size(); i++) {
    // & and < need escaping in XML. In one rare circumstance >
    // needs escaping too. I'll always do it, as I'm allowed to.
    if(Input[i] == '&') {
      Input.replace(i, 1, "&amp;");
      continue;
    }
    if(Input[i] == '<') {
      Input.replace(i, 1, "&lt;");
      continue;
    }
    if(Input[i] == '>') {
      Input.replace(i, 1, "&gt;");
      continue;
    }
    // " and ' might need escaping inside attributes, I'll do them all.
    if(Attribute == false)
      continue;
    
    if(Input[i] == '\'') {
      Input.replace(i, 1, "&apos;");
      continue;
    }
    if(Input[i] == '"') {
      Input.replace(i, 1, "&quot;");
      continue;
    }
  }
}


//Actual callbacks for expat. void*, here we come!
void AbstractXMLParser::XML_StartElement(void *userData, const XML_Char * name, const XML_Char ** atts) {
  static_cast<AbstractXMLParser*>(userData)->XmlStartHandler(name, atts);
}

void AbstractXMLParser::XML_EndElement(void *userData, const XML_Char * name) {
  static_cast<AbstractXMLParser*>(userData)->XmlEndHandler(name);
}

void AbstractXMLParser::XML_CharacterData(void *userData, const XML_Char * s, int len) {
  static_cast<AbstractXMLParser*>(userData)->XmlCData(s,len);
}
