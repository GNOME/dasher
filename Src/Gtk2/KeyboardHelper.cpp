#include "KeyboardHelper.h"

#include <gdk/gdkkeysyms.h>

CKeyboardHelper::CKeyboardHelper() {
  // TODO: Hard code the mappings for now, but eventually make these
  // configurable (at which point we shouldn't need to know about the
  // GDK constants.

  // 1 = E
  // 2 = W
  // 3 = N
  // 4 = S

  // Left of keyboard

  m_mTable[GDK_a] = 1;
  m_mTable[GDK_s] = 2;
  m_mTable[GDK_w] = 3;
  m_mTable[GDK_z] = 4;
  
  // Right of keyboard

  m_mTable[GDK_j] = 1;
  m_mTable[GDK_k] = 2;
  m_mTable[GDK_i] = 3;
  m_mTable[GDK_m] = 4;

  // Arrows

  m_mTable[GDK_Left] = 1;
  m_mTable[GDK_Right] = 2;
  m_mTable[GDK_Up] = 3;
  m_mTable[GDK_Down] = 4;

  // Arrows on numeric keypad

  m_mTable[GDK_KP_Left] = 1;
  m_mTable[GDK_KP_Right] = 2;
  m_mTable[GDK_KP_Up] = 3;
  m_mTable[GDK_KP_Down] = 4;

  // Numbers

  m_mTable[GDK_1] = 1;
  m_mTable[GDK_2] = 2;
  m_mTable[GDK_3] = 3;
  m_mTable[GDK_4] = 4;
  
  // 0 = keyboard start/stop

  m_mTable[GDK_space] = 0;
}

int CKeyboardHelper::ConvertKeycode(int iCode) {

  std::map<int,int>::iterator it(m_mTable.find(iCode));

  if(it != m_mTable.end())
    return it->second;
  else
    return -1;
}
