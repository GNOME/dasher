// Alphabet.h
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

#ifndef __DASHER_ALPHABET_H__
#define __DASHER_ALPHABET_H__

#include "AlphIO.h"
#include "AlphabetMap.h"
#include "../DasherTypes.h"
#include "GroupInfo.h"

#include <cstdlib>
#include <iostream>
#include <vector>

namespace Dasher {
  ///
  /// \defgroup Alphabet Alphabet information
  /// @{

  class CAlphabet {
  public:
    CAlphabet(const CAlphIO::AlphInfo & AlphInfo);

    /// Return number of text symbols - inc space and para, but no control/conversion start/end
    int GetNumberTextSymbols() const {
      return m_Characters.size();
    } 

    Opts::ScreenOrientations GetOrientation() const {
      return m_Orientation;
    } 

    Opts::AlphabetTypes GetType() const {
      return m_DefaultEncoding;
    }

    const std::string & GetTrainingFile() const {
      return m_TrainingFile;
    }
    const std::string &GetGameModeFile() const {
      return m_GameModeFile;
    }
    const std::string & GetPalette() const {
      return m_DefaultPalette;
    }

    symbol GetParagraphSymbol() const;
    symbol GetSpaceSymbol() const;
    symbol GetStartConversionSymbol() const;
    symbol GetEndConversionSymbol() const;

    const std::string & GetDisplayText(symbol i) const {
      return m_Display[i];
    }
    // return display string for i'th symbol

    const std::string & GetText(symbol i) const {
      return m_Characters[i];
    } 
    // return string for i'th symbol

    int GetColour(symbol i, int iPhase) const;

    int GetTextColour(symbol i) const;      // return the foreground colour for i'th symbol
    const std::string & GetForeground(symbol i) const {
      return m_Foreground[i];
    } // return the foreground colour for i'th symbol

/*     int GetGroupCount() const { */
/*       return m_iGroups; */
/*     } int GetGroupStart(int i) const { */
/*       return m_GroupStart[i]; */
/*     } int GetGroupEnd(int i) const { */
/*       return m_GroupEnd[i]; */
/*     } */

    //int get_group(symbol i) const {return m_Group[i];}                
    // return group membership of i'th symbol
    
    class SymbolStream {
    public:
      SymbolStream(const CAlphabet *pAlph, std::istream &_in);
      symbol next();
    private:
      void readMore();
      const alphabet_map &map;
      char buf[1024];
      int pos, len;
      std::istream &in;
    };
    
    // Fills Symbols with the symbols corresponding to Input. {{{ Note that this
    // is not necessarily reversible by repeated use of GetText. Some text
    // may not be recognised and so discarded. }}}

    void GetSymbols(std::vector<symbol> &Symbols, const std::string &Input) const;
    //SymbolStream *GetSymbols(std::istream &in) const;

    void Trace() const;         // diagnostic

    const std::string &GetDefaultContext() const {
      return m_strDefaultContext;
    }

    const SGroupInfo *m_pBaseGroup;
    const int iNumChildNodes;
  private:

    // Add the characters that can appear in Nodes
    /// add single char to the character set; return it's index as a symbol
    symbol AddChar(std::string NewCharacter, std::string Display, int Colour, std::string Foreground);    // add single char to the alphabet

    Opts::AlphabetTypes m_DefaultEncoding;
    Opts::ScreenOrientations m_Orientation;
    symbol m_ParagraphSymbol;
    symbol m_SpaceSymbol;
    symbol m_StartConversionSymbol;
    symbol m_EndConversionSymbol;
    ///text and colour for the control node in this alphabet. TODO: use!
    std::string m_strControlDisplay;
    int m_iControlColour;

    std::string m_TrainingFile;
    std::string m_GameModeFile;
    std::string m_DefaultPalette;

    class utf8_length
      {
        public:
          utf8_length();
          int operator[](const unsigned char) const;
          int max_length;
        private:
          int utf8_count_array[0x100];
      };
    static utf8_length m_utf8_count_array;

    // TODO: This is inane
    std::vector < std::string > m_Characters;   // stores the characters
    std::vector < std::string > m_Display;      // stores how the characters are visually represented in the Dasher nodes
    std::vector < int >m_Colours;       // stores the colour of the characters
    std::vector < std::string > m_Foreground;   // stores the colour of the character foreground
    // ----

    alphabet_map TextMap;

    std::string m_strDefaultContext;
  };

  /// @}

  inline symbol CAlphabet::GetParagraphSymbol() const {
    return m_ParagraphSymbol;
  }

  inline symbol CAlphabet::GetSpaceSymbol() const {
    return m_SpaceSymbol;
  }

  inline symbol CAlphabet::GetStartConversionSymbol() const {
    return m_StartConversionSymbol;
  }

  inline symbol CAlphabet::GetEndConversionSymbol() const {
    return m_EndConversionSymbol;
  }
  

}                              // end namespace dasher

#endif                          // ifndef __DASHER_ALPHABET_H__
