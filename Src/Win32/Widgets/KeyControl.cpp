// KeyControl.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002 Iain Murray, Inference Group, Cavendish, Cambridge.
//
/////////////////////////////////////////////////////////////////////////////

#include "WinCommon.h"

#include "KeyControl.h"
#include "../resource.h"
#include "../Dasher.h"

#include <utility>              // for std::pair
//#include <sstream>

using namespace Dasher;
using namespace std;


// FIXME - lines marked //F need to be reimplemented using the new scheme

CKeyBox::CKeyBox(HWND Parent, CDasher *pDasher)
:m_pDasher(pDasher), NewUniform(-1) 
{
//      m_sBuffer.resize(1000);
  m_hwnd = 0;
  DialogBoxParam(WinHelper::hInstApp, (LPCTSTR) IDD_KEYCONTROL1, Parent, (DLGPROC) WinWrapMap::WndProc, (LPARAM) this);
}

std::string CKeyBox::GetControlText(HWND Dialog, int ControlID) {
  HWND Control = GetDlgItem(Dialog, ControlID);
  LRESULT BufferLength = SendMessage(Control, WM_GETTEXTLENGTH, 0, 0) + 1;      // +1 to allow for terminator
  TCHAR *Buffer = new TCHAR[BufferLength];
  SendMessage(Control, WM_GETTEXT, BufferLength, (LPARAM) Buffer);
  string ItemName;
  WinUTF8::wstring_to_UTF8string(Buffer, ItemName);
  delete[]Buffer;
  return ItemName;
}

void CKeyBox::PopulateWidgets() {
//      int widgets[18];
//      char dummybuffer[256];
//      wchar_t widebuffer[256];

/*	widgets[0]=IDC_UPX;
	widgets[1]=IDC_UPY;
	widgets[2]=IDC_DOWNX;
	widgets[3]=IDC_DOWNY;
	widgets[4]=IDC_LEFTX;
	widgets[5]=IDC_LEFTY;
	widgets[6]=IDC_RIGHTX;
	widgets[7]=IDC_RIGHTY;
	widgets[8]=IDC_5X;
	widgets[9]=IDC_5Y;
	widgets[10]=IDC_6X;
	widgets[11]=IDC_6Y;
	widgets[12]=IDC_7X;
	widgets[13]=IDC_7Y;
	widgets[14]=IDC_8X;
	widgets[15]=IDC_8Y;
	widgets[16]=IDC_9X;
	widgets[17]=IDC_9Y;

	int* coords = m_pCanvas->getkeycoords(); */


 

//      std::basic_ostringstream<TCHAR> strUniform;
//      strUniform.setf(ios::fixed);
//      strUniform.precision(1);
//      strUniform << m_pCanvas->getuniform()/10.0;

// FIXME - why do we store things like 'uniform' here - surely they should be stored in the interface?

  

/*	for (int i=0; i<18; i++) {
		EditBox = GetDlgItem(m_hwnd, widgets[i]);
		SendMessage(EditBox, LB_RESETCONTENT, 0, 0);
	
		itoa(keycoords[i],dummybuffer,10);
		mbstowcs(widebuffer,dummybuffer,256);

		SendMessage(EditBox, WM_SETTEXT, 0, (LPARAM) widebuffer);
	}
	if (m_pCanvas->getforward()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_KCFORWARD), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pCanvas->getbackward()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_KCBACK), BM_SETCHECK, BST_CHECKED, 0);
	}
	if (m_pCanvas->getselect()==true) {
		SendMessage(GetDlgItem(m_hwnd, IDC_KCSELECT), BM_SETCHECK, BST_CHECKED, 0);
	} */
}

LRESULT CKeyBox::WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_INITDIALOG:
    {
      if(!m_hwnd) {             // If this is the initial dialog for the first time
        m_hwnd = Window;
        PopulateWidgets();
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
        string ItemName;
        SendMessage(GetDlgItem(Window, IDC_TEXT), WM_SETTEXT, 0, (LPARAM) Buffer);
        delete[]Buffer;
      }
      break;
    case (IDOK_KEYCONT):
/*			keycoords[0]=atoi(GetControlText(Window,IDC_UPX).c_str());
			keycoords[1]=atoi(GetControlText(Window,IDC_UPY).c_str());
			keycoords[2]=atoi(GetControlText(Window,IDC_DOWNX).c_str());
			keycoords[3]=atoi(GetControlText(Window,IDC_DOWNY).c_str());
			keycoords[4]=atoi(GetControlText(Window,IDC_LEFTX).c_str());
			keycoords[5]=atoi(GetControlText(Window,IDC_LEFTY).c_str());
			keycoords[6]=atoi(GetControlText(Window,IDC_RIGHTX).c_str());
			keycoords[7]=atoi(GetControlText(Window,IDC_RIGHTY).c_str());
			keycoords[8]=atoi(GetControlText(Window,IDC_5X).c_str());
			keycoords[9]=atoi(GetControlText(Window,IDC_5Y).c_str());
			keycoords[10]=atoi(GetControlText(Window,IDC_6X).c_str());
			keycoords[11]=atoi(GetControlText(Window,IDC_6Y).c_str());
			keycoords[12]=atoi(GetControlText(Window,IDC_7X).c_str());
			keycoords[13]=atoi(GetControlText(Window,IDC_7Y).c_str());
			keycoords[14]=atoi(GetControlText(Window,IDC_8X).c_str());
			keycoords[15]=atoi(GetControlText(Window,IDC_8Y).c_str());
			keycoords[16]=atoi(GetControlText(Window,IDC_9X).c_str());
			keycoords[17]=atoi(GetControlText(Window,IDC_9Y).c_str()); */
     
      if(NewUniform != -1) {
        // DJW - this looks a bit nasty
        m_pDasher->SetLongParameter(LP_UNIFORM, static_cast < long >(NewUniform));

      }
      // Move forward on button press
/*			if (SendMessage(GetDlgItem(Window,IDC_KCFORWARD), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pCanvas->setforward(true);
			} else {
				m_pCanvas->setforward(false);
			}
			// Move backward on button press
			if (SendMessage(GetDlgItem(Window,IDC_KCBACK), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pCanvas->setbackward(true);
			} else {
				m_pCanvas->setbackward(false);
			}
			// Select on button press
			if (SendMessage(GetDlgItem(Window,IDC_KCSELECT), BM_GETCHECK, 0, 0)==BST_CHECKED) {
				m_pCanvas->setselect(true);
			} else {
				m_pCanvas->setselect(false);
			} */
      return TRUE;
      break;
    case (IDCANCEL):
      {
        EndDialog(Window, LOWORD(wParam));
        return TRUE;
      }
    case ID_CANCEL_KEYCONT:
      EndDialog(Window, LOWORD(wParam));
      return TRUE;
      break;
    }
  default:
    return FALSE;
  }
}
