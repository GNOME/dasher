#pragma once

#include "../DasherCore/DasherInterfaceBase.h"
#include "../DasherCore/UserLog.h"

extern CONST UINT WM_DASHER_EVENT;
#define _WM_DASHER_EVENT (LPCWSTR)"wm_dasher_event"

extern CONST UINT WM_DASHER_FOCUS;
#define _WM_DASHER_FOCUS (LPCWSTR)"wm_dasher_focus"

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

  CCanvas *GetCanvas() {
	  return m_pCanvas;
  }

  void SetEdit(CDashEditbox * pEdit);

  void ExternalEventHandler(Dasher::CEvent *pEvent);

  virtual void WriteTrainFile(const std::string &strNewText);

    void					  Main(); 


private:

  virtual void ScanAlphabetFiles(std::vector<std::string> &vFileList);
  virtual void ScanColourFiles(std::vector<std::string> &vFileList);
  virtual void SetupPaths();
  virtual void SetupUI();
  virtual void CreateLocalFactories();
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