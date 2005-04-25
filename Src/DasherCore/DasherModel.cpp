// DasherModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "../Common/Common.h"

#include "DasherModel.h"
#include "LanguageModelling/PPMLanguageModel.h"

using namespace Dasher;
using namespace std;

//////////////////////////////////////////////////////////////////////
// CDasherModel
//////////////////////////////////////////////////////////////////////

CDasherModel::CDasherModel(const CAlphabet* pAlphabet, CDashEditbox* pEditbox, LanguageModelID idLM, 
						   bool Dimensions, bool Eyetracker, bool Paused)
  : m_pcAlphabet(pAlphabet), m_pEditbox(pEditbox) ,
	 m_Dimensions(Dimensions),  m_Eyetracker(Eyetracker),  m_Paused(Paused), 
	 m_Root(0), m_iNormalization(1<<16),	m_uniform(50) , m_pLanguageModel(NULL)
{

	CSymbolAlphabet alphabet(pAlphabet->GetNumberSymbols());
	m_pLanguageModel = new CPPMLanguageModel(alphabet);
	
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


CDasherModel::~CDasherModel()
{

	// DJW 20031106 - hope to get it right this time
	
	if (oldroots.size()>0) { 
		delete oldroots[0];
		oldroots.clear();
		// At this point we have also deleted the root - so better NULL pointer
		m_Root=NULL;
	}
	
	delete m_Root;

	m_pLanguageModel->ReleaseContext(LearnContext);
	delete m_pLanguageModel;

}


void CDasherModel::Make_root(int whichchild)
 // find a new root node 
{

  symbol t=m_Root->Symbol();
  if (t) {  // SYM0
    m_pLanguageModel->LearnSymbol(LearnContext, t);
  }

	CDasherNode * oldroot=m_Root;
	
	CDasherNode **children=m_Root->Children();
	m_Root=children[whichchild];
	//	oldroot->Children()[whichchild]=0;  // null the pointer so we don't delete the whole tree
	//	delete oldroot;
	
	oldroots.push_back(oldroot);

	while (oldroots.size()>10) {
	  oldroots[0]->Delete_dead(oldroots[1]);
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


void CDasherModel::Update(CDasherNode *node,CDasherNode *under_mouse,int iSafe)
// go through the Dasher nodes, delete ones who have expired
// decrease the time left for nodes which arent safe
// safe nodes are those which are under the mouse or offspring of this node
{
  if (node==under_mouse) {
		iSafe=1;
  }
    	
	if (node->Alive()) {
		CDasherNode **children=node->Children();
		if (children) {
			unsigned int i;
			for (i=1;i<node->ChildCount();i++)
				Update(children[i],under_mouse,iSafe);
		}
	}
	return;
}

/////////////////////////////////////////////////////////////////////////////

void CDasherModel::Start()
{
	m_Rootmin=0;
	m_Rootmax=m_DasherY;

	//	total_nats = 0.0;

	
	/*
	// DJW 20031106 - this is unsafe - oldroots[1] is not valid when size == 1
	while (oldroots.size()>0) {
	  oldroots[0]->Delete_dead(oldroots[1]);
	  delete oldroots[0];
	  oldroots.pop_front();
	}
	*/

	// DJW 20031106 - hope to get it right this time
	
	if (oldroots.size()>0) { 
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
	m_Root->Recursive_Push_Node(0);
	
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
	CDasherNode *under_mouse=Get_node_under_mouse(miMousex,miMousey);

	under_mouse->Push_Node();

	if (Framerate() > 4) {
		// push node under mouse but with x coord on RHS
		CDasherNode *right=Get_node_under_mouse(50,miMousey);
		right->Push_Node();
	}


	if (Framerate() > 8) {
		// push node under the crosshair
		CDasherNode* under_cross=Get_node_under_crosshair();
		under_cross->Push_Node();
	}

	int iRandom;
#if defined(_WIN32_WCE)
	iRandom=Random();
#else
	iRandom=rand();
#endif
	if (Framerate() > 8) {
		// add some noise and push another node
		CDasherNode *right=Get_node_under_mouse(50,miMousey+iRandom%500-250);
		right->Push_Node();
	}
#if defined(_WIN32_WCE)
	iRandom=Random();
#else
	iRandom=rand();
#endif
	if (Framerate() > 15) {
		// add some noise and push another node
		CDasherNode *right=Get_node_under_mouse(50,miMousey+iRandom%500-250);
		right->Push_Node();
	}

	// only do this is Dasher is flying
	if (Framerate() > 30) {
		for (int i=1;i<int(Framerate()-30)/3;i++) {
#if defined(_WIN32_WCE)	
		iRandom=Random();
#else
		iRandom=rand();
#endif
		// push at a random node on the RHS
		CDasherNode *right=Get_node_under_mouse(50,miMousey+iRandom%1000-500);
		right->Push_Node();
	
		}
	}

	Update(m_Root,under_mouse,0);

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

	new_under_cross->Push_Node();

	// push node under goto point

	// We don't have a mousex, so "emulating" one.
	CDasherNode* node_under_goto = Get_node_under_mouse(50, miMousey);

	node_under_goto->Push_Node();

	Update(m_Root,new_under_cross,0);

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

void CDasherModel::Dump() const 
	// diagnostic dump
{
		// OutputDebugString(TEXT(" ptr   symbol   context Next  Child    pushme pushed cscheme   lbnd  hbnd \n"));
		m_Root->Dump_node();
}


///////////////////////////////////////////////////////////////////


void CDasherModel::GetProbs(CLanguageModel::Context context, vector<symbol> &NewSymbols,
		vector<unsigned int> &Groups, vector<unsigned int> &Probs, int iNorm) const
{
	int iSymbols = m_pcAlphabet->GetNumberSymbols();


	int uniform_add = ((iNorm / 1000 ) / iSymbols ) * m_uniform;
	int nonuniform_norm = iNorm - iSymbols * uniform_add;

	NewSymbols.resize(iSymbols);
	Groups.resize(iSymbols);
	for (int i=0;i<iSymbols;i++) 
	{
		NewSymbols[i]=i; // This will be replaced by something that works out valid nodes for this context
		Groups[i]=m_pcAlphabet->get_group(i);
	}

	m_pLanguageModel->GetProbs(context,Probs,nonuniform_norm);

	for( int k=0; k < Probs.size(); ++k )
		Probs[k] += uniform_add;

#if _DEBUG
	int iTotal = 0;
	for( int k=0; k < Probs.size(); ++k )
		iTotal += Probs[k];
	DASHER_ASSERT(iTotal == iNorm);
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

///////////////////////////////////////////////////////////////////
