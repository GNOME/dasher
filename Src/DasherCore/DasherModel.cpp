// DasherModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "DasherModel.h"

#include <iostream>

#include "../Common/Random.h"
#include "LanguageModelling/PPMLanguageModel.h"
#include "LanguageModelling/WordLanguageModel.h"


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
	 m_bControlMode(false)
{

  // Convert the full alphabet to a symbolic representation for use in the language model
  
  CSymbolAlphabet alphabet( pAlphabet->GetNumberTextSymbols() );
  alphabet.SetSpaceSymbol( pAlphabet->GetSpaceSymbol() ); // FIXME - is this right, or do we have to do some kind of translation?

  // Create an appropriate language model;

  // FIXME - return to using enum here

  switch( idLM ) {
  case idPPM:
    m_pLanguageModel = new CPPMLanguageModel(alphabet, _params);
    break;
  case idWord:
    m_pLanguageModel = new CWordLanguageModel(alphabet, _params);
    break;
  }
	
  LearnContext = m_pLanguageModel->CreateEmptyContext();
  
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
	if (t) 
	{  // SYM0
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
	
	m_Root=new CDasherNode(*this,0,0,0,0,Opts::Nodes1,0,Normalization(),m_pLanguageModel, false, 7);
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

double CDasherModel::Get_new_root_coords(myint Mousex,myint Mousey)
{
	double dRx=1.0*m_DasherOX/Mousex;
	
	double dRxnew;

	// URGH - floating point

	int iSteps=m_fr.Steps();

	DASHER_ASSERT(iSteps>0);

	if (Mousex<m_DasherOX) {

		if (Mousex<=0)
			Mousex=1;

		//dRxnew=pow(dRx,1.0/iSteps);  // or exp(log(rx)/steps) - i think the replacement is faster   
		dRxnew=1+(dRx-1)/iSteps;
		
		const double dRxmax=m_fr.Rxmax();
		if (dRxnew>dRxmax)
		 dRxnew=dRxmax;

	} 
	else 
	{
		dRxnew=1+(dRx-1)/iSteps;

		// Stop zooming out when no parents
		if (m_Rootmax<m_DasherY && m_Rootmin> myint(0) ) 
			return(1.0);
	} 

	// dRxnew is the scale factor (treat the above as a definition)

	myint above=(Mousey-m_Rootmin);//*(1-rxnew)/(1-rx);
	myint below=(m_Rootmax-Mousey);//*(1-rxnew)/(1-rx);

	// Distances above and below the mouse cursor of the top and bottom of the canvas

	// The below is horrible - fix it sometime

	myint miDistance=m_DasherY/2-Mousey;


	if (Mousex!=m_DasherOX)
		miDistance=myint(miDistance*(dRxnew-1)/(dRx-1));
	else
		miDistance = miDistance/iSteps;

	myint miNewrootzoom=Mousey+miDistance;

	myint newRootmax=miNewrootzoom+myint(below*dRxnew);
	myint newRootmin=miNewrootzoom-myint(above*dRxnew);

	if (newRootmin>=m_DasherY/2)  
		newRootmin= m_DasherY/2-1;
	if (newRootmax<=m_DasherY/2)  
		newRootmax= m_DasherY/2+1;

	// Need to check whether we've expanded beyond limit
	if (newRootmax<m_Rootmax_max && newRootmin > m_Rootmin_min)    
	{
		m_Rootmax=newRootmax;
		m_Rootmin=newRootmin;
	} 
	else
	{
		// TODO - force a new root to be chosen
	}

	return log( dRxnew );

}

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

void CDasherModel::Tap_on_display(myint miMousex,myint miMousey, unsigned long Time) 
	// work out the next viewpoint, opens some new nodes
{
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
	  DeleteCharacters(new_under_cross,old_under_cross);
	}

	if (new_under_cross->isSeen()==true) {
	  if (new_under_cross->Control()!=true) {
	    SetBitrate(m_dMaxRate);
	  }
	  return;
	}

	new_under_cross->Seen(true);

	if (new_under_cross->Control()==true) {
	  //		m_pEditbox->outputcontrol(new_under_cross->GetControlTree()->pointer,new_under_cross->GetControlTree()->data,new_under_cross->GetControlTree()->type);
		OutputCharacters(new_under_cross);
		SetBitrate(m_dMaxRate/3);
	} else {
	  OutputCharacters(new_under_cross);
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
	  DeleteCharacters(new_under_cross,old_under_cross);
	}

	if (new_under_cross->isSeen()==true)
	  return;

	new_under_cross->Seen(true);

	OutputCharacters(new_under_cross);
}

void CDasherModel::OutputCharacters(CDasherNode *node) 
{
  if (node->Parent()!=NULL && node->Parent()->isSeen()!=true) 
  {
    node->Parent()->Seen(true);
    OutputCharacters(node->Parent());
  }
  symbol t=node->Symbol();
  if (t) // SYM0
  {
    m_pEditbox->output(t);
  } 
  else if (node->Control()==true) 
  {
	  m_pEditbox->outputcontrol(node->GetControlTree()->pointer,node->GetControlTree()->data,node->GetControlTree()->type);
  }
}

bool CDasherModel::DeleteCharacters (CDasherNode *newnode, CDasherNode *oldnode) 
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
			if (oldnode->Symbol()!= GetControlSymbol() && oldnode->Control()==false && oldnode->Symbol()!=0)  // SYM0
			{
				m_pEditbox->deletetext(oldnode->Symbol());
			}
			oldnode->Seen(false);
			return true;
		}
		if (DeleteCharacters(newnode,oldnode->Parent())==true)
		{
			if (oldnode->Symbol()!= GetControlSymbol() && oldnode->Control()==false && oldnode->Symbol()!=0) // SYM0 
			{
				m_pEditbox->deletetext(oldnode->Symbol());
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
			if (oldnode->Control()==true||oldnode->Symbol()== GetControlSymbol() || oldnode->Symbol()==0) 
			{
				oldnode=oldnode->Parent();
				continue;
			}
			m_pEditbox->deletetext(oldnode->Symbol());
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
		vector<unsigned int> &Groups, vector<unsigned int> &Probs, int iNorm) const
{
	// Total number of symbols
	int iSymbols = m_pcAlphabet->GetNumberSymbols();

	// Number of text symbols, for which the language model gives the distribution
	int iTextSymbols = m_pcAlphabet->GetNumberTextSymbols();




	NewSymbols.resize(iSymbols);
	Groups.resize(iSymbols);
	for (int i=0;i<iSymbols;i++) 
	{
		NewSymbols[i]=i; // This will be replaced by something that works out valid nodes for this context
		Groups[i]=m_pcAlphabet->get_group(i);
	}

	// TODO - sort out size of control node - for the timebeing I'll fix the control node at 5%

	int uniform_add;
	int nonuniform_norm;
	int control_space;

	if( !m_bControlMode ) {
	  control_space = 0;
	  uniform_add = ((iNorm / 1000 ) / (iSymbols-1) ) * m_uniform; // Subtract 1 from no symbols to lose control node
	  nonuniform_norm = iNorm - (iSymbols-1) * uniform_add;
	}
	else {
	  control_space = iNorm * 0.05;
	  uniform_add = (((iNorm - control_space) / 1000 ) / (iSymbols-1) ) * m_uniform; // Subtract 1 from no symbols to lose control node
	  nonuniform_norm = iNorm - control_space - (iSymbols-1) * uniform_add;
	}

	m_pLanguageModel->GetProbs(context,Probs,nonuniform_norm);


#if _DEBUG
	int iTotal = 0;
	for( int k=0; k < Probs.size(); ++k )
		iTotal += Probs[k];
	DASHER_ASSERT(iTotal == nonuniform_norm);
#endif

	for( int k=0; k < Probs.size(); ++k )
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

//	DASHER_ASSERT(pNode->Symbol()!=0);

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

	if ( pNode->Symbol()== GetControlSymbol() || pNode->Control() ) 
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
		ppChildren[i]=new CDasherNode(*this,pNode,0,0,0,Opts::Nodes1,0,int((i+1)*quantum),m_pLanguageModel,false,240);
		i++;

		// Now the control children
		pTemp = pControlTreeChildren;
		while( pTemp != NULL)
		{
			if (pTemp->colour != -1)
			{
				ppChildren[i]=new CDasherNode(*this,pNode,0,0,i,ChildScheme,int(i*quantum),int((i+1)*quantum),m_pLanguageModel,true,pTemp->colour, pTemp);
			} 
			else 
			{
				ppChildren[i]=new CDasherNode(*this,pNode,0,0,i,ChildScheme,int(i*quantum),int((i+1)*quantum),m_pLanguageModel,true,(i%99)+11, pTemp);
			}
			i++;
			pTemp = pTemp->next;
		}
		pNode->SetChildren(ppChildren, iChildCount);
		return;
	}

	vector<symbol> newchars;   // place to put this list of characters
	vector<unsigned int> cum,groups;   // for the probability list

	GetProbs(pNode->Context(),newchars,groups,cum,Normalization());
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
	for (int i=0; i< iChildCount; i++)
	{
		if (newchars[i]== GetSpaceSymbol())
			ChildScheme = SpecialScheme;
		else
			ChildScheme = NormalScheme;
		ppChildren[i]=new CDasherNode(*this,pNode,newchars[i],groups[i],i,ChildScheme,iLbnd,cum[i],m_pLanguageModel,false,GetColour(i));
		iLbnd = cum[i];
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
