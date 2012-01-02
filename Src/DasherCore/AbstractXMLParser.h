/*
 *  AbstractXMLParser.h
 *  Dasher
 *
 *  Created by Alan Lawrence on 17/03/2011.
 *  Copyright 2011 Cavendish Laboratory. All rights reserved.
 *
 */

#ifndef __ABSTRACT_XML_PARSER_H__
#define __ABSTRACT_XML_PARSER_H__

#include "../Common/Common.h"
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Messages.h"

#include <string>
#include <expat.h>
#include <iostream>

class AbstractParser {
public:
  AbstractParser(CMessageDisplay *pMsgs) : m_pMsgs(pMsgs) { }
  ///Utility method: constructs an ifstream to read from the specified file,
  /// then calls Parse(string&,istream&,bool) with the description 'file://strPath'
  virtual bool ParseFile(const std::string &strPath, bool bUser);
  
  /// \param strDesc string to display to user to identify the source of this data,
  /// if there is an error. (Suggest: use a url, e.g. file://...)
  /// \param bUser if True, the file is from a user location (editable), false if from a
  /// system one. (Some subclasses treat the data differently according to which of these
  /// it is from.)
  virtual bool Parse(const std::string &strDesc, std::istream &in, bool bUser) = 0;
  
protected:
  ///The MessageDisplay to use to inform the user. Subclasses should use this
  /// too for any (e.g. semantic) errors they may detect.
  CMessageDisplay * const m_pMsgs;
};

///Basic wrapper over (Expat) XML Parser, handling file IO and wrapping C++
/// virtual methods over C callbacks. Subclasses must implement methods to
/// handle actual tags.
class AbstractXMLParser : public AbstractParser {
public:
  ///Parse (the whole) file - done in chunks to avoid loading the whole thing into memory.
  /// Any errors _besides_ file-not-found, will be passed to m_pMsgs as modal messages.
  virtual bool Parse(const std::string &strDesc, std::istream &in, bool bUser);
protected:
  ///Create an AbstractXMLParser which will use the specified MessageDisplay to
  /// inform the user of any errors.
  AbstractXMLParser(CMessageDisplay *pMsgs);

  ///Subclasses may call to get the description of the current file
  const std::string &GetDesc() {return m_strDesc;}
  ///Subclasses may call to determine if the current file is from a user location
  bool isUser();
  
  ///Subclass should override to handle a start tag
  virtual void XmlStartHandler(const XML_Char *name, const XML_Char **atts)=0;
  ///Subclass should override to handle an end tag
  virtual void XmlEndHandler(const XML_Char *name)=0;
  ///Subclass may override to handle character data; the default implementation does nothing.
  ///\param str pointer to string data, note is NOT null-terminated
  ///\param len number of bytes to read from pointer
  virtual void XmlCData(const XML_Char *str, int len);
  
  ///Utility function provided for subclasses wishing to perform XML _output_.
  // & to &amp;  < to &lt; and > to &gt;  and if (Attribute) ' to &apos; and " to &quot;
  /// \param Input string to escape, will be updated in-place.
  void XML_Escape(std::string &Input, bool Attribute);
private:
  /// The actual callbacks passed to the expat library.
  /// These just convert the void* we passed to the library back into
  /// an instance pointer, to get a C++ class to work with a plain C library.
  static void XML_StartElement(void *userData, const XML_Char * name, const XML_Char ** atts);
  static void XML_EndElement(void *userData, const XML_Char * name);
  static void XML_CharacterData(void *userData, const XML_Char * s, int len);
  bool m_bUser;
  std::string m_strDesc;
};

#endif
