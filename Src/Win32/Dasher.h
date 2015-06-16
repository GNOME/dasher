#pragma once

#include "Common/wincommon.h"
#include "../DasherCore/DashIntfScreenMsgs.h"
#include "../DasherCore/UserLog.h"

#ifdef _WIN32_WCE
//on WinCE, do not support speech
#undef WIN32_SPEECH
#endif

#ifdef WIN32_SPEECH
#include <sapi.h>
#endif

#include <string>
#include <vector>

extern CONST UINT WM_DASHER_FOCUS;
#define _WM_DASHER_FOCUS (LPCWSTR)"wm_dasher_focus"

class CCanvas;
class CEdit;
class CDasherWindow;

namespace Dasher {
class CDasher : public CDashIntfScreenMsgs
{
public:
  CDasher(HWND Parent, CDasherWindow *pWindow, CEdit *pEdit);
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

  void HandleEvent(int iParameter);
  void editOutput(const std::string &strText, CDasherNode *pSource);
  void editDelete(const std::string &strText, CDasherNode *pSource);
  unsigned int ctrlMove(bool bForwards, CControlManager::EditDistance iDist);
  unsigned int ctrlDelete(bool bForwards, CControlManager::EditDistance iDist);
    
  virtual void WriteTrainFile(const std::string &filename, const std::string &strNewText);
  void Main(); 

  virtual std::string GetAllContext();
  std::string GetContext(unsigned int iStart, unsigned int iLength);

#ifdef WIN32_SPEECH
  bool SupportsSpeech();
  void Speak(const std::string &text, bool bInterrupt);
#endif
  bool SupportsClipboard() {return true;};
  void CopyToClipboard(const std::string &text);
  
  virtual int GetFileSize(const std::string &strFileName);
private:

  virtual void ScanFiles(AbstractParser *parser, const std::string &strPattern);
  virtual void CreateModules();

  void ScanDirectory(const Tstring &strMask, std::vector<std::string> &vFileList);
  bool                    GetWindowSize(int* pTop, int* pLeft, int* pBottom, int* pRight);
  void                    Log();                        // Does the logging

  CCanvas *m_pCanvas;
  HWND m_hParent;
  CDasherWindow *m_pWindow;
  CEdit *m_pEdit;
#ifdef WIN32_SPEECH
  ISpVoice *pVoice;
  bool m_bAttemptedSpeech;
#endif
};
}
