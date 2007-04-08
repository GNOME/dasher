#include "Panel.h"

Panel::Panel(HWND Hpanel):m_hwnd(Hpanel) {
}

void Panel::Move(int x, int y, int width, int height) {
  MoveWindow(m_hwnd, x, y, width, height, TRUE);
}

void Panel::Paint() {
  SendMessage(m_hwnd, WM_PAINT, 0, 0);
}
