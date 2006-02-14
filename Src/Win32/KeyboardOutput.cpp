#include "../Common/Common.h"
#include "../DasherCore/DasherTypes.h"
#include "KeyboardOutput.h"

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
const char *DisplayText(strText.c_str());

INPUT fakekey[2];

    if( DisplayText[0]==0xd && DisplayText[1]==0xa) {
			// Newline, so we want to fake an enter
			fakekey[0].type=fakekey[1].type=INPUT_KEYBOARD;
			fakekey[0].ki.wVk=fakekey[1].ki.wVk=VK_RETURN;
			fakekey[0].ki.time=fakekey[1].ki.time=0;
			fakekey[1].ki.dwFlags=KEYEVENTF_KEYUP;

			SendInput(2,fakekey,sizeof(INPUT));
		}
		wchar_t outputstring[256];
		int i=mbstowcs(outputstring,DisplayText,255);

		for (int j=0; j<i; j++) {
			fakekey[0].type=INPUT_KEYBOARD;
#ifdef DASHER_WINCE
			fakekey[0].ki.dwFlags=KEYEVENTF_KEYUP;
#else
			fakekey[0].ki.dwFlags=KEYEVENTF_UNICODE;
#endif
			fakekey[0].ki.wVk=0;
			fakekey[0].ki.time=NULL;
			fakekey[0].ki.wScan=outputstring[j];
			SendInput(1,fakekey,sizeof(INPUT));
		}
}

void CKeyboardOutput::Delete(std::string strText) {
    INPUT fakekey[2];

  	fakekey[0].type=fakekey[1].type=INPUT_KEYBOARD;
		fakekey[0].ki.wVk=fakekey[1].ki.wVk=VK_BACK;
		fakekey[0].ki.time=fakekey[1].ki.time=0;
		fakekey[1].ki.dwFlags=KEYEVENTF_KEYUP;
    fakekey[0].ki.dwFlags=0;
		SendInput(2,fakekey,sizeof(INPUT));
}
