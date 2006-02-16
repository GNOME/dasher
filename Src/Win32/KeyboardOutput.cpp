#include "../Common/Common.h"
#include "../DasherCore/DasherTypes.h"
#include "KeyboardOutput.h"
#include "../Common/WinUTF8.h"

#include "../DasherCore/Event.h"

void CKeyboardOutput::HandleEvent(Dasher::CEvent *pEvent) {
  if(pEvent->m_iEventType == 2) {
    Dasher::CEditEvent * pEvt(static_cast< Dasher::CEditEvent * >(pEvent));
    switch (pEvt->m_iEditType) {
    case 1:
      Output(pEvt->m_sText);
      break;
    case 2:
      Delete(pEvt->m_sText);
      break;
    }
  }
}

void CKeyboardOutput::Output(std::string strText) {
  std::wstring wstrText;
  WinUTF8::UTF8string_to_wstring(strText, wstrText);
  
  INPUT *fakekey;

  if( wstrText[0]==0xd && wstrText[1]==0xa) {
	  // Newline, so we want to fake an enter
    fakekey = new INPUT[2];

		fakekey[0].type=fakekey[1].type=INPUT_KEYBOARD;
		fakekey[0].ki.wVk=fakekey[1].ki.wVk=VK_RETURN;
		fakekey[0].ki.time=fakekey[1].ki.time=0;
		fakekey[0].ki.dwFlags=0;
    fakekey[1].ki.dwFlags=KEYEVENTF_KEYUP;
		
    SendInput(2, fakekey, sizeof(INPUT));
    delete[] fakekey;
	}
  else {
		fakekey = new INPUT[2*wstrText.size()];

		for (int j=0; j<wstrText.size(); j++) {
			fakekey[2*j].type=fakekey[2*j + 1].type=INPUT_KEYBOARD;
#ifdef DASHER_WINCE
			fakekey[2*j].ki.dwFlags=fakekey[2*j + 1].ki.dwFlags=KEYEVENTF_KEYUP;
#else
			fakekey[2*j].ki.dwFlags=KEYEVENTF_UNICODE;
      fakekey[2*j + 1].ki.dwFlags=KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
#endif
			fakekey[2*j].ki.wVk=fakekey[2*j+1].ki.wVk=0;
			fakekey[2*j].ki.time=fakekey[2*j+1].ki.time=NULL;
			fakekey[2*j].ki.wScan=fakekey[2*j+1].ki.wScan=wstrText[j];
		}

    SendInput(2*wstrText.size(), fakekey, sizeof(INPUT));
    delete[] fakekey;
  }
}

void CKeyboardOutput::Delete(std::string strText) {
  std::wstring wstrText;
  WinUTF8::UTF8string_to_wstring(strText, wstrText);

  int iCount;

  if( wstrText[0]==0xd && wstrText[1]==0xa)
    iCount = 1;
  else
    iCount = wstrText.size();

  INPUT *fakekey;
  fakekey = new INPUT[iCount * 2];

  for(int j(0); j < iCount; ++j) {
  	fakekey[2*j].type=fakekey[2*j + 1].type=INPUT_KEYBOARD;
		fakekey[2*j].ki.wVk=fakekey[2*j + 1].ki.wVk=VK_BACK;
		fakekey[2*j].ki.time=fakekey[2*j + 1].ki.time=0;
    fakekey[2*j].ki.dwFlags=0;
		fakekey[2*j + 1].ki.dwFlags=KEYEVENTF_KEYUP;
  }
		
  SendInput(iCount,fakekey,sizeof(INPUT));
  delete[] fakekey;
}
