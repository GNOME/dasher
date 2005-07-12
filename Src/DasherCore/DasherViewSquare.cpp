// DasherViewSquare.cpp
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "DasherViewSquare.h"
#include "DasherModel.h"

#include <iostream>
#include <algorithm>

using namespace Dasher;

// FIXME - quite a lot of the code here probably should be moved to
// the parent class (DasherView). I think we really should make the
// parent class less general - we're probably not going to implement
// anything which uses radically different co-ordinate transforms, and
// we can always override if necessary.

/////////////////////////////////////////////////////////////////////////////

/* render everything between rootmin and rootmax */

void CDasherViewSquare::RenderNodes()
{
  Screen().Blank();
  
  DASHER_ASSERT(DasherModel().Root()!=0);
  
  //DASHER_TRACEOUTPUT("RenderNodes\n");
  
  // Render nodes to screen object (should use off screen buffer)
  
  RecursiveRender(DasherModel().Root(), DasherModel().Rootmin(), DasherModel().Rootmax(), DasherVisibleMaxX());
  
  // DelayDraw the text nodes
  m_DelayDraw.Draw(Screen());

  Crosshair(DasherModel().DasherOX()); // add crosshair
}

/////////////////////////////////////////////////////////////////////////////

int CDasherViewSquare::RecursiveRender(CDasherNode* pRender, myint y1,myint y2,int mostleft)
{
	DASHER_ASSERT_VALIDPTR_RW(pRender);

	// Decide which colour to use when rendering the child

	int Color;

  	if (ColourMode==true) 
	{
	  if (pRender->Colour()!=-1) {
	    Color = pRender->Colour();
	  } else {
	    if (pRender->Symbol()==DasherModel().GetSpaceSymbol()) {
	      Color = 9;
	    } else if (pRender->Symbol()==DasherModel().GetControlSymbol()) {
	      Color = 8;
	    } else {
	      Color = (pRender->Symbol()%3)+10;
	    }
	  }
	} else {
	  Color = pRender->Phase()%3; 
	}

	if ((pRender->ColorScheme()%2)==1 && Color<130 && ColourMode==true) { // We don't loop on high
	  Color+=130;                                // colours
	}

	//	DASHER_TRACEOUTPUT("%x ",Render);
	
	std::string display;
	if (pRender->GetControlTree()!=NULL) 
		display = pRender->GetControlTree()->text;

	if (RenderNode(pRender->Symbol(), Color, pRender->ColorScheme(), y1, y2, mostleft, display))
	{
		// yuk
		if (!pRender->ControlChild() && pRender->Symbol() < DasherModel().GetAlphabet().GetNumberTextSymbols() )
			RenderGroups(pRender, y1, y2);
	}
	else
	{
		pRender->Kill();
		return 0;
	}

	int iChildCount = pRender->ChildCount();
	if (!iChildCount)
	  return 0;

	int norm=DasherModel().Normalization();
	for (int i=0; i< iChildCount; i++) 
	{
		CDasherNode* pChild = pRender->Children()[i];
		if ( pChild->Alive() ) 
		{
			myint Range=y2-y1;
			myint newy1=y1+(Range * pChild->Lbnd() )/norm;
			myint newy2=y1+(Range * pChild->Hbnd() )/norm;
			RecursiveRender(pChild, newy1, newy2, mostleft);
		}
	}
	return 1;


}

/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::RenderGroups(CDasherNode* Render, myint y1, myint y2)
{
	CDasherNode** Children = Render->Children();
	if (!Children)
		return;
	int current=0;
	int lower=0;
	int upper=0;
    std::string Label="";

	myint range=y2-y1;
	
	const CAlphabet& alphabet = DasherModel().GetAlphabet();

	for (int iGroup=1; iGroup < alphabet.GetGroupCount();  iGroup++) 
	{
		int lower = alphabet.GetGroupStart(iGroup);
		int upper = alphabet.GetGroupEnd(iGroup);
	
		myint lbnd=Children[lower]->Lbnd();
		myint hbnd=Children[upper-1]->Hbnd();
		myint newy1=y1+(range*lbnd)/DasherModel().Normalization();
		myint newy2=y1+(range*hbnd)/DasherModel().Normalization();
		int mostleft;
		if (ColourMode==true) 
		{
			std::string Label = DasherModel().GroupLabel(iGroup);
			int Colour = DasherModel().GroupColour(iGroup);
                  
            if (Colour!=-1) 
			{
				RenderNode(0,DasherModel().GroupColour(iGroup),Opts::Groups,newy1,newy2,mostleft,Label);
			} 
			else 
			{
			    RenderNode(0,(current%3)+110,Opts::Groups,newy1,newy2,mostleft,Label);
			}
		} 
		else 
		{
			RenderNode(0,current-1,Opts::Groups,newy1,newy2,mostleft,Label);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

CDasherViewSquare::Cymap::Cymap(myint iScale)
{
	double dY1=0.25;    // Amount of acceleration
	double dY2=0.95;    // Accelerate Y movement below this point
	double dY3=0.05;    // Accelerate Y movement above this point

	m_Y2=myint (dY2 * iScale );
	m_Y3=myint (dY3 * iScale );
	m_Y1=myint(1.0/dY1);
}

/////////////////////////////////////////////////////////////////////////////

CDasherViewSquare::CDasherViewSquare(CDasherScreen* DasherScreen, CDasherModel& DasherModel, Dasher::Opts::ScreenOrientations Orientation, bool Colourmode)
  : CDasherView(DasherScreen, DasherModel, Orientation, Colourmode)
{
	ChangeScreen(DasherScreen);
	
	// tweak these if you know what you are doing
	m_dXmpa=0.2;   // these are for the x non-linearity
	m_dXmpb=0.5;
	m_dXmpc=0.9;
	m_dXmpd=0.5;   // slow X movement when accelerating Y


	KeyControl=false;

	onebutton=-2000;

	m_ymap = Cymap(DasherModel.DasherY());

	CDasherModel::CRange rActive(m_ymap.unmap(0), m_ymap.unmap(DasherModel.DasherY()) );
	DasherModel.SetActive(rActive);
}

/////////////////////////////////////////////////////////////////////////////

int CDasherViewSquare::RenderNode(const symbol Character, const int Color, Opts::ColorSchemes ColorScheme,
	myint y1, myint y2, int& mostleft, const std::string& displaytext)
{
	DASHER_ASSERT(y2>=y1);

// //	DASHER_TRACEOUTPUT("RenderNode Symbol:%d Colour:%d, ColourScheme:%d Display:%s \n",Character,Color,ColorScheme,displaytext.c_str());
// 	//DASHER_TRACEOUTPUT("RenderNode %I64d %I64d",y1,y2);

// 	// Get the screen positions of the node in co-ords such that dasher RHS runs from 0 to DasherModel.DasherY
// 	screenint s1,s2;
// 	Cint32 iSize = dashery2screen(y1,y2,s1,s2);

// 	// Actual height in pixels
// 	Cint32 iHeight = Cint32( (Cint32) (iSize * CanvasY)/ (Cint32) DasherModel().DasherY() );

// 	if (iHeight <=1)
// 		return 0;

// 	// horizontal width of the square is controlled by the true size (y2-y1) in Dasher world

		
// 	// All squares are right-aligned.
// 	screenint iRight=CanvasX;
		
// 	screenint iNewleft=iLeft, iNewtop=s1, iNewright=iRight, iNewbottom=s2;

// 	// Do the rotation
// 	MapScreen(&iNewleft, &iNewtop);
// 	MapScreen(&iNewright, &iNewbottom);

//	DASHER_TRACEOUTPUT("--------- %i %i\n",iNewtop,iNewbottom);

	//Screen().DrawRectangle(iNewleft, iNewtop, iNewright, iNewbottom, Color, ColorScheme);


	// FIXME - Get sensibel limits here (to allow for non-linearities)

 	screenint s1,s2;
 	Cint32 iSize = dashery2screen(y1,y2,s1,s2);

 	// Actual height in pixels
 	Cint32 iHeight = Cint32( (Cint32) (iSize * CanvasY)/ (Cint32) DasherModel().DasherY() );

	if( iHeight <= 1 )
	  return 0; // We're too small to render

	if(( y1 > DasherVisibleMaxY() ) || ( y2 < DasherVisibleMinY() ))
	  return 0; // We're entirely off screen, so don't render.

	myint iDasherSize( y2 - y1 );
#
	// FIXME - get rid of pointless assignment below

	int iTruncation( m_iTruncation ); // Trucation farction times 100;
	int iTruncationType( m_iTruncationType);

	if( iTruncation == 0 ) { // Regular squares
	  DasherDrawRectangle( iDasherSize, y2, 0, y1, Color, ColorScheme );
	}
	else {
	  int iDasherY( DasherModel().DasherY() );

	  int iSpacing( iDasherY / 128 ); // FIXME - assuming that this is an integer below


	  int iXStart;
	  
	  switch( iTruncationType ) {
	  case 0:
	    iXStart = 0;
	    break;
	  case 1:
	    iXStart = iSize - iSize * iTruncation / 200;
	    break;
	  case 2:
	    iXStart = iSize - iSize * iTruncation / 100;
	    break;
	  }


	  int iTipMin( (y2 - y1) * iTruncation / (200) + y1 );
	  int iTipMax( y2 - (y2 - y1) * iTruncation / (200) );
	  
	  

	  int iLowerMin( (y1 / iSpacing)*iSpacing );
	  int iLowerMax( (iTipMin / iSpacing )*iSpacing );

	  int iUpperMin( (iTipMax / iSpacing )*iSpacing );
	  int iUpperMax( (y2 / iSpacing)*iSpacing );

	  if( iLowerMin < 0 )
	    iLowerMin = 0;
	  
	  if( iLowerMax < 0 )
	    iLowerMax = 0;

	  if( iUpperMin < 0 )
	    iUpperMin = 0;
	  
	  if( iUpperMax < 0 )
	    iUpperMax = 0;
	  

	  if( iLowerMin > iDasherY )
	    iLowerMin = iDasherY;
	  
	  if( iLowerMax > iDasherY )
	    iLowerMax = iDasherY;

	  if( iUpperMin > iDasherY )
	    iUpperMin = iDasherY;
	  
	  if( iUpperMax > iDasherY )
	    iUpperMax = iDasherY;

	  while( iLowerMin < y1 ) 
	    iLowerMin += iSpacing;

	  while( iLowerMax > iTipMin)
	    iLowerMax -= iSpacing;
	   
	  while( iUpperMin < iTipMax )
	    iUpperMin += iSpacing;

	  while( iUpperMax > y2 )
	    iUpperMax -= iSpacing;


	  int iLowerCount( (iLowerMax - iLowerMin) / iSpacing + 1 );
	  int iUpperCount( (iUpperMax - iUpperMin) / iSpacing + 1 );

	  if( iLowerCount < 0 )
	    iLowerCount = 0;
	  
	  if( iUpperCount < 0 )
	    iUpperCount = 0;

	  int iTotalCount( iLowerCount + iUpperCount + 6 );

	  myint *x( new myint[iTotalCount] );
	  myint *y( new myint[iTotalCount] );

	  // Weird duplication here is to make truncated squares possible too

	  x[0] = 0;
	  y[0] = y1;
	  x[1] = iXStart;
	  y[1] = y1;

	  x[iLowerCount + 2] = iDasherSize;
	  y[iLowerCount + 2] = iTipMin;
	  x[iLowerCount + 3] = iDasherSize;
	  y[iLowerCount + 3] = iTipMax;

	  x[iTotalCount - 2] = iXStart;
	  y[iTotalCount - 2] = y2;
	  x[iTotalCount - 1] = 0;
	  y[iTotalCount - 1] = y2;

	  for( int i(0); i < iLowerCount; ++i ) {
	    x[i+2] = (iLowerMin + i * iSpacing - y1) * (iDasherSize - iXStart) / (iTipMin - y1) + iXStart;
	    y[i+2] = iLowerMin + i * iSpacing;
	  }

	  for( int i(0); i < iUpperCount; ++i ) {
	    x[i+iLowerCount + 4] = (y2 - (iUpperMin + i * iSpacing)) * (iDasherSize - iXStart) / (y2 - iTipMax) + iXStart;
	    y[i+iLowerCount + 4] = iUpperMin + i * iSpacing;
	  }
	  
	  DasherPolygon( x, y, iTotalCount, Color );

	delete x;
	delete y;

	}
	
	myint iDasherAnchorX( iDasherSize );

	myint iDasherAnchorY( (std::min(y2 , DasherVisibleMaxY()) + std::max(y1, DasherVisibleMinY()))/2 );
	
	std::string sDisplayText;
	
	if( displaytext != std::string("") )
	  sDisplayText = displaytext;
	else
	  sDisplayText = DasherModel().GetDisplayText(Character);
	
	DasherDrawText( iDasherAnchorX, y1, iDasherAnchorX, y2, sDisplayText, mostleft );
	
	return 1;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherViewSquare::CheckForNewRoot()
{
	CDasherNode * const root=DasherModel().Root();
	CDasherNode ** const children=root->Children();


	myint y1=DasherModel().Rootmin();
	myint y2=DasherModel().Rootmax();

	// This says that the root node must enclose everything visible.
	// Tiny probability characters near the bottom will cause a problem
	// with forcing to reparent to the previous one.

	if ((y1>myint(0) || y2 < DasherModel().DasherY() || dasherx2screen(y2-y1)>0)) {
		DasherModel().Reparent_root(root->Lbnd(),root->Hbnd());
		return;
	}

	if (children==0)
		return;

	int alive=0;
	int theone=0;
	unsigned int i;

	// Find whether there is exactly one alive child; if more, we don't care.
	for (i=0;i<root->ChildCount();i++) 
	{
		if (children[i]->Alive()) 
		{
			alive++;
			theone=i;
            if(alive>1) break; 
		}
	}

	if (alive==1) 
	{	  
	// We must have zoomed sufficiently that only one child of the root node 
	// is still alive.  Let's make it the root.

	  y1=DasherModel().Rootmin();
	  y2=DasherModel().Rootmax();
	  myint range=y2-y1;

	  myint newy1=y1+(range*children[theone]->Lbnd())/DasherModel().Normalization();
	  myint newy2=y1+(range*children[theone]->Hbnd())/DasherModel().Normalization();
	  if (newy1<myint(0) && newy2> DasherModel().DasherY()) {
	    myint left=dasherx2screen(newy2-newy1);
	    if (left<myint(0)) {
	      DasherModel().Make_root(theone);
	      return;
	    }
	  }
	}
}

/// Convert screen co-ordinates to dasher co-ordinates. This doesn't
/// include the nonlinear mapping for eyetracking mode etc - it is
/// just the inverse of the mapping used to calculate the screen
/// positions of boxes etc.

void CDasherViewSquare::Screen2Dasher( screenint iInputX, screenint iInputY, myint &iDasherX, myint &iDasherY, bool b1D, bool bNonlinearity ) {

  // Things we're likely to need:

  myint iDasherWidth = DasherModel().DasherY();
  myint iDasherHeight = DasherModel().DasherY();

  myint iCanvasWidth = CanvasX;
  myint iCanvasHeight = CanvasY;

  screenint iScreenWidth = Screen().GetWidth();
  screenint iScreenHeight = Screen().GetHeight();
  

  if( b1D ) { // Special case for 1D mode...
    iDasherX = iInputX * iDasherWidth / iScreenWidth;
    iDasherY = iInputY * iDasherHeight / iScreenHeight;
    return;
  }

  // Calculate the bounding box of the Dasher canvas in screen
  // co-ordinates (this will depend on the orientation due to the
  // margin)
  //
  // FIXME - there's some horrible stuff here due to CanvasX and
  // CanvasY having no sane relationship with the actual width and
  // height of the canvas
  
  screenint iScreenCanvasMinX;
  screenint iScreenCanvasMaxX;
  screenint iScreenCanvasMinY;
  screenint iScreenCanvasMaxY;

 switch (ScreenOrientation) {
 case (Dasher::Opts::LeftToRight):
   iScreenCanvasMinX = 0;
   iScreenCanvasMaxX = iCanvasWidth;
   iScreenCanvasMinY = 0;
   iScreenCanvasMaxY = iCanvasHeight;

   // Chose iDasherWidth so that the cross hair is in the centre of the screen, even though we have a margin down one side
   //   iDasherWidth = (iDasherHeight * (iScreenCanvasMaxX - iScreenCanvasMinX)) / (2*(iScreenCanvasMaxX - iScreenCanvasMinX) - iScreenWidth);

   break;
 case (Dasher::Opts::RightToLeft):
   iScreenCanvasMinX = iScreenWidth - iCanvasWidth;
   iScreenCanvasMaxX = iScreenWidth;
   iScreenCanvasMinY = 0;
   iScreenCanvasMaxY = iCanvasHeight;

   //   iDasherWidth = (iDasherHeight * (iScreenCanvasMaxX - iScreenCanvasMinX)) / (2*(iScreenCanvasMaxX - iScreenCanvasMinX) - iScreenWidth);
   
   break;
 case (Dasher::Opts::TopToBottom):
   iScreenCanvasMinX = 0;
   iScreenCanvasMaxX = iScreenWidth;
   iScreenCanvasMinY = 0;
   iScreenCanvasMaxY = iScreenHeight * iCanvasWidth / iScreenWidth; // URGH

   //   iDasherWidth = (iDasherHeight * (iScreenCanvasMaxY - iScreenCanvasMinY)) / (2*(iScreenCanvasMaxY - iScreenCanvasMinY) - iScreenHeight);

   break;
 case (Dasher::Opts::BottomToTop):
   iScreenCanvasMinX = 0;
   iScreenCanvasMaxX = iScreenWidth;
   iScreenCanvasMinY = iScreenHeight - iScreenHeight * iCanvasWidth / iScreenWidth; // Still URGH
   iScreenCanvasMaxY = iScreenHeight; 

   //   iDasherWidth = (iDasherHeight * (iScreenCanvasMaxY - iScreenCanvasMinY)) / (2*(iScreenCanvasMaxY - iScreenCanvasMinY) - iScreenHeight);
   break;
 default:
   break;
 }

 // FIXME - everything above this line should be cached, as it only changes very rarely

 // Now we've got the bouding box, use the input positions relative to
 // the box to get the actual Dasher co-ordinates
 
 switch (ScreenOrientation) {
 case (Dasher::Opts::LeftToRight):
   iDasherX = (iScreenCanvasMaxX - iInputX) * iDasherWidth / (iScreenCanvasMaxX - iScreenCanvasMinX);
   iDasherY = (iInputY - iScreenCanvasMinY) * iDasherHeight / (iScreenCanvasMaxY - iScreenCanvasMinY);
   break;
 case (Dasher::Opts::RightToLeft):
   iDasherX = (iInputX - iScreenCanvasMinX) * iDasherWidth / (iScreenCanvasMaxX - iScreenCanvasMinX);
   iDasherY = (iInputY - iScreenCanvasMinY) * iDasherHeight / (iScreenCanvasMaxY - iScreenCanvasMinY);
   break;
 case (Dasher::Opts::TopToBottom):
   iDasherX = (iScreenCanvasMaxY - iInputY) * iDasherWidth / (iScreenCanvasMaxY - iScreenCanvasMinY);
   iDasherY = (iInputX - iScreenCanvasMinX) * iDasherHeight / (iScreenCanvasMaxX - iScreenCanvasMinX);
   break;
 case (Dasher::Opts::BottomToTop):
   iDasherX = (iInputY - iScreenCanvasMinY) * iDasherWidth / (iScreenCanvasMaxY - iScreenCanvasMinY);
   iDasherY = (iInputX - iScreenCanvasMinX) * iDasherHeight / (iScreenCanvasMaxX - iScreenCanvasMinX);
   break;
 default:
   break;
 }

 if( bNonlinearity ) {
   iDasherX = ixmap( iDasherX / static_cast<double>( DasherModel().DasherY() ) ) * DasherModel().DasherY();
   iDasherY = m_ymap.unmap( iDasherY );
 }

}

/// Convert dasher co-ordinates to screen co-ordinates

void CDasherViewSquare::Dasher2Screen( myint iDasherX, myint iDasherY, screenint &iScreenX, screenint &iScreenY ) {

  // Apply the nonlinearities

  iDasherX = xmap( iDasherX / static_cast<double>( DasherModel().DasherY() ) ) * DasherModel().DasherY();
  iDasherY = m_ymap.map( iDasherY );

  // Things we're likely to need:

  myint iDasherWidth = DasherModel().DasherY();
  myint iDasherHeight = DasherModel().DasherY();

  myint iCanvasWidth = CanvasX;
  myint iCanvasHeight = CanvasY;

  screenint iScreenWidth = Screen().GetWidth();
  screenint iScreenHeight = Screen().GetHeight();
  
  // Calculate the bounding box of the Dasher canvas in screen
  // co-ordinates (this will depend on the orientation due to the
  // margin)
  //
  // FIXME - there's some horrible stuff here due to CanvasX and
  // CanvasY having no sane relationship with the actual width and
  // height of the canvas
  
  screenint iScreenCanvasMinX;
  screenint iScreenCanvasMaxX;
  screenint iScreenCanvasMinY;
  screenint iScreenCanvasMaxY;

 switch (ScreenOrientation) {
 case (Dasher::Opts::LeftToRight):
   iScreenCanvasMinX = 0;
   iScreenCanvasMaxX = iCanvasWidth;
   iScreenCanvasMinY = 0;
   iScreenCanvasMaxY = iCanvasHeight;

   //   iDasherWidth = (iDasherHeight * (iScreenCanvasMaxX - iScreenCanvasMinX)) / (2*(iScreenCanvasMaxX - iScreenCanvasMinX) - iScreenWidth);

   break;
 case (Dasher::Opts::RightToLeft):
   iScreenCanvasMinX = iScreenWidth - iCanvasWidth;
   iScreenCanvasMaxX = iScreenWidth;
   iScreenCanvasMinY = 0;
   iScreenCanvasMaxY = iCanvasHeight;

   //   iDasherWidth = (iDasherHeight * (iScreenCanvasMaxX - iScreenCanvasMinX)) / (2*(iScreenCanvasMaxX - iScreenCanvasMinX) - iScreenWidth);
   break;
 case (Dasher::Opts::TopToBottom):
   iScreenCanvasMinX = 0;
   iScreenCanvasMaxX = iScreenWidth;
   iScreenCanvasMinY = 0;
   iScreenCanvasMaxY = iScreenHeight * iCanvasWidth / iScreenWidth; // URGH 

   //  iDasherWidth = (iDasherHeight * (iScreenCanvasMaxY - iScreenCanvasMinY)) / (2*(iScreenCanvasMaxY - iScreenCanvasMinY) - iScreenHeight);
   break;
 case (Dasher::Opts::BottomToTop):
   iScreenCanvasMinX = 0;
   iScreenCanvasMaxX = iScreenWidth;
   iScreenCanvasMinY = iScreenHeight - iScreenHeight * iCanvasWidth / iScreenWidth; // Still URGH
   iScreenCanvasMaxY = iScreenHeight;

   //   iDasherWidth = (iDasherHeight * (iScreenCanvasMaxY - iScreenCanvasMinY)) / (2*(iScreenCanvasMaxY - iScreenCanvasMinY) - iScreenHeight);
   break;
 default:
   break;
 }

 // FIXME - everything above this line should be cached, as it only changes very rarely

 // Now we've got the bouding box, use the input positions relative to
 // the box to get the actual Dasher co-ordinates
 
 switch (ScreenOrientation) {
 case (Dasher::Opts::LeftToRight):
   iScreenX = iScreenCanvasMaxX - iDasherX * ( iScreenCanvasMaxX - iScreenCanvasMinX) / iDasherWidth;
   iScreenY = iDasherY  * ( iScreenCanvasMaxY - iScreenCanvasMinY) / iDasherHeight + iScreenCanvasMinY;
   break;
 case (Dasher::Opts::RightToLeft):
   iScreenX = iScreenCanvasMinX + iDasherX * ( iScreenCanvasMaxX - iScreenCanvasMinX) / iDasherWidth;
   iScreenY = iDasherY  * ( iScreenCanvasMaxY - iScreenCanvasMinY) / iDasherHeight + iScreenCanvasMinY;
   break;
 case (Dasher::Opts::TopToBottom):
   iScreenX = iDasherY  * ( iScreenCanvasMaxX - iScreenCanvasMinX) / iDasherHeight + iScreenCanvasMinX;
   iScreenY = iScreenCanvasMaxY - iDasherX * ( iScreenCanvasMaxY - iScreenCanvasMinY) / iDasherWidth;
   break;
 case (Dasher::Opts::BottomToTop):
   iScreenX = iDasherY  * ( iScreenCanvasMaxX - iScreenCanvasMinX) / iDasherHeight + iScreenCanvasMinX;
   iScreenY = iScreenCanvasMinY + iDasherX * ( iScreenCanvasMaxY - iScreenCanvasMinY) / iDasherWidth;
   break;
 default:
   break;
 }
}

/// The minimum Dasher Y co-ordinate which will be visible

myint CDasherViewSquare::DasherVisibleMinY() {
  return m_ymap.unmap( 0 );
}

/// The maximum Dasher Y co-ordinate which will be visible

myint CDasherViewSquare::DasherVisibleMaxY() {
  return m_ymap.unmap( DasherModel().DasherY() );
}

/// The maximum Dasher X co-ordinate which will be visible

myint CDasherViewSquare::DasherVisibleMaxX() {
  return ixmap( 1.0 ) * DasherModel().DasherY();
}

/// Convert raw Dasher co-ordinates to the equivalent 1D mode position

void CDasherViewSquare::Dasher2OneD( myint &iDasherX, myint &iDasherY ) {

    double disty,circlesize,yfullrange,yforwardrange,angle,ellipse_eccentricity,ybackrange,yb,x;	
    
    // The distance between the Y coordinate and the centreline in pixels
    disty=DasherModel().DasherOY()-iDasherY;
        
    circlesize=    DasherModel().DasherY()/2.5;
    yforwardrange= DasherModel().DasherY()/3.2; // Was 1.6
    yfullrange=    yforwardrange*1.6;
    ybackrange=    yfullrange-yforwardrange;
    ellipse_eccentricity=6;
 
    if (disty>yforwardrange) {
      // If the distance between y-coord and centreline is > radius,
      // we should be going backwards, off the top.
      yb=(disty-yforwardrange)/ybackrange;
      
      if (yb>1) {
	x=0;
	iDasherY=double(DasherModel().DasherOY());
      }
      else { 
	angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));

	x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
	iDasherY=-(1+cos(angle))*circlesize/2+DasherModel().DasherOY();
      }
    }
    else if (disty <-(yforwardrange)) {
      // Backwards, off the bottom.
      yb=-(disty+yforwardrange)/ybackrange;
      
      if (yb>1) {
	x=0;
	iDasherY=double(DasherModel().DasherOY());
      }   
      else {
	angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));
	
	x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
	iDasherY=(1+cos(angle))*circlesize/2+DasherModel().DasherOY();
      }   
    }
    
    else {
      angle=((disty*3.14159/2)/yforwardrange);
      x=cos(angle)*circlesize;
      iDasherY=-sin(angle)*circlesize+DasherModel().DasherOY();
    }
    x=DasherModel().DasherOX()-x;

    iDasherX = x;
}

/// Convert raw Dasher co-ordinates to eyetracker position

void CDasherViewSquare::Dasher2Eyetracker( myint &iDasherX, myint &iDasherY ) {

  double disty=DasherModel().DasherOY()-iDasherY;

  myint x( iDasherX );

  myint dasherOX=DasherModel().DasherOX(); 

  //  if( iDasherX < dasherOX ) {

      //cout << "dasherOX: " << dasherOX << endl; 
      myint dasherOY=DasherModel().DasherOY(); 
         
      // X co-ordinate changes. 
      double double_x = (x/dasherOX);  // Fraction of way over to crosshair
      double double_y = -((iDasherY-dasherOY)/(double)(dasherOY) ); // Fraction above the crosshair
             
      // FIXME - I have *no* idea how this is supposed to work - someone else fix it and comment the code please!

      double xmax_y = xmax(double_x, double_y); 
                 
      if(double_x < xmax_y) { 
        double_x = xmax_y; 
      } 

      x = dasherOX*double_x;                 

      // Finished x-coord changes.

      double repulsionparameter=0.5;
      iDasherY = dasherOY - (1.0+ double_y*double_y* repulsionparameter ) * disty ;


      if( iDasherX < x )
      iDasherX = x;
      // }

//   int iDasherOX( DasherModel().DasherOX() );
//   int iDasherOY( DasherModel().DasherOY() );

//   int iDasherMaxY( DasherModel().DasherY() );

//   // Add some gain to the Y value

//   double dYGain( 1.1 );

//   iDasherY = (iDasherY - iDasherOY) * dYGain + iDasherOY;

//   // Truncate the Y range, as behaviour isn't well defined outside of these limits

//   if( iDasherY < 0 )
//     iDasherY = 0;
  
//   if( iDasherY > iDasherMaxY )
//     iDasherY = iDasherMaxY;

//   if( iDasherX > iDasherOX )
//     return; // Don't do anything else if we're to the left of the crosshair

//   int iDasherMinX; // Minimum X value to allow for the circle

//   if( iDasherY < iDasherOY ) {
//     iDasherMinX = (1 - sqrt( 1 - pow(((iDasherOY - iDasherY) / static_cast<double>(iDasherOY)), 2.0 ) )) * iDasherOX;
//   }
//   else {
//     iDasherMinX = (1 - sqrt( 1 - pow(((iDasherY - iDasherOY) / static_cast<double>(iDasherMaxY - iDasherOY)), 2.0 ) )) * iDasherOX;
//   }
  
//   // If X is outside of the circle, bring it in.

//   if( iDasherX < iDasherMinX )
//     iDasherX = iDasherMinX;
}

/// Convert abstract 'input coordinates', which may or may not
/// correspond to actual screen positions, depending on the settings,
/// into dasher co-ordinates. Modes are:
///
/// 0 = Direct (ie mouse)
/// 1 = 1D
/// 2 = Eyetracker
///
/// This should be done once initially, then we work in Dasher
/// co-ordinates for everything else. Input co-ordinates will be
/// assumed to range over the extent of the screen.
///
/// TODO: Abstract out modes into an enum

void CDasherViewSquare::Input2Dasher( screenint iInputX, screenint iInputY, myint &iDasherX, myint &iDasherY, int iType, int iMode ) {

  // FIXME - need to incorporate one-button mode?

  // First convert the supplied co-ordinates to 'linear' Dasher co-ordinates

  switch( iType ) {
  case 0:
    // Raw secreen coordinates
    if( iMode == 2 ) {
      // First apply the autocalibration offset
      iInputY += int(m_yAutoOffset); // FIXME - we need more flexible autocalibration to work with orientations other than left-to-right
    }

    if( iMode == 0 )
      Screen2Dasher( iInputX, iInputY, iDasherX, iDasherY, false, true );
    else if( iMode == 1 )
      Screen2Dasher( iInputX, iInputY, iDasherX, iDasherY, true, false );
    else
      Screen2Dasher( iInputX, iInputY, iDasherX, iDasherY, false, false );
    break;
  

  case 1:
    // Raw dasher coordinates

    iDasherX = iInputX;
    iDasherY = iInputY;
    break;
  default:
    // ERROR
    break;
  }

  // Then apply any non-linearity

  switch( iMode ) {
  case 0: // Direct mode
    // Simply get the dasher co-ordinate under the mouse cursor
    //    Screen2Dasher( iInputX, iInputY, iDasherX, iDasherY );
    

    break;
  case 1: // 1D mode
    // Ignore orientation - iInputY maps directly to the single dimension in this case
    //    iDasherY = iInputY * DasherModel().DasherY() / Screen().GetHeight();
    
    // Apply non-linear mapping
    Dasher2OneD( iDasherX, iDasherY );

    break;
  case 2: // Eyetracker mode

    // Then find the dasher co-ordinate under the offset mouse position
  
    // Finally apply the non-linear transformation
    Dasher2Eyetracker( iDasherX, iDasherY );

    break;
  default:
    // Oops!
    break;
  }
}

/// Draw a polyline specified in Dasher co-ordinates

/* in this case, a "polyline" is a square
 */

void CDasherViewSquare::DasherPolyline( myint *x, myint *y, int n, int iColour ) {

	CDasherScreen::point* ScreenPoints = new CDasherScreen::point[n];

	for( int i(0); i < n; ++i )
		Dasher2Screen( x[i], y[i], ScreenPoints[i].x, ScreenPoints[i].y );

	if( iColour != -1 ) {
		Screen().Polyline(ScreenPoints,n,iColour);
	} else {
		Screen().Polyline(ScreenPoints,n);
	}
	delete [] ScreenPoints;
}

// Draw a filled polygon specified in Dasher co-ordinates

/* XXX not used */

void CDasherViewSquare::DasherPolygon( myint *x, myint *y, int n, int iColour ) {

        CDasherScreen::point* ScreenPoints = new CDasherScreen::point[n];

        for( int i(0); i < n; ++i )
                Dasher2Screen( x[i], y[i], ScreenPoints[i].x, ScreenPoints[i].y );

        Screen().Polygon(ScreenPoints,n,iColour);
        delete [] ScreenPoints;
}

// Draw a box specified in Dasher co-ordinates

/* XXX called more commonly than polyline? */

void CDasherViewSquare::DasherDrawRectangle( myint iLeft, myint iTop, myint iRight, myint iBottom,
					     const int Color, Opts::ColorSchemes ColorScheme ) {
  
  screenint iScreenLeft;
  screenint iScreenTop;
  screenint iScreenRight;
  screenint iScreenBottom;

  Dasher2Screen( iLeft, iTop, iScreenLeft, iScreenTop );
  Dasher2Screen( iRight, iBottom, iScreenRight, iScreenBottom );

  Screen().DrawRectangle(iScreenLeft, iScreenTop, iScreenRight, iScreenBottom, Color, ColorScheme);
}

/// Draw a rectangle centred on a given dasher co-ordinate, but with a size specified in screen co-ordinates (used for drawing the mouse blob)

/* XXX when called? */

void CDasherViewSquare::DasherDrawCentredRectangle( myint iDasherX, myint iDasherY, screenint iSize,
						    const int Color, Opts::ColorSchemes ColorScheme ) {
  
  screenint iScreenX;
  screenint iScreenY;

  Dasher2Screen( iDasherX, iDasherY, iScreenX, iScreenY );

  Screen().DrawRectangle(iScreenX - iSize, iScreenY - iSize, iScreenX + iSize, iScreenY + iSize, Color, ColorScheme);
}

/// Draw text specified in Dasher co-ordinates. The position is
/// specified as two co-ordinates, intended to the be the corners of
/// the leading edge of the containing box.

/* XXX what are the dasher coordinates defined as */

void CDasherViewSquare::DasherDrawText( myint iAnchorX1, myint iAnchorY1, myint iAnchorX2, myint iAnchorY2, const std::string &sDisplayText, int &mostleft ) {

  // Don't draw text which will overlap with text in an ancestor.

  if (iAnchorX1 > mostleft)
    iAnchorX1 = mostleft;

  if (iAnchorX2 > mostleft)
    iAnchorX2 = mostleft;
  
  screenint iScreenAnchorX1;
  screenint iScreenAnchorY1; 
  screenint iScreenAnchorX2;
  screenint iScreenAnchorY2;

  Dasher2Screen( iAnchorX1, iAnchorY1, iScreenAnchorX1, iScreenAnchorY1 );
  Dasher2Screen( iAnchorX2, iAnchorY2, iScreenAnchorX2, iScreenAnchorY2 );

  // Truncate the ends of the anchor line to be on the screen - this
  // prevents us from loosing characters off the top and bottom of the
  // screen

  TruncateToScreen( iScreenAnchorX1, iScreenAnchorY1 );
  TruncateToScreen( iScreenAnchorX2, iScreenAnchorY2 );

  // Actual anchor point is the midpoint of the anchor line

  screenint iScreenAnchorX( (iScreenAnchorX1 + iScreenAnchorX2)/2 );
  screenint iScreenAnchorY( (iScreenAnchorY1 + iScreenAnchorY2)/2 );

  // Compute font size based on position
  int Size = Screen().GetFontSize();
  screenint iLeftTimesFontSize = (iAnchorX1+iAnchorX2)*Screen().GetFontSize()/2;
  if (iLeftTimesFontSize > DasherModel().DasherY()*1/20) 
    Size *= 20;
  else if (iLeftTimesFontSize > DasherModel().DasherY()*1/160) 
    Size *= 14;
  else
    Size *= 11;
	
  screenint TextWidth, TextHeight, OriginX=0, OriginY=0;
  
  Screen().TextSize(sDisplayText, &TextWidth, &TextHeight, Size);

  // Poistion of text box relative to anchor depends on orientation

  screenint newleft2;
  screenint newtop2;
  screenint newright2;
  screenint newbottom2;

  switch (ScreenOrientation) {
  case (Dasher::Opts::LeftToRight):
    newleft2 = iScreenAnchorX;
    newtop2 = iScreenAnchorY - TextHeight/2;
    newright2 = iScreenAnchorX + TextWidth;
    newbottom2 = iScreenAnchorY + TextHeight/2;
    break;
  case (Dasher::Opts::RightToLeft):
    newleft2 = iScreenAnchorX - TextWidth;
    newtop2 = iScreenAnchorY - TextHeight/2;
    newright2 = iScreenAnchorX;
    newbottom2 = iScreenAnchorY + TextHeight/2;
   break;
 case (Dasher::Opts::TopToBottom):
   newleft2 = iScreenAnchorX - TextWidth/2;
   newtop2 = iScreenAnchorY;
   newright2 = iScreenAnchorX + TextWidth/2;
   newbottom2 = iScreenAnchorY + TextHeight;
   break;
 case (Dasher::Opts::BottomToTop):
   newleft2 = iScreenAnchorX - TextWidth/2;
   newtop2 = iScreenAnchorY - TextHeight;
   newright2 = iScreenAnchorX + TextWidth/2;
   newbottom2 = iScreenAnchorY;
   break;
 default:
   break;
 }


  // Update the value of mostleft to take into account the new text

  myint iDasherNewLeft;
  myint iDasherNewTop;
  myint iDasherNewRight;
  myint iDasherNewBottom;

  Screen2Dasher( newleft2, newtop2, iDasherNewLeft, iDasherNewTop, false, true );
  Screen2Dasher( newright2, newbottom2, iDasherNewRight, iDasherNewBottom, false, true );

  mostleft = std::min(iDasherNewRight,iDasherNewLeft);

  // Actually draw the text. We use DelayDrawText as the text should
  // be overlayed once all of the boxes have been drawn.

  m_DelayDraw.DelayDrawText(sDisplayText, newleft2, newtop2, Size);

}

/// Truncate a set of co-ordinates so that they are on the screen

void CDasherViewSquare::TruncateToScreen( screenint &iX, screenint &iY ) {
  if( iX < 0 )
    iX = 0;
  if( iX > Screen().GetWidth() )
    iX = Screen().GetWidth();

  if( iY < 0 )
    iY = 0;
  if( iY > Screen().GetHeight() )
    iY = Screen().GetHeight(); 
}


/////////////////////////////////////////////////////////////////////////////

/* called periodically to move the view
arguments are position of mouse
converts this to coordinates in core representation and then calls 
DasherModel().Tap_on_display
*/

// work out the next viewpoint
// move the rectangles accordingly
void CDasherViewSquare::TapOnDisplay(screenint mousex,screenint mousey, unsigned long Time, Dasher::VECTOR_SYMBOL_PROB* vectorAdded, int* numDeleted) 
{

  // NOTE - we now ignore the values which are actually passed to the display

  // FIXME - Actually turn autocalibration on and off!
  // FIXME - AutoCalibrate should use Dasher co-ordinates, not raw mouse co-ordinates?
  // FIXME - Have I broken this by moving it before the offset is applied?
  // FIXME - put ymap stuff back in 


  // FIXME - optimise this

  int iCoordinateCount( GetCoordinateCount() );

  myint *pCoordinates( new myint[ iCoordinateCount ] );

  int iType( GetCoordinates( iCoordinateCount, pCoordinates ));

  if( iCoordinateCount == 1 ) {
    mousex = 0;
    mousey = pCoordinates[ 0 ];
  }
  else {
    mousex = pCoordinates[ 0 ];
    mousey = pCoordinates[ 1 ];
  }

  delete pCoordinates;

  bool autocalibrate=1;
  if (autocalibrate) {
    AutoCalibrate(&mousex, &mousey);
  }

  myint iDasherX;
  myint iDasherY;

  // Convert the input co-ordinates to dasher co-ordinates

  Input2Dasher( mousex, mousey, iDasherX, iDasherY, iType, DasherModel().GetMode() );

  // Don't go off the canvas - FIXME - is this always needed, or just in direct mode?

  if( DasherModel().GetMode() == 0 ) {
    if( iDasherY > DasherModel().DasherY() )
      iDasherY = DasherModel().DasherY();
    if( iDasherY < 0 )
      iDasherY = 0;
  }

  m_iDasherXCache = iDasherX;
  m_iDasherYCache = iDasherY;


  // Request an update at the calculated co-ordinates

  DasherModel().Tap_on_display(iDasherX,iDasherY, Time, vectorAdded, numDeleted);
  CheckForNewRoot();

  // Cache the Dasher Co-ordinates, so we can use them later for things like drawing the mouse position


}

/////////////////////////////////////////////////////////////////////////////
// move to the specified point

/* used in button mode, perform a smooth animated zoom in a number of
   steps which takes the specified point to the crosshair
*/

void CDasherViewSquare::GoTo(screenint mousex,screenint mousey) 
{
	// convert mouse (screen) coords into dasher coords
	
	UnMapScreen(&mousex, &mousey);
	myint idasherx,idashery;
	screen2dasher(mousex,mousey,&idasherx,&idashery);
	DasherModel().GoTo(idasherx,idashery);
	CheckForNewRoot();
}

/////////////////////////////////////////////////////////////////////////////

/* draw hints which indicate what will happen when a user clicks in
   button mode
 */

void CDasherViewSquare::DrawGoTo(screenint mousex, screenint mousey)
{
  // Draw a box surrounding the area of the screen that will be zoomed into
  UnMapScreen(&mousex, &mousey);
  myint idasherx,idashery;
  screen2dasher(mousex,mousey,&idasherx,&idashery);
  // So, we have a set of coordinates. We need a bunch of points back.
  myint height=DasherModel().PlotGoTo(idasherx, idashery);
  myint top, bottom, left, right;

  // Convert back to screen coordinates?
  top=mousey-height/2;
  bottom=mousey+height/2;
  left=mousex+height/2;
  right=mousex-height/2;
  top=dashery2screen(top);
  bottom=dashery2screen(bottom);
  left=dasherx2screen(left);
  right=dasherx2screen(right);
  
  // Draw the lines
  Screen().DrawRectangle(left, top+5, right, top-5, 1, Opts::ColorSchemes(Objects));
  Screen().DrawRectangle(left+5, top+5, left, bottom-5, 1, Opts::ColorSchemes(Objects));
  Screen().DrawRectangle(left, bottom+5, right, bottom-5, 1, Opts::ColorSchemes(Objects));
}
  
/////////////////////////////////////////////////////////////////////////////

/*
put a black blob where current mouse position is, used for people with
bad eyesight
*/

void CDasherViewSquare::DrawMouse(screenint mousex, screenint mousey)
{  

  int iCoordinateCount( GetCoordinateCount() );

  myint *pCoordinates( new myint[ iCoordinateCount ] );

  int iType( GetCoordinates( iCoordinateCount, pCoordinates ) );

  if( iCoordinateCount == 1 ) {
    mousex = 0;
    mousey = pCoordinates[ 0 ];
  }
  else {
    mousex = pCoordinates[ 0 ];
    mousey = pCoordinates[ 1 ];
  }

  delete pCoordinates;  
  myint iDasherX;
  myint iDasherY;

  Input2Dasher( mousex, mousey, iDasherX, iDasherY, iType, DasherModel().GetMode() );

  if (ColourMode==true) {
    DasherDrawCentredRectangle( iDasherX, iDasherY, 5, 2, Opts::ColorSchemes(Objects));
  } else {
    DasherDrawCentredRectangle( iDasherX, iDasherY, 5, 1, Opts::ColorSchemes(Objects));
  }

    //     if (DasherModel().Dimensions()==true || DasherModel().Eyetracker()==true) {
  
// 	  int Swapper;
	
// 	  myint dasherx,dashery;
// 	  screen2dasher(mousex,mousey,&dasherx,&dashery);
// 	  mousex=dasherx2screen(dasherx);
// 	  mousey=dashery2screen(dashery);
// 	  switch (ScreenOrientation) {
// 	  case (LeftToRight):
// 	    break;
// 	  case (RightToLeft):
// 	    mousex = Screen().GetWidth() - mousex;
// 	    break;
// 	  case (TopToBottom):
// 	    Swapper = ( mousex * Screen().GetHeight()) / Screen().GetWidth();
// 	    mousex = (mousey  * Screen().GetWidth()) / Screen().GetHeight();
// 	    mousey = Swapper;
// 	    break;
// 	  case (BottomToTop):
// 	    // Note rotation by 90 degrees not reversible like others
// 	    Swapper = Screen().GetHeight() - ( mousex * Screen().GetHeight()) / Screen().GetWidth();
// 	    mousex = (mousey  * Screen().GetWidth()) / Screen().GetHeight();
// 	    mousey = Swapper;
// 	    break;
// 	  default:
// 	    break;
// 	  }
// 	}

// 	if (ColourMode==true) {
// 	  Screen().DrawRectangle(mousex-5, mousey-5, mousex+5, mousey+5, 2, Opts::ColorSchemes(Objects));
// 	} else {
// 	  Screen().DrawRectangle(mousex-5, mousey-5, mousex+5, mousey+5, 1, Opts::ColorSchemes(Objects));
// 	}
}

/////////////////////////////////////////////////////////////////////////////

/// Draw a line from the crosshair to the mouse position

void CDasherViewSquare::DrawMouseLine(screenint mousex, screenint mousey)
{

  int iCoordinateCount( GetCoordinateCount() );

  myint *pCoordinates( new myint[ iCoordinateCount ] );

  int iType( GetCoordinates( iCoordinateCount, pCoordinates ) );

  if( iCoordinateCount == 1 ) {
    mousex = 0;
    mousey = pCoordinates[ 0 ];
  }
  else {
    mousex = pCoordinates[ 0 ];
    mousey = pCoordinates[ 1 ];
  }

  delete pCoordinates;  

  myint x[2];
  myint y[2];

  // Start of line is the crosshair location

  x[0] = DasherModel().DasherOX();
  y[0] = DasherModel().DasherOY();

  // End of line is the mouse cursor location - note that we should
  // probably be using a chached value rather than computing this
  // separately to TapOn

  Input2Dasher( mousex, mousey, x[1], y[1], iType, DasherModel().GetMode() );
  
  // Actually plot the line

  if (ColourMode==true) {
    DasherPolyline(x,y,2,1);
  } else {
    DasherPolyline(x,y,2,-1);
  }
}

/////////////////////////////////////////////////////////////////////////////

/* deprecated
shouldn't be actually called
*/

void CDasherViewSquare::DrawKeyboard()
{
  CDasherScreen::point line[2];
  line[0].x = 200;
  line[0].y = 0;
  line[1].x = 200;
  line[1].y = CanvasY/2;
  
  if (ColourMode==true) {
    Screen().Polyline(line,2,6);
  } else {
    Screen().Polyline(line,2);
  }

  line[0].x = 200;
  line[0].y = CanvasY/2;
  line[1].x = 0;
  line[1].y = CanvasY/2;

  if (ColourMode==true) {
    Screen().Polyline(line,2,6);
  } else {
    Screen().Polyline(line,2);
  }

  line[0].x = 200;
  line[0].y = CanvasY/2;
  line[1].x = 200;
  line[1].y = CanvasY;

  if (ColourMode==true) {
    Screen().Polyline(line,2,6);
  } else {
    Screen().Polyline(line,2);
  }
}

/* XXX from old version of button mode */

void CDasherViewSquare::ResetSum() 
{ 
	m_ySum=0; 
}

void CDasherViewSquare::ResetSumCounter() 
{ 
	m_ySumCounter=0; 
}

void CDasherViewSquare::ResetYAutoOffset()
{ 
	m_yAutoOffset=0; 
}


/////////////////////////////////////////////////////////////////////////////

/* XXX chris doesn't know */

void CDasherViewSquare::ChangeScreen(CDasherScreen* NewScreen)
{
	CDasherView::ChangeScreen(NewScreen);
	screenint Width = Screen().GetWidth();
	screenint Height = Screen().GetHeight();
	CanvasX=9*Width/10;
	CanvasBorder=Width-CanvasX;
	CanvasY=Height;
}

/////////////////////////////////////////////////////////////////////////////

/* for automatic eyetracker calibration */

int CDasherViewSquare::GetAutoOffset() const
{
	 return m_yAutoOffset;
}


/////////////////////////////////////////////////////////////////////////////

/// Convert screen co-ordinates to dasher co-ordinates, possibly
/// involving a non-linear transformation for 1D mode, eyetracking
/// mode etc.
///
/// FIXME - lots of floating point arithmetic here

void CDasherViewSquare::screen2dasher(screenint imousex, screenint imousey, myint* idasherx, myint* idashery) const
{
    bool eyetracker=DasherModel().Eyetracker();
    // bool DasherRunning = DasherModel().Paused();

    // Add the eyetracker autocalibration offset if necessary

    if( eyetracker )
      imousey += int(m_yAutoOffset);

    // Maybe this mousey tweak should take place earlier, elsewhere, and 
    // have a permanent effect on mousey rather than just local.

    //SUMMARY OF  Y autocallibrate additions: 
    // If autocallibrate  {
    //    tweak mousey right now before anyone looks at it.
    //    If dasher running {
    //        Adjust our tweak estimate
    //    }
    // }
    // end summary

    // If autocallibrate  {
    //    tweak mousey right now before anyone looks at it.
    //           NOTE: yAutoOffset should be set to zero ONCE when Dasher 
    //                 first started, then left alone. In principle, if 
    //                 someone moves their Dasher window from one locn to another
    //  then it might be reasonable to re-zero the offset. But don't.

	// Convert the Y mouse coordinate to one that's based on the canvas size
	double dashery=double(imousey*DasherModel().DasherY()/CanvasY);
	bool useonebutton=0;
	if (useonebutton) {
		int onebutton = CDasherView::GetOneButton();
	    dashery=onebutton;
	}
    
	// Convert the X mouse coordinate to one that's based on the canvas size 
	// - we want this the opposite way round to the mouse coordinate system, 
	// hence the fudging. ixmap gives us the X nonlinearity.	
	double x=ixmap(1.0*(CanvasX-imousex)/CanvasX)*DasherModel().DasherY();

    // Disable one-button mode for now.
       // if (eyetracker==true) { dashery=onebutton; }
	
    // If we're in standard mode, fudge things for the vertical acceleration
	if (DasherModel().Dimensions()==false && KeyControl==false && eyetracker==false) {
		dashery = m_ymap.unmap(dashery);
		if (dashery>DasherModel().DasherY()) {
			dashery=DasherModel().DasherY();
		}
		if (dashery<0) {
			dashery=0;
		}
	}

    // The X and Y origins.
    myint dasherOX=DasherModel().DasherOX();
    //cout << "dasherOX: " << dasherOX << endl;
    myint dasherOY=DasherModel().DasherOY();
    // For Y co-ordinate changes. 
    // disty is the distance between y and centreline. 
    double disty=double(dasherOY)-dashery;                  
    //cout << "disty: " << disty << endl;

	// If we're in one-dimensional mode, make new x,y
	if (DasherModel().Dimensions()==true) {
		//if (eyetracker==true && !(x<DasherModel().DasherOX() && pow(pow(DasherModel().DasherY()/2-dashery,2)+pow(x-DasherModel().DasherOX(),2),0.5)>DasherModel().DasherY()/2.5)) {
		//	*mousex=int(x);
		//	*mousey=int(dashery);
		//	return;
		//}
      
		double disty,circlesize,yfullrange,yforwardrange,angle,ellipse_eccentricity,ybackrange,yb;	

		// The distance between the Y coordinate and the centreline in pixels
		disty=dasherOY-dashery;
        
        //		double rel_dashery=dashery+1726;
		//      double rel_dasherOY=dasherOY+1726;
		//cout << "x: " << x << endl;
		//cout << "dashery: " << rel_dashery << endl << endl;

		// The radius of the circle transcribed by the one-dimensional mapping
		circlesize=    DasherModel().DasherY()/2.5;
		yforwardrange= DasherModel().DasherY()/1.6;
		yfullrange=    yforwardrange*1.6;
		ybackrange=    yfullrange-yforwardrange;
		ellipse_eccentricity=6;
 
		if (disty>yforwardrange) {
			// If the distance between y-coord and centreline is > radius,
			// we should be going backwards, off the top.
			yb=(disty-yforwardrange)/ybackrange;
            
			if (yb>1) {
				x=0;
				dashery=double(dasherOY);
			}
			else { 
				angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));
				x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
				dashery=-(1+cos(angle))*circlesize/2+dasherOY;
			}
		}
		else if (disty <-(yforwardrange)) {
			// Backwards, off the bottom.
			yb=-(disty+yforwardrange)/ybackrange;

			if (yb>1) {
			x=0;
				dashery=double(dasherOY);
            }   
			else {
				angle=(yb*3.14159)*(yb+(1-yb)*(ybackrange/yforwardrange/ellipse_eccentricity));

				x=(-sin(angle)*circlesize/2)*ellipse_eccentricity;
				dashery=(1+cos(angle))*circlesize/2+dasherOY;
			}   
		}

        else {
          angle=((disty*3.14159/2)/yforwardrange);
          x=cos(angle)*circlesize;
          dashery=-sin(angle)*circlesize+dasherOY;
        }
		x=dasherOX-x;
    }
    else if (eyetracker==true) {

      myint dasherOX=DasherModel().DasherOX(); 
      //cout << "dasherOX: " << dasherOX << endl; 
      myint dasherOY=DasherModel().DasherOY(); 
         
      // X co-ordinate changes. 
      double double_x = (x/dasherOX); 
      double double_y = -((dashery-dasherOY)/(double)(dasherOY) ); 
             
      double xmax_y = xmax(double_x, double_y); 
                 
      if(double_x < xmax_y) { 
        double_x = xmax_y; 
      } 

      x = dasherOX*double_x;                 

      // Finished x-coord changes.

      double repulsionparameter=0.5;
      dashery = dasherOY - (1.0+ double_y*double_y* repulsionparameter ) * disty ;
    } 
    /* 
    // Finish the yautocallibrate
    //    If dasher running, adjust our tweak estimate
      if(!DasherRunning==true) {
        CDasherView::yFilterTimescale = 60;
        CDasherView::ySum += disty; 
        CDasherView::ySumCounter++; 
       
        CDasherView::ySigBiasPercentage=50;
        CDasherView::ySigBiasPixels = CDasherView::ySigBiasPercentage * DasherModel().DasherY() / 100;

        // FIXME: screen2dasher appears to be being called thrice per frame.
        // I don't know why.  
        CDasherView::ySigBiasPixels*=3;

        cout <<"ySum: " << CDasherView::ySum << " | ySigBiasPixels: " << CDasherView::ySigBiasPixels << " | disty: " << disty << " | yAutoOffset: " << CDasherView::yAutoOffset << endl;
  
        if (CDasherView::ySumCounter > CDasherView::yFilterTimescale) {
          CDasherView::ySumCounter = 0;

          // 'Conditions A', as specified by DJCM.  Only make the auto-offset
          // change if we're past the significance boundary.

          if (CDasherView::ySum > CDasherView::ySigBiasPixels || CDasherView::ySum < -CDasherView::ySigBiasPixels) {
             if (CDasherView::ySum > CDasherView::yFilterTimescale)
              CDasherView::yAutoOffset--; 
              else if (CDasherView::ySum < -CDasherView::yFilterTimescale)
              CDasherView::yAutoOffset++;
            CDasherView::ySum = 0;
          }
        }
    }
    */
    *idasherx=myint(x);

	*idashery=myint(dashery);
}

/////////////////////////////////////////////////////////////////////////////

/* transformations for eyetracker autocalibration 
(it's here because it needs to look at where pointer is relative to crosshair)
*/

void CDasherViewSquare::AutoCalibrate(screenint *mousex, screenint *mousey)
{
    double dashery=double(*mousey)*double(DasherModel().DasherY())/double(CanvasY);
    myint dasherOY=DasherModel().DasherOY();
    double disty=double(dasherOY)-dashery;
    bool DasherRunning = DasherModel().Paused();


    if(!DasherRunning==true) {
        m_yFilterTimescale = 20;
        m_ySum += (int)disty;
        m_ySumCounter++;

        m_ySigBiasPercentage=50;
        m_ySigBiasPixels = m_ySigBiasPercentage * DasherModel().DasherY() / 100;

        //cout << "yAutoOffset: " << CDasherView::yAutoOffset << endl;

        if (m_ySumCounter > m_yFilterTimescale) {
          m_ySumCounter = 0;

          // 'Conditions A', as specified by DJCM.  Only make the auto-offset
          // change if we're past the significance boundary.

          if (m_ySum > m_ySigBiasPixels || m_ySum < -m_ySigBiasPixels) {
	    if (m_ySum > m_yFilterTimescale) {
                 m_yAutoOffset--;
	    }
	    else if (m_ySum < -m_yFilterTimescale)
	      m_yAutoOffset++;
            
             m_ySum = 0;
          }
        }
        
        //*mousey=int(dashery);
    }
}



/////////////////////////////////////////////////////////////////////////////

