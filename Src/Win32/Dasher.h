#pragma once

#include "Common/wincommon.h"
#include "../DasherCore/DasherInterfaceBase.h"
#include "../DasherCore/UserLog.h"

#include <string>
#include <vector>

extern CONST UINT WM_DASHER_EVENT;
#define _WM_DASHER_EVENT (LPCWSTR)"wm_dasher_event"

extern CONST UINT WM_DASHER_FOCUS;
#define _WM_DASHER_FOCUS (LPCWSTR)"wm_dasher_focus"

extern CONST UINT WM_DASHER_GAME_MESSAGE;
#define _WM_DASHER_GAME_MESSAGE (LPCWSTR)"wm_dasher_game_message"


class CCanvas;
class CSlidebar;
class CDashEditbox;

namespace Dasher {
class CDasher : public CDasherInterfaceBase 
{
public:
  CDasher(HWND Parent);
  ~CDasher(void);

  // The following functions will not be part of the final interface
  // In order to get rid of this, it'll be necessary to make this class
  // act more like a UI widget.

  //CCanvas *GetCanvas() {
	 // return m_pCanvas;
  //}

  // TODO: Can some of these be obtained by inheriting from something?
  void Move(int iX, int iY, int iWidth, int iHeight);
  void TakeFocus();

  void SetEdit(CDashEditbox * pEdit);

  void ExternalEventHandler(Dasher::CEvent *pEvent);
  void GameMessageOut(int message, const void* messagedata);
  
  virtual void WriteTrainFile(const std::string &strNewText);

  void Main(); 


private:

  virtual void ScanAlphabetFiles(std::vector<std::string> &vFileList);
  virtual void ScanColourFiles(std::vector<std::string> &vFileList);
  virtual void SetupPaths();
  virtual void SetupUI();
  virtual void CreateModules();
  virtual void StartTimer();
  virtual void ShutdownTimer();
  void CreateSettingsStore();

  virtual int GetFileSize(const std::string &strFileName);
  void ScanDirectory(const Tstring &strMask, std::vector<std::string> &vFileList);
  bool                    GetWindowSize(int* pTop, int* pLeft, int* pBottom, int* pRight);
  void                    Log();                        // Does the logging

  CCanvas *m_pCanvas;
  CDashEditbox *m_pEdit;
 
  HWND m_hParent;
};
}