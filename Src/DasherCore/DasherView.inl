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
	RecursiveRender(m_DasherModel.Root(), m_DasherModel.Rootmin(), m_DasherModel.Rootmax(), 2);
	
	Crosshair(m_DasherModel.Canvasox()); // add crosshair
	
	m_Screen->Display();                 // copy from buffer to screen
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
			int Swapper = static_cast<int>(*DrawX * XYScale);
			*DrawX = static_cast<int>(*DrawY / XYScale);
			*DrawY = Swapper;
			break;
			}
		case (BottomToTop): {
			// Note rotation by 90 degrees not reversible like others
			int Swapper = m_Screen->GetHeight() - static_cast<int>(*DrawX * XYScale);
			*DrawX = static_cast<int>(*DrawY / XYScale);
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
			int Swapper = static_cast<int>(*MouseX * XYScale);
			*MouseX = static_cast<int>(*MouseY / XYScale);
			*MouseY = Swapper;
			break;
			}
		case (BottomToTop): {
			int Swapper = static_cast<int>(*MouseX * XYScale);
			*MouseX = static_cast<int>((m_Screen->GetHeight() - *MouseY) / XYScale);
			*MouseY = Swapper;
			break;
			}
		default:
			break;
	}
}