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
			int Swapper = *DrawX * XYScale;
			*DrawX = *DrawY / XYScale;
			*DrawY = Swapper;
			break;
			}
		case (BottomToTop): {
			// Note rotation by 90 degrees not reversible like others
			int Swapper = m_Screen->GetHeight() - *DrawX * XYScale;
			*DrawX = *DrawY / XYScale;
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
			int Swapper = *MouseX * XYScale;
			*MouseX = *MouseY / XYScale;
			*MouseY = Swapper;
			break;
			}
		case (BottomToTop): {
			int Swapper = *MouseX * XYScale;
			*MouseX = (m_Screen->GetHeight() - *MouseY) / XYScale;
			*MouseY = Swapper;
			break;
			}
		default:
			break;
	}
}