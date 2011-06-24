
#include "WinCommon.h"
#include "WindowSelect.h"
#include "../resource.h"
#include <utility>              // for std::pair

using namespace Dasher;

CWinSel::CWinSel(HWND Parent, CEdit *m_pEdit)
:m_hwnd(0), m_pEdit(m_pEdit)
{
  g_hRectanglePen = CreatePen(PS_SOLID, 3, RGB(256, 0, 0));
  g_bStartSearchWindow = FALSE;
  g_hBitmapFinderToolFilled = LoadBitmap(WinHelper::hInstApp, MAKEINTRESOURCE(IDB_FINDER_FULL));
    g_hBitmapFinderToolEmpty = LoadBitmap(WinHelper::hInstApp, MAKEINTRESOURCE(IDB_FINDER_EMPTY));
  g_hCursorSearchWindow = LoadCursor(WinHelper::hInstApp, MAKEINTRESOURCE(IDC_SEARCH_CURSOR));
  DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_WINSELECT, Parent, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
}

long CWinSel::RefreshWindow(HWND hwndWindowToBeRefreshed) {  long lRet = 0;  InvalidateRect(hwndWindowToBeRefreshed, NULL, TRUE);  UpdateWindow(hwndWindowToBeRefreshed);  RedrawWindow(hwndWindowToBeRefreshed, NULL, NULL, RDW_FRAME | RDW_INVALIDATE                 | RDW_UPDATENOW | RDW_ALLCHILDREN);
      return lRet;}
long CWinSel::SearchWindow(HWND hwndDialog) {
  long lRet = 0;  g_bStartSearchWindow = TRUE;
  // Set the screen cursor to the BullsEye cursor.
  if(g_hCursorSearchWindow) {      g_hCursorPrevious = SetCursor(g_hCursorSearchWindow);  }
  else {    g_hCursorPrevious = NULL;   }  SetCapture(hwndDialog);  SetFinderToolImage(hwndDialog, FALSE);  return lRet;}

long CWinSel::DoMouseMove(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam) {  POINT screenpoint;  HWND hwndFoundWindow = NULL;  long lRet = 0;
  // Must use GetCursorPos() instead of calculating   // from "lParam".  GetCursorPos(&screenpoint);
  // Determine the window that lies underneath   // the mouse cursor.  hwndFoundWindow = WindowFromPoint(screenpoint);
  // We have just found a new window.  // If there was a previously found window, we must   // instruct it to refresh itself.   // This is done to remove any highlighting   // effects drawn by us.
  if(g_hwndFoundWindow) {    RefreshWindow(g_hwndFoundWindow);    }

  // Indicate that this found window is now 
  // the current global found window.
  g_hwndFoundWindow = hwndFoundWindow;

  // We now highlight the found window.
  HighlightFoundWindow(hwndDialog, g_hwndFoundWindow);  return lRet;}

long CWinSel::HighlightFoundWindow(HWND hwndDialog, HWND hwndFoundWindow) {
  // The DC of the found window.
  HDC hWindowDC = NULL;

  // Handle of the existing pen in the DC of the found window.
  HGDIOBJ hPrevPen = NULL;
  // Handle of the existing brush in the DC of the found window.
  HGDIOBJ hPrevBrush = NULL;  RECT rect;                  // Rectangle area of the found window.
  long lRet = 0;
  // Get the screen coordinates of the rectangle 
  // of the found window.
  GetWindowRect(hwndFoundWindow, &rect);
  // Get the window DC of the found window.
  hWindowDC = GetWindowDC(hwndFoundWindow);  if(hWindowDC) {
    // Select our created pen into the DC and     // backup the previous pen.    hPrevPen = SelectObject(hWindowDC, g_hRectanglePen);
    // Select a transparent brush into the DC and     // backup the previous brush.    hPrevBrush = SelectObject(hWindowDC,     GetStockObject(HOLLOW_BRUSH));
    // Draw a rectangle in the DC covering     // the entire window area of the found window.    Rectangle(hWindowDC, 0, 0,     rect.right - rect.left, rect.bottom - rect.top);
    // Reinsert the previous pen and brush     // into the found window's DC.    SelectObject(hWindowDC, hPrevPen);    SelectObject(hWindowDC, hPrevBrush);
    // Finally release the DC.    ReleaseDC(hwndFoundWindow, hWindowDC);  }   return lRet;}

long CWinSel::DoMouseUp(HWND hwndDialog, UINT message, WPARAM wParam, LPARAM lParam) {  long lRet = 0;
  // If we had a previous cursor, set the   // screen cursor to the previous one.  // The cursor is to stay exactly where it   // is currently located when the   // left mouse button is lifted.  if(g_hCursorPrevious) {     SetCursor(g_hCursorPrevious);   }
  // If there was a found window, refresh   // it so that its highlighting is erased.   if(g_hwndFoundWindow) {    RefreshWindow(g_hwndFoundWindow);
  }
  // Very important : must release the mouse capture.  ReleaseCapture();  // Set the global search window flag to FALSE.  g_bStartSearchWindow = FALSE;   return lRet;}

void CWinSel::SetFinderToolImage(HWND hwndDialog, BOOL full) {
  HBITMAP hBmpToSet = NULL;
  if(full) {
    // Set a FILLED image.
    hBmpToSet = g_hBitmapFinderToolFilled;  }
  else {
    // Set an EMPTY image.    hBmpToSet = g_hBitmapFinderToolEmpty;
  }
  SendDlgItemMessage     (    (HWND) hwndDialog,   // handle of dialog box
    (int)IDC_DRAGCONTROL, // identifier of control
    (UINT) STM_SETIMAGE,  // message to send
    (WPARAM) IMAGE_BITMAP,        // first message parameter
    (LPARAM) hBmpToSet    // second message parameter
    );
}

LRESULT CWinSel::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_MOUSEMOVE: {    if(g_bStartSearchWindow) {
      // Only when we have started the 
      // Window Searching operation will we 
      // track mouse movement.
      DoMouseMove(Window, message, wParam, lParam);    }    return TRUE;    break;  }
  case WM_LBUTTONUP: {    if(g_bStartSearchWindow)    {
      // Only when we have started the 
      // window searching operation will we
      // be interested when the user lifts 
      // up the left mouse button.
      DoMouseUp(Window, message, wParam, lParam);      SetFinderToolImage(Window, TRUE);     }    return TRUE;    break;     }
  case WM_INITDIALOG:
    {
      if(!m_hwnd) {             // If this is the initial dialog for the first time
        m_hwnd = Window;
      }
      return TRUE;
      break;
    }
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case (IDC_DISPLAY):
      if(HIWORD(wParam) == EN_CHANGE) {
        HWND Control = GetDlgItem(Window, IDC_DISPLAY);
        LRESULT BufferLength = SendMessage(Control, WM_GETTEXTLENGTH, 0, 0) + 1;        // +1 to allow for terminator
        TCHAR *Buffer = new TCHAR[BufferLength];
        SendMessage(Control, WM_GETTEXT, BufferLength, (LPARAM) Buffer);
        std::string ItemName;
        SendMessage(GetDlgItem(Window, IDC_TEXT), WM_SETTEXT, 0, (LPARAM) Buffer);
        delete[]Buffer;
      }
      break;
    case (IDOK):
      m_pEdit->SetWindow(g_hwndFoundWindow);
      // Fall through 
    case (IDCANCEL):
      {
        EndDialog(Window, LOWORD(wParam));
        return TRUE;
      }
    case IDC_DRAGCONTROL:
      {
        SearchWindow(Window);
        return TRUE;
        break;
      }
    }
  default:
    return FALSE;
  }
}
