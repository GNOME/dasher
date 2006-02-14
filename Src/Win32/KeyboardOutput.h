#ifndef __KEYBOARD_OUTPUT_H__
#define __KEYBOARD_OUTPUT_H__

#include <Windows.h>
#include <string>

namespace Dasher {
  class CEvent;
};

class CKeyboardOutput {
public:
  CKeyboardOutput() {
    m_bHaveTarget = false;
  };

  void HandleEvent(Dasher::CEvent * pEvent);
  void Output(std::string strText);
  void Delete(std::string strText);

  void SetTarget(HWND hTarget) {
    m_hTarget = hTarget;
    m_bHaveTarget = true;
  }

private:
  HWND m_hTarget;
  bool m_bHaveTarget;
};

#endif