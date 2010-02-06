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
    CAlphabet();
    CAlphabet(const CAlphIO::AlphInfo & AlphInfo);

    // Return size of alphabet, including control symbols
    int GetNumberSymbols() const {
      return m_Characters.size();
    }                           // return size of alphabet

    /// Return number of text symbols 
    /// Text symbols are everything which doesn't generate a new root, 
    /// i.e. control mode and conversion mode
    int GetNumberTextSymbols() const {

      // TODO: This really does need to be fixed, as this will sometimes be 2
      return m_Characters.size() - 1;
    } 

    Opts::ScreenOrientations GetOrientation() {
      return m_Orientation;
    } 

    Opts::AlphabetTypes GetType() {
      return m_DefaultEncoding;
    }

    const std::string & GetTrainingFile() const {
      return m_TrainingFile;
    }
    std::string GetGameModeFile() {
      return m_GameModeFile;
    }
    std::string & GetPalette() {
      return m_DefaultPalette;
    }

    symbol GetParagraphSymbol() const;
    symbol GetSpaceSymbol() const;
    symbol GetControlSymbol() const;
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

    int GetTextColour(symbol i);      // return the foreground colour for i'th symbol
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

    void SetOrientation(Opts::ScreenOrientations Orientation) {
      m_Orientation = Orientation;
    }
    void SetLanguage(Opts::AlphabetTypes Group) {
      m_DefaultEncoding = Group;
    }
    void SetTrainingFile(std::string TrainingFile) {
      m_TrainingFile = TrainingFile;
    }
    void SetGameModeFile(std::string GameModeFile) {
      m_GameModeFile = GameModeFile;
    }
    void SetPalette(std::string Palette) {
      m_DefaultPalette = Palette;
    }

    const std::string &GetDefaultContext() const {
      return m_strDefaultContext;
    }

    SGroupInfo *m_pBaseGroup;
    int iNumChildNodes;
  private:

    // Add the characters that can appear in Nodes
    void AddChar(std::string NewCharacter, std::string Display, int Colour, std::string Foreground);    // add single char to the alphabet

    // Alphabet language parameters
    void AddParagraphSymbol(std::string NewCharacter, std::string Display, int Colour, std::string Foreground);
    void AddSpaceSymbol(std::string NewCharacter, std::string Display, int Colour, std::string Foreground);
    void AddControlSymbol(std::string NewCharacter, std::string Display, int Colour, std::string Foreground);
    void AddStartConversionSymbol(std::string NewCharacter, std::string Display , int Colour, std::string Foreground);
    void AddEndConversionSymbol(std::string NewCharacter, std::string Display, int Colour, std::string Foreground);

    Opts::AlphabetTypes m_DefaultEncoding;
    Opts::ScreenOrientations m_Orientation;
    symbol m_ParagraphSymbol;
    symbol m_SpaceSymbol;
    symbol m_ControlSymbol;
    symbol m_StartConversionSymbol;
    symbol m_EndConversionSymbol;

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

  inline symbol CAlphabet::GetControlSymbol() const {
    return m_ControlSymbol;
  }

  inline symbol CAlphabet::GetStartConversionSymbol() const {
    return m_StartConversionSymbol;
  }

  inline symbol CAlphabet::GetEndConversionSymbol() const {
    return m_EndConversionSymbol;
  }
  

}                              // end namespace dasher

#endif                          // ifndef __DASHER_ALPHABET_H__
