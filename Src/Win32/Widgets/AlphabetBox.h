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
//#include "../../DasherCore/Alphabet/AlphIO.h"
#include "PrefsPageBase.h"

class CAlphabetBox : public CPrefsPageBase {
public:
	CAlphabetBox(HWND Parent, Dasher::CDasherInterfaceBase *DI, CAppSettings *pAppSettings);
protected:
	LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);
private:
  Dasher::CDasherInterfaceBase * m_pDasherInterface;
  HWND m_hPropertySheet;

  HWND CustomBox;

  std::vector < std::string > AlphabetList;
  std::string m_CurrentAlphabet;
  Dasher::CAlphIO::AlphInfo CurrentInfo;

  std::vector < std::string > ColourList;
  
  Dasher::CColourIO::ColourInfo CurrentColourInfo;

  // Some status flags:
  bool Editing;
  bool EditChar;
  bool Cloning;
  unsigned int CurrentGroup;
  unsigned int CurrentChar;

 HWND slider;
  HWND uniformbox;

  TCHAR m_tcBuffer[1000];


  void PopulateList();
  /*void InitCustomBox();
  std::string GetControlText(HWND Dialog, int ControlID);
  void NewGroup(std::string NewGroup);
  void ShowGroups();
  void ShowGroupChars();
  void CustomCharacter(std::string Display, std::string Text, int Colour);
  bool UpdateInfo();*/
  bool Apply();
 
};

#endif /* #ifndef __AlphabetBox_h__ */
