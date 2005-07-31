// AlphabetBox.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __AlphabetBox_h__
#define __AlphabetBox_h__

#include "../resource.h"
#include "../../DasherCore/Alphabet/AlphIO.h"
#include "../../DasherCore/Win32/DasherInterface.h"

class CAlphabetBox:public CWinWrap {
public:
  CAlphabetBox(HWND Parent, CDasherInterface *DI);
protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  CDasherInterface * m_pDasherInterface;

  HWND CustomBox;

  std::vector < std::string > AlphabetList;
  std::string m_CurrentAlphabet;
  Dasher::CAlphIO::AlphInfo CurrentInfo;
  // Some status flags:
  bool Editing;
  bool EditChar;
  bool Cloning;
  unsigned int CurrentGroup;
  unsigned int CurrentChar;

  void PopulateList();
  void InitCustomBox();
  std::string GetControlText(HWND Dialog, int ControlID);
  void NewGroup(std::string NewGroup);
  void ShowGroups();
  void ShowGroupChars();
  void CustomCharacter(std::string Display, std::string Text, int Colour);
  bool UpdateInfo();
  bool Apply();
  bool Validate();
};

#endif /* #ifndef __AlphabetBox_h__ */
