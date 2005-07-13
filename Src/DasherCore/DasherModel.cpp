// DasherModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "DasherModel.h"

//#include <iostream>

#include "../Common/Random.h"
#include "LanguageModelling/PPMLanguageModel.h"
#include "LanguageModelling/WordLanguageModel.h"
#include "LanguageModelling/DictLanguageModel.h"
#include "LanguageModelling/MixtureLanguageModel.h"

using namespace Dasher;
using namespace std;


//////////////////////////////////////////////////////////////////////
// CDasherModel
//////////////////////////////////////////////////////////////////////

CDasherModel::CDasherModel(const CAlphabet* pAlphabet, CDashEditbox* pEditbox, LanguageModelID idLM, CLanguageModelParams *_params,
						   bool Dimensions, bool Eyetracker, bool Paused)
  : m_pcAlphabet(pAlphabet), m_pEditbox(pEditbox) ,
	 m_Dimensions(Dimensions),  m_Eyetracker(Eyetracker),  m_Paused(Paused), 
	 m_Root(0), m_iNormalization(1<<16),	m_uniform(50) , m_pLanguageModel(NULL),
     m_bControlMode(false), m_bAdaptive(true), total_nats(0)
{

  // Convert the full alphabet to a symbolic representation for use in the language model
  
  CSymbolAlphabet alphabet( pAlphabet->GetNumberTextSymbols() );
  alphabet.SetSpaceSymbol( pAlphabet->GetSpaceSymbol() ); // FIXME - is this right, or do we have to do some kind of translation?
  alphabet.SetAlphabetPointer( pAlphabet ); // Horrible hack, but ignore for now.

  // Create an appropriate language model;

  // FIXME - return to using enum here

  switch( idLM ) {
  case idPPM:
    m_pLanguageModel = new CPPMLanguageModel(alphabet, _params);
    break;
  case idWord:
    m_pLanguageModel = new CWordLanguageModel(alphabet, _params);
    break; 
  case idMixture:
    m_pLanguageModel = new CMixtureLanguageModel(alphabet, _params);
    break;
  case idJapanese:
  	std::cout << "Japanese Language Model" << endl;
    m_pLanguageModel = new CPPMLanguageModel(alphabet, _params);
    break;
  default:
    std::cout << "Oops - hit default case" << std::endl;
    break;
  }

  //  std::cout << m_pLanguageModel << " " << idLM << std::endl;
	
  LearnContext = m_pLanguageModel->CreateEmptyContext();

  //  std::cout << "Learn context is " << LearnContext << std::endl;
  
  // various settings
  int iShift = 12;
  m_DasherY = 1<<iShift;
  m_DasherOY = m_DasherY/2;
  m_DasherOX = m_DasherY/2;
  m_dAddProb = 0.003;
  
  m_Active = CRange(0,m_DasherY);
    
  m_Rootmin_min = int64_min/m_iNormalization/2;
  m_Rootmax_max = int64_max/m_iNormalization/2;

}

//////////////////////////////////////////////////////////////////////

CDasherModel::~CDasherModel()
{
	
	if (oldroots.size()>0) 
	{ 
		delete oldroots[0];
		oldroots.clear();
		// At this point we have also deleted the root - so better NULL pointer
		m_Root=NULL;
	}
	
	delete m_Root;

	m_pLanguageModel->ReleaseContext(LearnContext);
	delete m_pLanguageModel;

}

//////////////////////////////////////////////////////////////////////

void CDasherModel::SetControlMode(bool b)
{
	m_bControlMode = b;
}

//////////////////////////////////////////////////////////////////////

void CDasherModel::Make_root(int whichchild)
// find a new root node 
{

	symbol t=m_Root->Symbol();
	if (t < m_pcAlphabet->GetNumberTextSymbols() ) 
	{  
	  // Only learn if we have adaptive behaviour enabled

	  if( m_bAdaptive )
	    // SYM0
	    m_pLanguageModel->LearnSymbol(LearnContext, t);
	}

	m_Root->DeleteNephews(whichchild);

	oldroots.push_back(m_Root);

	m_Root = m_Root->Children()[whichchild];

	while (oldroots.size()>10) 
	{ 
		oldroots[0]->OrphanChild( oldroots[1] );
		delete oldroots[0];
		oldroots.pop_front();
	}

	myint range=m_Rootmax-m_Rootmin;
	m_Rootmax = m_Rootmin + (range*m_Root->Hbnd())/Normalization();
	m_Rootmin = m_Rootmin+ (range*m_Root->Lbnd())/Normalization();
}

void CDasherModel::Reparent_root(int lower, int upper)
{

  /* Change the root node to the parent of the existing node
     We need to recalculate the coordinates for the "new" root as the 
     user may have moved around within the current root */

  if (oldroots.size()==0) // There is no node to reparent to
    return;

  /* Determine how zoomed in we are */

  myint iRootWidth = m_Rootmax-m_Rootmin;
  myint iWidth = upper-lower;
//  double scalefactor=(m_Rootmax-m_Rootmin)/static_cast<double>(upper-lower);

  m_Root=oldroots.back();
  oldroots.pop_back();
  

  m_Rootmax = m_Rootmax+ ( myint((Normalization()-upper))*iRootWidth / iWidth );
 
  
  m_Rootmin = m_Rootmin - ( myint(lower) * iRootWidth/ iWidth );

}

/////////////////////////////////////////////////////////////////////////////

CDasherNode * CDasherModel::Get_node_under_crosshair()
{
	return m_Root->Get_node_under(Normalization(),m_Rootmin,m_Rootmax,m_DasherOX,m_DasherOY);
}

/////////////////////////////////////////////////////////////////////////////


CDasherNode * CDasherModel::Get_node_under_mouse(myint Mousex,myint Mousey)
{
	return m_Root->Get_node_under(Normalization(),m_Rootmin,m_Rootmax,Mousex,Mousey);
}

/////////////////////////////////////////////////////////////////////////////


void CDasherModel::Get_string_under_mouse(const myint Mousex,const myint Mousey, vector<symbol> &str)
{
	m_Root->Get_string_under(Normalization(),m_Rootmin,m_Rootmax,Mousex,Mousey,str);
	return;
}


/////////////////////////////////////////////////////////////////////////////

void CDasherModel::Start()
{
	m_Rootmin=0;
	m_Rootmax=m_DasherY;


	if (oldroots.size()>0) 
	{ 
		delete oldroots[0];
		oldroots.clear();
		// At this point we have also deleted the root - so better NULL pointer
		m_Root=NULL;
	}
	delete m_Root;
	
	m_Root=new CDasherNode(*this,0,0,0,Opts::Nodes1,0,Normalization(),m_pLanguageModel, false, 7);
	CLanguageModel::Context therootcontext=m_pLanguageModel->CreateEmptyContext();

	if (m_pEditbox) {
		string ContextString;
		m_pEditbox->get_new_context(ContextString,5);
		if (ContextString.size()==0) {
		  // If there is no root context, pretend that we've just
		  // finished a sentence
		  ContextString=". " + ContextString;
		}
		EnterText(therootcontext, ContextString);
	
		m_pLanguageModel->ReleaseContext(LearnContext);
		LearnContext = m_pLanguageModel->CloneContext(therootcontext);
	}

	m_Root->SetContext(therootcontext);    // node takes control of the context
	Recursive_Push_Node(m_Root,0);
	
//	m_pLanguageModel->ReleaseNodeContext(therootcontext);
//	ppmmodel->dump();
//	dump();
	
}

/////////////////////////////////////////////////////////////////////////////

///
/// CDasherModel::Get_new_root_coords( myint Mousex,myint Mousey )
/// 
/// Calculate the new co-ordinates for the root node after a single
/// update step. For further information, see Doc/geometry.tex.
/// 
/// \param Mousex x mouse co-ordinate measured right to left.
/// \param Mousey y mouse co-ordinate measured top to bottom.
/// \return Log of the scale factor, equivalent to the number of nats entered
///

double CDasherModel::Get_new_root_coords(myint Mousex,myint Mousey)
{
  // Comments refer to the code immedialtely before them


  if (Mousex<=0) {
    Mousex=1;
  }

  // Avoid Mousex=0, as this corresponds to infinite zoom

  int iTargetMin( Mousey - (m_DasherY*Mousex)/(2*m_DasherOX) );
  int iTargetMax( Mousey + (m_DasherY*Mousex)/(2*m_DasherOX) );

  // Calculate what the extremes of the viewport will be when the
  // point under the cursor is at the cross-hair. This is where 
  // we want to be in iSteps updates
  
  int iSteps=m_fr.Steps();
  
  DASHER_ASSERT(iSteps>0);
  
  // iSteps is the number of update steps we need to get the point
  // under the cursor over to the cross hair. Calculated in order to
  // keep a constant bit-rate.

  int iNewTargetMin;
  int iNewTargetMax;

  iNewTargetMin = ( iTargetMin * m_DasherY / ( m_DasherY + ( iSteps - 1 ) * ( iTargetMax - iTargetMin ) ));
  
  iNewTargetMax = ( (iTargetMax * iSteps - iTargetMin * ( iSteps - 1 )) * m_DasherY ) / ( m_DasherY + ( iSteps - 1 ) * ( iTargetMax - iTargetMin ));

//   int iZoomCentre;

//   iZoomCentre = m_DasherY * iTargetMin / (m_DasherY + iTargetMin - iTargetMax );
//   double dScale;

//   dScale = ( m_DasherY / static_cast<double>(iTargetMax - iTargetMin) - 1 ) / static_cast<double>( iSteps ) + 1;


//   std::cout << iZoomCentre << ", " << dScale << ": " << iTargetMin << " - " << iTargetMax << " => " << iNewTargetMin << " - " << iNewTargetMax << std::endl;

  iTargetMin = iNewTargetMin;
  iTargetMax = iNewTargetMax; 

  // Calculate the new values of iTargetMin and iTargetMax required to
  // perform a single update step. Note that the slightly awkward
  // expressions are in order to reproduce the behaviour of the old
  // algorithm

  myint iMinSize( m_fr.MinSize( m_DasherY ) );

  // Calculate the minimum size of the viewport corresponding to the
  // maximum zoom.

  if( (iTargetMax - iTargetMin ) < iMinSize ) {

    iNewTargetMin = iTargetMin * ( m_DasherY - iMinSize ) / ( m_DasherY - ( iTargetMax - iTargetMin ) );
    iNewTargetMax = iNewTargetMin + iMinSize;

    iTargetMin = iNewTargetMin;
    iTargetMax = iNewTargetMax; 

  }

  //  std::cout << Mousey << " " << iTargetMin << " " << iTargetMax << std::endl;

  // Check we're not going faster than the speed slider setting
  // allows, and adjust if necessary. Note that if we re-size the
  // target we need to be careful about where we centre the new range
  // (hence the slightly complicated expression above)

  DoZoom( iTargetMin, iTargetMax );

  // Actually do the zooming
  
  return -1.0 * log( (iTargetMax - iTargetMin)/static_cast<double>(m_DasherY) );
     
  // Return value is the zoom factor so we can keep track of bitrate.
}

/// Zoom the display so that [iTargetMin,iTargetMax] (in current
/// Dasher co-ordinates) are the new extremes of the viewport.

void CDasherModel::DoZoom( myint iTargetMin, myint iTargetMax ) {

  myint newRootmin( ( ( m_Rootmin - iTargetMin ) * m_DasherY ) / ( iTargetMax - iTargetMin ) );
  myint newRootmax( ( ( m_Rootmax - iTargetMax ) * m_DasherY ) / ( iTargetMax - iTargetMin ) + m_DasherY );

  // Update the max and min of the root node to make iTargetMin and iTargetMax the edges of the viewport.

  if( newRootmin > m_DasherY/2 )
    newRootmin = m_DasherY/2;
  
  if( newRootmax < m_DasherY/2 )
    newRootmax = m_DasherY/2;

  // Check that we haven't drifted too far. The rule is that we're not
  // allowed to let the root max and min cross the midpoint of the
  // screen.

  if (newRootmax<m_Rootmax_max && newRootmin > m_Rootmin_min && (newRootmax - newRootmin) > m_DasherY/4 )    
    {
      // Only update if we're not making things big enough to risk
      // overflow. In theory we should have reparented the root well
      // before getting this far.
      //
      // Also don't allow the update if it will result in making the
      // root too small. Again, we should have re-generated a deeper
      // root in most cases, but the original root is an exception.

      m_Rootmax=newRootmax;
      m_Rootmin=newRootmin;
    } 
  else
    {
      // TODO - force a new root to be chosen, so that we get better
      // behaviour than just having Dasher stop at this point.

    }
}

// double CDasherModel::Get_new_root_coords(myint Mousex,myint Mousey)
// {
//   // FIXME - get rid of floating point here.

//   // I *think* all co-ordinates are in the dasher co-ordinate system

//   // Comments refer to the code immedialtely before them

//   std::cout << Mousex << " " << Mousey << std::endl;

//   if (Mousex<=0) {
//     Mousex=1;
//   }
  
//   // Avoid the very right-hand edge, as this will cause infinities

//   if( Mousex == m_DasherOX ) {
//     Mousex = m_DasherOX - 1;
//   }
    
//   // Also avoid the crosshair, as this causes singularities

//   double dRx=m_DasherOX/static_cast<double>(Mousex);
  
//   // m_DasherOX - x co-ordinate of cross-hair, so this is the scale
//   // factor needed to put the point under the mouse under the
//   // cross hair.
  
//   double dRxnew;
  
//   int iSteps=m_fr.Steps();
  
//   DASHER_ASSERT(iSteps>0);
  
//   // iSteps is the number of update steps we need to get the point
//   // under the cursor over to the cross hair. Calculated in order to
//   // keep a constant bit-rate.

//   dRxnew=1+(dRx-1)/iSteps;
    
//   // Calculate the required single step zoom to achieve dRx in
//   // iSteps steps. Note that this should be:
//   //
//   // dRxnew=pow(dRx,1.0/iSteps)
//   //
//   // But we make an approximation in order to avoid the need for
//   // floating point arithmetic.
  
//   const double dRxmax=m_fr.Rxmax();
  
//   if (dRxnew>dRxmax)
//     dRxnew=dRxmax;
  
//   // Check we're not going faster than the speed slider setting
//   // allows.

//   myint zoom_centre( m_DasherY/2 - ((m_DasherY/2 - Mousey) * (m_DasherOX)/(m_DasherOX - Mousex)));

//   // Calculte the point about which we're zooming by projecting from
//   // the crosshair through the mouse pointer to the right hand edge.

//   myint newRootmin=zoom_centre - dRxnew * ( zoom_centre - m_Rootmin );
//   myint newRootmax=zoom_centre + dRxnew * ( m_Rootmax - zoom_centre );

//   // Scale the root about those edges

//   if (newRootmin>=m_DasherY/2)  
//     newRootmin= m_DasherY/2-1;

//   if (newRootmax<=m_DasherY/2)  
//     newRootmax= m_DasherY/2+1;

//   // Check that we haven't drifted too far. The rule is that we're not
//   // allowed to let the root max and min cross the midpoint of the
//   // screen.

//   if (newRootmax<m_Rootmax_max && newRootmin > m_Rootmin_min && (newRootmax - newRootmin) > m_DasherY/4 )    
//     {
//       // Only update if we're not making things big enough to risk
//       // overflow. In theory we should have reparented the root well
//       // before getting this far.
//       //
//       // Also don't allow the update if it will result in making the
//       // root too small. Again, we should have re-generated a deeper
//       // root in most cases, but the original root is an exception.

//       m_Rootmax=newRootmax;
//       m_Rootmin=newRootmin;
//     } 
//   else
//     {
//       // TODO - force a new root to be chosen, so that we get better
//       // behaviour than just having Dasher stop at this point.
//     }
  
//    return log( dRxnew );
   
  
//    // Return value is the zoom factor so we can keep track of bitrate.
// }


// double CDasherModel::Get_new_root_coords(myint Mousex,myint Mousey)
// {
//   // FIXME - get rid of floating point here.

//   // I *think* all co-ordinates are in the dasher co-ordinate system

//   // Comments refer to the code immedialtely before them

//   double dRx=1.0*m_DasherOX/Mousex;
  
//   // m_DasherOX - x co-ordinate of cross-hair, so this is the scale
//   // factor needed to put the dasher point under the mouse under the
//   // cross hair.
  
//   double dRxnew;
  
//   int iSteps=m_fr.Steps();
  
//   DASHER_ASSERT(iSteps>0);
  
//   // iSteps is the number of update steps we need to get the point
//   // under the cursor over to the cross hair. Calculated in order to
//   // keep a constant bit-rate.

//   if (Mousex<m_DasherOX) {
    
//     // We're in the 'move forward' side of the canvas.

//     if (Mousex<=0)
//       Mousex=1;
    
//     // Avoid the very right-hand edge, as this will cause infinities -
//     // note that this seems utterly pointless, as we've already
//     // calculated dRx by this point and don't use Mousex again in this
//     // function, so maybe we should move this to the start?
    

//     dRxnew=1+(dRx-1)/iSteps;
    
//     // Calculate the required single step zoom to achieve dRx in
//     // iSteps steps. Note that this should be:
//     //
//     // dRxnew=pow(dRx,1.0/iSteps)
//     //
//     // But we make an approximation in order to avoid the need for
//     // floating point arithmetic.

//     const double dRxmax=m_fr.Rxmax();

//     if (dRxnew>dRxmax)
//       dRxnew=dRxmax;

//     // Check we're not going faster than the speed slider setting
//     // allows.

//   } 
//   else 
//     {
//       // We're in the 'move backwards' section of the canvas

//       dRxnew=1+(dRx-1)/iSteps;

//       // See above  - (not entirely sure why we need to treat this as a special case...)

//       if (m_Rootmax<m_DasherY && m_Rootmin> myint(0) ) 
// 	return(1.0);

//       // m_DasherY is the bottom of the visible canvas, so make sure
//       // that the root entirely fits on the screen (this seems
//       // contrary to what David just said...)

//     } 

//   // Now we've figured out what to scale, do the scaling. Remember,
//   // dRxnew is the scale factor.

//   myint above=(Mousey-m_Rootmin);//*(1-rxnew)/(1-rx);
//   myint below=(m_Rootmax-Mousey);//*(1-rxnew)/(1-rx);
  
//   // Distances above and below the mouse cursor to the edges of the
//   // root node.

//   myint miDistance=m_DasherY/2-Mousey;
  
//   // miDistance - y distance of mouse cursor above the mid point of the canvas

//   if (Mousex!=m_DasherOX)
//     miDistance=myint(miDistance*(dRxnew-1)/(dRx-1));
//   else
//     miDistance = miDistance/iSteps;

//   // Work out what the new y distance of mouse above midpoint should
//   // be after the update. Who knows why we have two cases here. Also,
//   // note that for the approximation used above, (dRxnew-1)/(dRx-1) is
//   // eual to 1/iSteps, so in this case the two branches do exactly the
//   // same thing. This wouldn't be the case if we used teh exact update
//   // rule.

//   myint miNewrootzoom=Mousey+miDistance;

//   // New centre point of the root note

//   myint newRootmax=miNewrootzoom+myint(below*dRxnew);
//   myint newRootmin=miNewrootzoom-myint(above*dRxnew);

//   // Calculate the new bounaries by scaling above and below

//   if (newRootmin>=m_DasherY/2)  
//     newRootmin= m_DasherY/2-1;
//   if (newRootmax<=m_DasherY/2)  
//     newRootmax= m_DasherY/2+1;

//   // Check that we haven't drifted too far. The rule is that we're not
//   // allowed to let the root max and min cross the midpoint of the
//   // screen.

//   if (newRootmax<m_Rootmax_max && newRootmin > m_Rootmin_min)    
//     {
//       // Only update if we're not making things big enough to risk
//       // overflow. In theory we should have reparented the root well
//       // before getting this far.

//       m_Rootmax=newRootmax;
//       m_Rootmin=newRootmin;
//     } 
//   else
//     {
//       // TODO - force a new root to be chosen, so that we get better
//       // behaviour than just having Dasher stop at this point.
//     }
  
//    return log( dRxnew );

//    // Return value is the zoom factor so we can keep track of bitrate.
// }


void CDasherModel::Get_new_goto_coords(double zoomfactor, myint MouseY) 
                                       // this was mousex.

{
  // First, we need to work out how far we need to zoom in
  //float zoomfactor=(m_DasherOX-MouseX)/(m_DasherOX*1.0);

  // Then zoom in appropriately
  m_Rootmax = m_Rootmax +myint(zoomfactor*(m_Rootmax-m_DasherY/2));
  m_Rootmin = m_Rootmin +myint(zoomfactor*(m_Rootmin-m_DasherY/2));

  // Afterwards, we need to take care of the vertical offset.
  myint up=(m_DasherY/2)-MouseY;
  m_Rootmax = m_Rootmax + up;
  m_Rootmin = m_Rootmin + up;
}

myint CDasherModel::PlotGoTo(myint MouseX, myint MouseY)
{
  // First, we need to work out how far we need to zoom in
  double zoomfactor=(m_DasherOX-MouseX)/(m_DasherOX*1.0);
  zoomfactor=pow(0.5,zoomfactor);

  myint height=int(m_DasherY*zoomfactor/2);

  return height;
}
  

/////////////////////////////////////////////////////////////////////////////

void CDasherModel::Tap_on_display(myint miMousex,myint miMousey, unsigned long Time, Dasher::VECTOR_SYMBOL_PROB* vectorAdded, int* numDeleted) 
	// work out the next viewpoint, opens some new nodes
{
    if (vectorAdded != NULL)
	    vectorAdded->clear();
    if (numDeleted != NULL)
	    *numDeleted = 0;

        // Find out the current node under the crosshair
        CDasherNode *old_under_cross=Get_node_under_crosshair();	
	
	// works out next viewpoint
	total_nats += Get_new_root_coords(miMousex,miMousey);

	// opens up new nodes

	// push node under mouse
	CDasherNode* pUnderMouse = Get_node_under_mouse(miMousex,miMousey);

	Push_Node(pUnderMouse);

	if (Framerate() > 4) 
	{
		// push node under mouse but with x coord on RHS
		CDasherNode* pRight = Get_node_under_mouse(50,miMousey);
		Push_Node(pRight);
	}

	if (Framerate() > 8) 
	{
		// push node under the crosshair
		CDasherNode* pUnderCross = Get_node_under_crosshair();
		Push_Node(pUnderCross);
	}

	int iRandom = RandomInt();

	if (Framerate() > 8) {
		// add some noise and push another node
		CDasherNode* pRight=Get_node_under_mouse(50,miMousey+iRandom%500-250);
		Push_Node(pRight);
	}

	iRandom=RandomInt();

	if (Framerate() > 15) {
		// add some noise and push another node
		CDasherNode *pRight=Get_node_under_mouse(50,miMousey+iRandom%500-250);
		Push_Node(pRight);
	}

	// only do this is Dasher is flying
	if (Framerate() > 30) 
	{
		for (int i=1;i<int(Framerate()-30)/3;i++) 
		{

			iRandom=RandomInt();
			// push at a random node on the RHS
			CDasherNode* pRight=Get_node_under_mouse(50,miMousey+iRandom%1000-500);
			Push_Node(pRight);

		}
	}

//	Update(m_Root,under_mouse,0);

	CDasherNode* new_under_cross = Get_node_under_crosshair();

	if (new_under_cross!=old_under_cross) {
	  DeleteCharacters(new_under_cross, old_under_cross, numDeleted);
	}

	if (new_under_cross->isSeen()==true) {
	  if (new_under_cross->ControlChild()!=true) {
	    SetBitrate(m_dMaxRate);
	  }
	  return;
	}

	new_under_cross->Seen(true);

	if (new_under_cross->ControlChild()==true) {
	  //		m_pEditbox->outputcontrol(new_under_cross->GetControlTree()->pointer,new_under_cross->GetControlTree()->data,new_under_cross->GetControlTree()->type);
		OutputCharacters(new_under_cross, vectorAdded);
		SetBitrate(m_dMaxRate/3);
	} else {
	  OutputCharacters(new_under_cross, vectorAdded);
	  SetBitrate(m_dMaxRate);
	}
	//	m_Root->Recursive_Push_Node(0);
}

void CDasherModel::GoTo(double zoomfactor, myint miMousey) 
	// work out the next viewpoint, opens some new nodes
{
        // Find out the current node under the crosshair
        CDasherNode *old_under_cross=Get_node_under_crosshair();	
	
	// works out next viewpoint
	Get_new_goto_coords(zoomfactor,miMousey);

	// push node under crosshair

	CDasherNode* new_under_cross = Get_node_under_crosshair();

	Push_Node(new_under_cross);
	
	// push node under goto point

	// We don't have a mousex, so "emulating" one.
	CDasherNode* node_under_goto = Get_node_under_mouse(50, miMousey);

	Push_Node(node_under_goto);
	
//	Update(m_Root,new_under_cross,0);

	if (new_under_cross!=old_under_cross) {
      int numDeleted = 0;
	  DeleteCharacters(new_under_cross,old_under_cross, &numDeleted);
	}

	if (new_under_cross->isSeen()==true)
	  return;

	new_under_cross->Seen(true);

	OutputCharacters(new_under_cross);
}

void CDasherModel::OutputCharacters(CDasherNode *node, Dasher::VECTOR_SYMBOL_PROB* vectorAdded) 
{
	if (vectorAdded != NULL)
		vectorAdded->clear();

  if (node->Parent()!=NULL && node->Parent()->isSeen()!=true) 
  {
    node->Parent()->Seen(true);
    OutputCharacters(node->Parent(), vectorAdded);
  }
  symbol t=node->Symbol();
  if (t) // SYM0
  {
		if (vectorAdded != NULL)
        {
            Dasher::SymbolProb item;
            item.sym    = t;
            item.prob   = node->GetProb();

			vectorAdded->push_back(item);
        }

    m_pEditbox->output(t);
  } 
  else if (node->ControlChild()==true) 
  {
	  m_pEditbox->outputcontrol(node->GetControlTree()->pointer,node->GetControlTree()->data,node->GetControlTree()->type);
  }
}

bool CDasherModel::DeleteCharacters (CDasherNode *newnode, CDasherNode *oldnode, int* numDeleted) 
{
	// DJW cant see how either of these can ever be NULL
	DASHER_ASSERT_VALIDPTR_RW(newnode);
	DASHER_ASSERT_VALIDPTR_RW(oldnode);

	if (newnode==NULL||oldnode==NULL)
		return false;

	// This deals with the trivial instance - we're reversing back over
	// text that we've seen already
	if (newnode->isSeen()==true) 
	{
		if (oldnode->Parent()==newnode) 
		{
			if (oldnode->Symbol()!= GetControlSymbol() && oldnode->ControlChild()==false && oldnode->Symbol()!=0)  // SYM0
			{
				m_pEditbox->deletetext(oldnode->Symbol());
				if (numDeleted != NULL)
					(*numDeleted)++;

			}
			oldnode->Seen(false);
			return true;
		}
		if (DeleteCharacters(newnode,oldnode->Parent(), numDeleted)==true)
		{
			if (oldnode->Symbol()!= GetControlSymbol() && oldnode->ControlChild()==false && oldnode->Symbol()!=0) // SYM0 
			{
				m_pEditbox->deletetext(oldnode->Symbol());
				if (numDeleted != NULL)
					(*numDeleted)++;
			}
			oldnode->Seen(false);
			return true;
		}
	} 
	else 
	{
		// This one's more complicated - the user may have moved onto a new branch
		// Find the last seen node on the new branch
		CDasherNode *lastseen=newnode->Parent();
		while (lastseen!=NULL && lastseen->isSeen()==false) 
		{      
			lastseen=lastseen->Parent();
		};
		// Delete back to last seen node
		while (oldnode!=lastseen) 
		{
			oldnode->Seen(false);
			if (oldnode->ControlChild()==true||oldnode->Symbol()== GetControlSymbol() || oldnode->Symbol()==0) 
			{
				oldnode=oldnode->Parent();
				continue;
			}
			m_pEditbox->deletetext(oldnode->Symbol());
			if (numDeleted != NULL)
				(*numDeleted)++;
			oldnode=oldnode->Parent();
			if (oldnode==NULL) {
				return false;
			}
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////

// Diagnostic trace
void CDasherModel::Trace() const 
{
	// OutputDebugString(TEXT(" ptr   symbol   context Next  Child    pushme pushed cscheme   lbnd  hbnd \n"));
	m_Root->Trace();
}


///////////////////////////////////////////////////////////////////


void CDasherModel::GetProbs(CLanguageModel::Context context, vector<symbol> &NewSymbols, 
							vector<unsigned int> &Probs, int iNorm, bool bLargeControl ) const
{
	// Total number of symbols
  int iSymbols = m_pcAlphabet->GetNumberSymbols(); // note that this includes the control node and the root node

	// Number of text symbols, for which the language model gives the distribution
	int iTextSymbols = m_pcAlphabet->GetNumberTextSymbols();

	NewSymbols.resize(iSymbols);
//	Groups.resize(iSymbols);
	for (int i=0;i<iSymbols;i++) 
	{
		NewSymbols[i]=i; // This will be replaced by something that works out valid nodes for this context
	//	Groups[i]=m_pcAlphabet->get_group(i);
	}

	// TODO - sort out size of control node - for the timebeing I'll fix the control node at 5%

	int uniform_add;
	int nonuniform_norm;
	int control_space;

	if( !m_bControlMode )
	{
	  control_space = 0;
	  uniform_add = ((iNorm * m_uniform) / 1000 ) / (iSymbols-2); // Subtract 2 from no symbols to lose control/root nodes
	  nonuniform_norm = iNorm - (iSymbols-2) * uniform_add;
	}
	else 
	{
	  if( bLargeControl )
	    control_space = int(iNorm * 0.1); // FIXME - constants here to be made parameters
	  else
	    control_space = int(iNorm * 0.01);

	  uniform_add = (((iNorm - control_space) * m_uniform / 1000 ) / (iSymbols-2) ); // Subtract 2 from no symbols to lose control/root nodes
	  nonuniform_norm = iNorm - control_space - (iSymbols-2) * uniform_add;
	}

	m_pLanguageModel->GetProbs(context,Probs,nonuniform_norm);


#if _DEBUG
	int iTotal = 0;
	for( int k=0; k < Probs.size(); ++k )
		iTotal += Probs[k];
	DASHER_ASSERT(iTotal == nonuniform_norm);
#endif

	for( int k=1; k < Probs.size(); ++k )
		Probs[k] += uniform_add;

	Probs.push_back( control_space );

#if _DEBUG
	iTotal = 0;
	for( int k=0; k < Probs.size(); ++k )
		iTotal += Probs[k];
//	DASHER_ASSERT(iTotal == iNorm);
#endif


}

///////////////////////////////////////////////////////////////////

void CDasherModel::SetUniform( int _uniform )
{ 
	m_uniform = _uniform; 
}

///////////////////////////////////////////////////////////////////

void CDasherModel::LearnText(CLanguageModel::Context context, string* TheText, bool IsMore)
{
  	vector<symbol> Symbols;
	
	m_pcAlphabet->GetSymbols(&Symbols, TheText, IsMore);
	
	for (unsigned int i=0; i<Symbols.size(); i++)
		m_pLanguageModel->LearnSymbol( context,  Symbols[i]);
}


///////////////////////////////////////////////////////////////////

void CDasherModel::EnterText(CLanguageModel::Context context, string TheText) const
{
	vector<symbol> Symbols;
	m_pcAlphabet->GetSymbols(&Symbols, &TheText, false);
	for (unsigned int i=0; i<Symbols.size(); i++)
		m_pLanguageModel->EnterSymbol(context, Symbols[i]);
}

///////////////////////////////////////////////////////////////////


CDasherModel::CTrainer::CTrainer(CDasherModel& DasherModel)
: m_DasherModel(DasherModel)
{
	m_Context = m_DasherModel.m_pLanguageModel->CreateEmptyContext();
}

///////////////////////////////////////////////////////////////////

void CDasherModel::CTrainer::Train(const std::vector<symbol>& vSymbols)
{
	for (int i=0; i<vSymbols.size() ; i++)
		m_DasherModel.m_pLanguageModel->LearnSymbol(m_Context,vSymbols[i]);
}

///////////////////////////////////////////////////////////////////

CDasherModel::CTrainer::~CTrainer()
{
	m_DasherModel.m_pLanguageModel->ReleaseContext(m_Context);

}

///////////////////////////////////////////////////////////////////

CDasherModel::CTrainer* CDasherModel::GetTrainer()
{
	return new CDasherModel::CTrainer(*this);
}

/////////////////////////////////////////////////////////////////////////////

/* gets probabilities and creates children for a node
 */

void CDasherModel::Push_Node(CDasherNode* pNode) 
{
	if ( pNode->Children() )
	{
		// if there are children just give them a poke
		unsigned int i;
		for (i=0;i< pNode->ChildCount() ;i++)
			pNode->Children()[i]->Alive(true);
		return;
	}

	// This ASSERT seems to routinely fail
	//DASHER_ASSERT(pNode->Symbol()!=0);


	// if we haven't got a context then derive it
	
	if (! pNode->Context() )
	{

		CLanguageModel::Context cont;
		// sym0
		if ( pNode->Symbol() < m_pcAlphabet->GetNumberTextSymbols() && pNode->Symbol()>0) 
		{
			CDasherNode* pParent = pNode->Parent();
			DASHER_ASSERT (pParent != NULL) ;
			// Normal symbol - derive context from parent
			cont =  m_pLanguageModel->CloneContext( pParent->Context() );
			m_pLanguageModel->EnterSymbol( cont, pNode->Symbol() );
		}
		else
		{
			// For new "root" nodes (such as under control mode), we want to 
			// mimic the root context
			cont = CreateEmptyContext();
			EnterText(cont, ". ");
		}
		pNode->SetContext(cont);

	}

	pNode->Alive(true);

	if ( pNode->Symbol()== GetControlSymbol() || pNode->ControlChild() ) 
	{

		ControlTree* pControlTreeChildren = pNode->GetControlTree();
	
		if ( pControlTreeChildren == NULL ) 
		{ 
			// Root of the tree 
			pControlTreeChildren = GetControlTree();
		}
		else 
		{ 
			// some way down
			pControlTreeChildren = pControlTreeChildren->children;
		}

		// Count total number of children

		// Always 1 child for a root symbol
		int iChildCount=1;

		// Control children
		ControlTree* pTemp = pControlTreeChildren;
		while(pTemp != NULL)
		{
			iChildCount++;
			pTemp = pTemp->next;
		}
		
		// Now we go back and build the node tree	  
		int quantum=int(Normalization()/iChildCount);

		CDasherNode** ppChildren = new CDasherNode* [iChildCount];

		ColorSchemes ChildScheme;
		if (pNode->ColorScheme() == Nodes1)
		{
			ChildScheme = Nodes2;
		} 
		else 
		{
			ChildScheme = Nodes1;
		}

		int i=0;
		// First a root node that takes up back to the text alphabet
		ppChildren[i]=new CDasherNode(*this,pNode,0,0,Opts::Nodes1,0,int((i+1)*quantum),m_pLanguageModel,false,240);
		i++;

		// Now the control children
		pTemp = pControlTreeChildren;
		while( pTemp != NULL)
		{
			if (pTemp->colour != -1)
			{
				ppChildren[i]=new CDasherNode(*this,pNode,0,i,ChildScheme,int(i*quantum),int((i+1)*quantum),m_pLanguageModel,true,pTemp->colour, pTemp);
			} 
			else 
			{
				ppChildren[i]=new CDasherNode(*this,pNode,0,i,ChildScheme,int(i*quantum),int((i+1)*quantum),m_pLanguageModel,true,(i%99)+11, pTemp);
			}
			i++;
			pTemp = pTemp->next;
		}
		pNode->SetChildren(ppChildren, iChildCount);
		return;
	}

	vector<symbol> newchars;   // place to put this list of characters
	vector<unsigned int> cum;   // for the probability list

	GetProbs(pNode->Context(),newchars,cum,Normalization(),m_pcAlphabet->GetSpaceSymbol() == pNode->Symbol());
	int iChildCount=newchars.size();

	DASHER_TRACEOUTPUT("ChildCount %d\n",iChildCount);
	// work out cumulative probs in place
	for (int i=1;i<iChildCount;i++)
		cum[i]+=cum[i-1];

	CDasherNode** ppChildren = new CDasherNode *[iChildCount];

	// create the children
	ColorSchemes NormalScheme, SpecialScheme;
	if (( pNode->ColorScheme()==Nodes1 ) || (pNode->ColorScheme()==Special1 ))
	{
		NormalScheme = Nodes2;
		SpecialScheme = Special2;
	} 
	else 
	{
		NormalScheme = Nodes1;
		SpecialScheme = Special1;
	}

	ColorSchemes ChildScheme;

	int iLbnd=0;
	for (int j=0; j< iChildCount; j++)
	{
		if (newchars[j]== GetSpaceSymbol())
			ChildScheme = SpecialScheme;
		else
			ChildScheme = NormalScheme;
		ppChildren[j]=new CDasherNode(*this,pNode,newchars[j],j,ChildScheme,iLbnd,cum[j],m_pLanguageModel,false,GetColour(j));
		iLbnd = cum[j];
	}
	pNode->SetChildren(ppChildren,iChildCount);

}

///////////////////////////////////////////////////////////////////

void CDasherModel::Recursive_Push_Node(CDasherNode* pNode, int iDepth) 
{

	if ( pNode->Range() < 0.1* Normalization()  ) 
	{
		return;
	}

	if ( pNode->Symbol() == GetControlSymbol() ) 
	{
		return;
	}

	Push_Node(pNode);

	if ( iDepth == 0 )
		return;

	for (int i=0; i< pNode->ChildCount(); i++) 
	{
		Recursive_Push_Node(pNode->Children()[i],iDepth-1);
	}
}
