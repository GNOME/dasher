#ifndef __KEYBOARD_HELPER_H__
#define __KEYBOARD_HELPER_H__

#include <map>

class CKeyboardHelper {
public:
  CKeyboardHelper();
  int ConvertKeyCode(int iCode);

private:
  std::map<int, int> m_mTable;
};

#endif