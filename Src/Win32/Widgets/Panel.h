#include <windows.h>

class Panel {
public:
  Panel(HWND Hpanel);
  void Move(int x, int y, int width, int height);
  void Paint();
private:
    HWND m_hwnd;
};
