// DasherView.inl
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////


inline void Dasher::CDasherView::Render()
{
	m_Screen->Blank();
	
	// Render nodes to screen object (should use off screen buffer)
	RecursiveRender(m_DasherModel.Root(), m_DasherModel.Rootmin(), m_DasherModel.Rootmax(), 2, false);
	RecursiveRender(m_DasherModel.Root(), m_DasherModel.Rootmin(), m_DasherModel.Rootmax(), 2, true);
	Crosshair(m_DasherModel.DasherOX()); // add crosshair
//	m_Screen->Display();                 // copy from buffer to screen
}

inline void Dasher::CDasherView::MapScreen(int* DrawX, int* DrawY)
{
	using namespace Dasher::Opts;
	
	switch (ScreenOrientation) {
		case (LeftToRight):
			break;
		case (RightToLeft):
			*DrawX = m_Screen->GetWidth() - *DrawX;
			break;
		case (TopToBottom): {
			int Swapper = ( *DrawX * m_Screen->GetHeight()) / m_Screen->GetWidth();
			*DrawX = (*DrawY  * m_Screen->GetWidth()) / m_Screen->GetHeight();
			*DrawY = Swapper;
			break;
			}
		case (BottomToTop): {
			// Note rotation by 90 degrees not reversible like others
			int Swapper = m_Screen->GetHeight() - ( *DrawX * m_Screen->GetHeight()) / m_Screen->GetWidth();
			*DrawX = (*DrawY  * m_Screen->GetWidth()) / m_Screen->GetHeight();
			*DrawY = Swapper;
			break;
			}
		default:
			break;
	}
}


inline void Dasher::CDasherView::UnMapScreen(int* MouseX, int* MouseY)
{
	using namespace Dasher::Opts;
	
	switch (ScreenOrientation) {
		case (LeftToRight):
			break;
		case (RightToLeft):
			*MouseX = m_Screen->GetWidth() - *MouseX;
			break;
		case (TopToBottom): {
			int Swapper = (*MouseX * m_Screen->GetHeight()) / m_Screen->GetWidth();
			*MouseX = (*MouseY *m_Screen->GetWidth()) / m_Screen->GetHeight();;
			*MouseY = Swapper;
			break;
			}
		case (BottomToTop): {
			int Swapper = (*MouseX * m_Screen->GetHeight()) / m_Screen->GetWidth();
			*MouseX = ((m_Screen->GetHeight() - *MouseY) * m_Screen->GetWidth()) / m_Screen->GetHeight();
			*MouseY = Swapper;
			break;
			}
		default:
			break;
	}
}
