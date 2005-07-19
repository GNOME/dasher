/*
	Short program to empty Windows clipbaord.
	Useful for calling in scripts when a simple but hacky solution is required.
	
	IAM 09/2002
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  OpenClipboard(NULL);
  EmptyClipboard();
  CloseClipboard();
  return 0;
}
