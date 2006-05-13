#include "Common\WinCommon.h"
#include "KeyboardHelper.h"

// TODO: This is essentially the same as the GTK code - should probably share them a little more

CKeyboardHelper::CKeyboardHelper() {
  // TODO: Hard code the mappings for now, but eventually make these
  // configurable (at which point we shouldn't need to know about the
  // GDK constants.

  // 1 = E
  // 2 = N
  // 3 = W
  // 4 = S

  // Left of keyboard

  m_mTable[0x41] = 1; //a
  m_mTable[0x53] = 3; // s
  m_mTable[0x57] = 2;// w
  m_mTable[0x5a] = 4;// z

  // Right of keyboard

  m_mTable[0x4a] = 1; // j
  m_mTable[0x4b] = 3; // k
  m_mTable[0x49] = 2; // i
  m_mTable[0x4d] = 4; // m

  // Arrows

  m_mTable[VK_LEFT] = 1;
  m_mTable[VK_RIGHT] = 3;
  m_mTable[VK_UP] = 2;
  m_mTable[VK_DOWN] = 4;

  // Arrows on numeric keypad

  m_mTable[VK_NUMPAD4] = 1;
  m_mTable[VK_NUMPAD6] = 3;
  m_mTable[VK_NUMPAD8] = 2;
  m_mTable[VK_NUMPAD2] = 4;

  // Numbers

  m_mTable[0x31] = 1; // 11
  m_mTable[0x32] = 2; // 2
  m_mTable[0x33] = 3; // 3
  m_mTable[0x34] = 4; // 4

  // 0 = keyboard start/stop

  m_mTable[VK_SPACE] = 0;
}

int CKeyboardHelper::ConvertKeyCode(int iCode) {
  std::map<int, int>::iterator it(m_mTable.find(iCode));

  if(it == m_mTable.end())
    return -1;
  else
    return it->second;
}