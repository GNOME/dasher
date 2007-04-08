// CustomColours.h
//
// Copyright (c) 2002 Iain Murray

#ifndef __CustomColours_h__
#define __CustomColours_h__

#include "ColourIO.h"

namespace Dasher {
  class CCustomColours;
}

/// \ingroup Colours
/// @{
class Dasher::CCustomColours {
 public:
  CCustomColours(const CColourIO::ColourInfo & ColourInfo);
  int GetNumColours() const {
    return m_Red.size();
  }
  int GetRed(int colour) const {
    return m_Red[colour];
  }
  int GetGreen(int colour) const {
    return m_Green[colour];
  }
  int GetBlue(int colour) const {
    return m_Blue[colour];
  }
 private:
  const CColourIO::ColourInfo * m_ColourInfo;
  std::vector<int> m_Red;
  std::vector<int> m_Green;
  std::vector<int> m_Blue;
};
/// @}

#endif /* #ifndef __CustomColours_h__ */
