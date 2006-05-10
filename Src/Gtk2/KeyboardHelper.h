#ifndef __KEYBOSRD_HELPER_H__
#define __KEYBOSRD_HELPER_H__

#include <map>

class CKeyboardHelper {
 public:
  CKeyboardHelper();

  int ConvertKeycode(int iCode);

 private:
  std::map<int, int> m_mTable;
};

#endif
