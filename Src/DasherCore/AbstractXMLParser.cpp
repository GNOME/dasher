/*
 *  AbstractXMLParser.cpp
 *  Dasher
 *
 *  Created by Alan Lawrence on 17/03/2011.
 *  Copyright 2011 Cavendish Laboratory. All rights reserved.
 *
 */

#include "AbstractXMLParser.h"

#include <string.h>

using std::string;

bool AbstractXMLParser::ParseFile(CMessageDisplay *pMsgs, const std::string &strFilename) {
  FILE *Input;
  if((Input = fopen(strFilename.c_str(), "r")) == (FILE *) 0) {
    // could not open file
    return false;
  }

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
    size_t len = fread(Buffer, 1, sizeof(Buffer), Input);
    Done = len < sizeof(Buffer);
    if(XML_Parse(Parser, Buffer, len, Done) == XML_STATUS_ERROR) {
      bRes=false;
      if (pMsgs) {
        const char *msg=_("XML Error %s in file %s somewhere in block: %s");
        const XML_LChar *xmle=XML_ErrorString(XML_GetErrorCode(Parser)); //think XML_LChar==char, depends on preprocessor variables...
        char *buf(new char[strlen(msg)+ strlen(xmle) + strFilename.length() + len + 3]);
        //constructing a string here to get a null terminator. Can we put a null into Buffer instead?
        sprintf(buf, msg, xmle, strFilename.c_str(), string(Buffer,len).c_str());
        pMsgs->Message(buf,true);
        delete buf;
      }
      break;
    }
  } while (!Done);

  XML_ParserFree(Parser);
  fclose(Input);
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
