// DasherView.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "DasherView.h"
using namespace Dasher;


CDasherView::CDasherView(CDasherScreen* DasherScreen, CDasherModel& DasherModel, Opts::ScreenOrientations Orientation)
  : m_Screen(DasherScreen), m_DasherModel(DasherModel), ScreenOrientation(Orientation), ColourMode(false)
{
//	XYScale = (double)m_Screen->GetHeight() / m_Screen->GetWidth();
}


void CDasherView::ChangeOrientation(Dasher::Opts::ScreenOrientations Orientation)
{
	ScreenOrientation = Orientation;
	Render();
}


void CDasherView::FlushAt(int mousex,int mousey) 
{
	m_DasherModel.Flush(0,0);
}

int CDasherView::RecursiveRender(CDasherNode* Render, myint y1,myint y2,int mostleft, bool text)
{
	symbol CurChar = Render->Symbol();
	int Color;

	if (ColourMode==true) {
	  Color = Render->Colour();
	} else {
	  Color = Render->Phase()%3; 
	}

	if (RenderNode(Render->Symbol(), Color, Render->Cscheme(), y1, y2, mostleft, Render->m_bForce, text))
		RenderGroups(Render, y1, y2, text);
	else
		Render->Kill();
	
	CDasherNode** const Children=Render->Children();
	if (!Children)
	  return 0;
	int norm=DasherModel().Normalization();
		for (unsigned int i=1; i<Render->Chars(); i++) {
		if (Children[i]->Alive()) {
			myint Range=y2-y1;
			myint newy1=y1+(Range*Children[i]->Lbnd())/norm;
			myint newy2=y1+(Range*Children[i]->Hbnd())/norm;
			RecursiveRender(Children[i], newy1, newy2, mostleft, text);
		}
	}
	return 1;


}


void CDasherView::RenderGroups(CDasherNode* Render, myint y1, myint y2, bool text)
{
	CDasherNode** Children = Render->Children();
	if (!Children)
		return;
	int current=0;
	int lower=0;
	int upper=0;
	myint range=y2-y1;
	for (unsigned int i=1; i<Render->Chars(); i++) {
		int g=Children[i]->Group();
		if (g!=current) {
			lower=upper;
			upper=i;
			
			if (current!=0) {
				myint lbnd=Children[lower]->Lbnd();
				myint hbnd=Children[upper]->Lbnd();
				myint newy1=y1+(range*lbnd)/m_DasherModel.Normalization();
				myint newy2=y1+(range*hbnd)/m_DasherModel.Normalization();
				int mostleft;
				bool force;
				RenderNode(0,current-1,Opts::Groups,newy1,newy2,mostleft,force,text);
			}
			current=g;
		}
	}
}
