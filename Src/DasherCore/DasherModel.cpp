// DasherModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "DasherModel.h"

using namespace Dasher;
using namespace std;

//////////////////////////////////////////////////////////////////////
// CDasherModel
//////////////////////////////////////////////////////////////////////

CDasherModel::CDasherModel(CDashEditbox* Editbox, CLanguageModel* LanguageModel, bool Dimensions)
  : m_editbox(Editbox), m_languagemodel(LanguageModel), m_Root(0), m_Dimensions(Dimensions)
{
	LearnContext = m_languagemodel->GetRootNodeContext();
	
	// various settings
	int iShift = 12;
	m_DasherY = 1<<iShift;
	m_DasherOY = m_DasherY/2;
	m_DasherOX = m_DasherY/2;
	m_dAddProb = 0.003;
}


CDasherModel::~CDasherModel()
{
	m_languagemodel->ReleaseNodeContext(LearnContext);
	delete m_Root;  // which will also delete all the whole structure
}


void CDasherModel::Make_root(int whichchild)
 // find a new root node 
{
	symbol t=m_Root->Symbol();
	if (t) {
		m_editbox->output(t);
		m_languagemodel->LearnNodeSymbol(LearnContext, t);
	}

	CDasherNode * oldroot=m_Root;
	
	CDasherNode **children=m_Root->Children();
	m_Root=children[whichchild];
	//	oldroot->Children()[whichchild]=0;  // null the pointer so we don't delete the whole tree
	//	delete oldroot;
	
	oldroots.push_back(oldroot);

	myint range=m_Rootmax-m_Rootmin;
	m_Rootmax=m_Rootmin+(range*m_Root->Hbnd())/Normalization();
	m_Rootmin+=(range*m_Root->Lbnd())/Normalization();
}

void CDasherModel::Reparent_root(int lower, int upper)
{
  /* Change the root node to the parent of the existing node
     We need to recalculate the coordinates for the "new" root as the 
     user may have moved around within the current root */

  /* Determine how zoomed in we are */
	float scalefactor=(m_Rootmax-m_Rootmin)/(upper-lower);

	m_Rootmax=int(m_Rootmax+((1024-upper)*scalefactor));
	m_Rootmin=int(m_Rootmin-(lower*scalefactor));

	m_editbox->deletetext();

	m_Root=oldroots.back();
	oldroots.pop_back();
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


void CDasherModel::Flush(const myint Mousex,const myint Mousey)
{
	vector<symbol> vtUnder;
	Get_string_under_mouse(m_DasherOX,m_DasherOY,vtUnder);
	unsigned int i;
	for (i=0;i<vtUnder.size();i++) {
	  if (vtUnder[i]==0)
	    continue;
	  m_editbox->flush(vtUnder[i]);
	}
}

/////////////////////////////////////////////////////////////////////////////

void CDasherModel::Update(CDasherNode *node,CDasherNode *under_mouse,int iSafe)
// go through the Dasher nodes, delete ones who have expired
// decrease the time left for nodes which arent safe
// safe nodes are those which are under the mouse or offspring of this node
{
//	if (node->pushme )
//		node->push_node();
	if (node==under_mouse)
		iSafe=1;
	if (!iSafe)
		node->Age();
//	dchar debug[256];
//	wsprintf(debug,TEXT("node->Age %d %f\n"),node->Age, fr.framerate());
//	OutputDebugString(debug);
	
	
	if (node->Age() > Framerate())
		node->Kill();
	
	
	if (node->Alive()) {
		CDasherNode **children=node->Children();
		if (children) {
			unsigned int i;
			for (i=1;i<node->Chars();i++)
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
	
	delete m_Root;
	CLanguageModel::CNodeContext* therootcontext=m_languagemodel->GetRootNodeContext();
	
	//Rootparent=new DasherNode(0,0,0,therootcontext,0,0,0,Normalization(),languagemodel);	
	if (m_editbox) {
		m_editbox->set_flushed(0);
		string ContextString;
		m_editbox->get_new_context(ContextString,5);
		if (ContextString.size() != 0) {
		  m_languagemodel->EnterText(therootcontext, ContextString);
		}
		m_languagemodel->ReleaseNodeContext(LearnContext);
		LearnContext = m_languagemodel->CloneNodeContext(therootcontext);
	}
	m_Root=new CDasherNode(0,0,0,0,Opts::Nodes1,0,Normalization(),m_languagemodel);
	m_Root->Push_Node(therootcontext);
	
	m_languagemodel->ReleaseNodeContext(therootcontext);
//	ppmmodel->dump();
//	dump();
	
}

/////////////////////////////////////////////////////////////////////////////

void CDasherModel::Get_new_root_coords(myint Mousex,myint Mousey)
{
	int cappedrate=0;
	double dRx=1.0,dRxnew=1.0;
	double dRxnew2;

	int iSteps=m_fr.Steps();

	if (Mousex<m_DasherOX) {
	//	rx=1.0001*Ixmap[mx]/Ixmap[cx];
		if (Mousex<=0)
			Mousex=1;
		dRx=1.0*m_DasherOX/Mousex;
		dRxnew=pow(dRx,1.0/iSteps);  // or exp(log(rx)/steps) - i think the replacement is faster   
	
		dRxnew2=1+(dRx-1)/iSteps;
		//+(rx-1)*(rx-1)*(1.0/fr.steps()-1.0)/2/fr.steps();
		

		const double dRxmax=m_fr.Rxmax();
		if (dRxnew>dRxmax)
		 dRxnew=dRxmax;
		//		cappedrate=1;
	} else {
		if (Mousex==m_DasherOX)
			Mousex++;
	//		OutputDebugString(TEXT("zoom out\n"));
		dRx=1.0001*m_DasherOX/Mousex;
		dRxnew=exp(log(dRx)/iSteps);
	//	get_coords(root->lbnd,root->hbnd,&x1,&y1,&y2);
		//if (x1>0 || y1>0 || y2<CanvasY)
		//go_back_a_char();
		if (m_Rootmax<m_DasherY && m_Rootmin>0)
			return;
	} 
//	dchar debug[256];
//	_stprintf(debug,TEXT("rx %f rxnew %f approx %f\n"),rx,rxnew,rxnew2);
//	OutputDebugString(debug);
	//wsprintf(debug,TEXT("rx %f rxnew %f\n"),rx,rxnew);
	//OutputDebugString(debug);
	myint above=(Mousey-m_Rootmin);//*(1-rxnew)/(1-rx);
	myint below=(m_Rootmax-Mousey);//*(1-rxnew)/(1-rx);

//	wsprintf(debug,TEXT("above %I64d below %I64d \n"),above,below);
//	OutputDebugString(debug);

	myint miDistance=m_DasherY/2-Mousey;
	miDistance=myint(miDistance*(dRxnew-1)/(dRx-1));
	myint miNewrootzoom=Mousey+miDistance;

	myint newRootmax=miNewrootzoom+myint(below*dRxnew);
	myint newRootmin=miNewrootzoom-myint(above*dRxnew);
	if (newRootmin<m_DasherY/2 && newRootmax>m_DasherY/2 && newRootmax<LLONG_MAX && newRootmin>LLONG_MIN) {
		m_Rootmax=newRootmax;
		m_Rootmin=newRootmin;	
	}

}

/////////////////////////////////////////////////////////////////////////////

void CDasherModel::Tap_on_display(myint miMousex,myint miMousey, unsigned long Time) 
	// work out the next viewpoint, opens some new nodes
{
	// works out next viewpoint
	Get_new_root_coords(miMousex,miMousey);

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
		CDasherNode *under_cross=Get_node_under_crosshair();
		under_cross->Push_Node();
	}

	unsigned int iRandom;
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


}

/////////////////////////////////////////////////////////////////////////////

void CDasherModel::Dump() const 
	// diagnostic dump
{
		// OutputDebugString(TEXT(" ptr   symbol   context Next  Child    pushme pushed cscheme   lbnd  hbnd \n"));
		m_Root->Dump_node();
}



