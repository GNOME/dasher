#ifndef __WinSel_h__
#define __WinSel_h__

#include "../resource.h"
#include "../../DasherCore/DasherTypes.h"
#include "Edit.h"

#include "WinWrap.h"

class CWinSel:public CWinWrap {
public:
  CWinSel(HWND Parent, CEdit * m_pEdit);

protected:
  LRESULT WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);

private:

  long RefreshWindow(HWND hwndWindowToBeRefreshed);
  long SearchWindow(HWND hwndDialog);

  long DoMouseMove(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam);

  long HighlightFoundWindow(HWND hwndDialog, HWND hwndFoundWindow);

  long DoMouseUp(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam);

  void SetFinderToolImage(HWND hwndDialog, BOOL full);

  HPEN g_hRectanglePen;
  HWND m_hwnd;
  HWND CustomBox;
  HWND g_hwndFoundWindow;
  CEdit *m_pEdit;
  BOOL g_bStartSearchWindow;
  HCURSOR g_hCursorSearchWindow, g_hCursorPrevious;

  HBITMAP g_hBitmapFinderToolFilled;

  HBITMAP g_hBitmapFinderToolEmpty;

};
#endif
