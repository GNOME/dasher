// TrainingHelper.h
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

#ifndef __TrainingHelper_h__
#define __TrainingHelper_h__

#include "Alphabet/Alphabet.h"

#include <string>

namespace Dasher {

  class CTrainingHelper {
  public:
	CTrainingHelper(const CAlphabet *m_pAlphabet);
	  
    void HandleStartElement(const XML_Char *szName, 
			    const XML_Char **pAtts);
  
    void HandleEndElement(const XML_Char *szName);
    
    void HandleCData(const XML_Char *szS, 
		     int iLen);

    void LoadFile(const std::string &strFileName);

  protected:
    const Dasher::CAlphabet *m_pAlphabet;

    virtual void Train(CAlphabet::SymbolStream &syms)=0;
	  
  private:  
	void LoadPlain(const std::string &strFileName);
	  
	void LoadXML(const std::string &strFileName);
	  
    bool m_bInSegment;
    std::string m_strCurrentText;
  };
};

#endif
