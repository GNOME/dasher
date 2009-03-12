#include "KeyboardHelper.h"
#define KEYCODE_UP 63232
#define KEYCODE_DOWN 63233
#define KEYCODE_LEFT 63233
#define KEYCODE_RIGHT 63234

/* TODO: This file essentially the same as the Win32 code - 'twas copied from there, and
   only very slightly modified. Even the Win32 code contained the comment:
// TODO: This is essentially the same as the GTK code - should probably share them a little more
  so we should really combine all three---
  i.e. put some kind of KeyboardHelper into DasherCore, or refactor interpretation of keycodes.
*/

CKeyboardHelper::CKeyboardHelper() {
  // TODO: Hard code the mappings for now (slightly differently to Win32!)

  // 1 = E
  // 2 = N
  // 3 = W
  // 4 = S

  // Left of keyboard

  m_mTable['a'] = m_mTable['A'] = 1; //a
  m_mTable['s'] = m_mTable['S'] = 3; // s
  m_mTable['w'] = m_mTable['W'] = 2;// w
  m_mTable['z'] = m_mTable['Z'] = 4;// z

  // Right of keyboard

  m_mTable['j'] = m_mTable['J'] = 1; //a
  m_mTable['k'] = m_mTable['K'] = 3; // s
  m_mTable['i'] = m_mTable['I'] = 2;// w
  m_mTable['m'] = m_mTable['M'] = 4;// z
/*  m_mTable[0x4a] = 1; // j
  m_mTable[0x4b] = 3; // k
  m_mTable[0x49] = 2; // i
  m_mTable[0x4d] = 4; // m
*/
  // Arrows

  m_mTable[KEYCODE_LEFT] = 1;
  m_mTable[KEYCODE_RIGHT] = 3;
  m_mTable[KEYCODE_UP] = 2;
  m_mTable[KEYCODE_DOWN] = 4;

  // Arrows on numeric keypad
  //TODO, don't have one on my MacBook, so not sure what codes to use!
  /*m_mTable[VK_NUMPAD4] = 1;
  m_mTable[VK_NUMPAD6] = 3;
  m_mTable[VK_NUMPAD8] = 2;
  m_mTable[VK_NUMPAD2] = 4;*/

  // Numbers

  m_mTable[0x31] = 1; // 11
  m_mTable[0x32] = 2; // 2
  m_mTable[0x33] = 3; // 3
  m_mTable[0x34] = 4; // 4

  // 0 = keyboard start/stop

  m_mTable[' '] = 0;
}

int CKeyboardHelper::ConvertKeyCode(int iCode) {
  std::map<int, int>::iterator it(m_mTable.find(iCode));

  if(it == m_mTable.end())
    return -1;
  else
    return it->second;
}