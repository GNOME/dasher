#include ".\dashermouseinput.h"

using namespace Dasher;

CDasherMouseInput::CDasherMouseInput( HWND _hwnd ) : m_hwnd( _hwnd )
{
}

CDasherMouseInput::~CDasherMouseInput(void)
{
}

int CDasherMouseInput::GetCoordinates( int iN, myint *pCoordinates ) { 
		
	POINT mousepos;		
	GetCursorPos(&mousepos);

	ScreenToClient(m_hwnd,&mousepos);	

	pCoordinates[0] = mousepos.x;
    pCoordinates[1] = mousepos.y;

	return 0; 
}