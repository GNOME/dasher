// DasherModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#include "DasherModel.h"

using namespace Dasher;
using namespace std;

//////////////////////////////////////////////////////////////////////
// CDasherModel
//////////////////////////////////////////////////////////////////////

CDasherModel::CDasherModel(CDashEditbox* Editbox, CLanguageModel* LanguageModel)
: m_editbox(Editbox), m_languagemodel(LanguageModel), m_Root(0)
{
	LearnContext = m_languagemodel->GetRootNodeContext();
	
	// various settings
	m_iShift = 12;
	m_isCanvasY = 1<<m_iShift;
	m_isCanvasOY = m_isCanvasY/2;
	m_isCanvasOX = m_isCanvasY/2;
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
	oldroot->Children()[whichchild]=0;  // null the pointer so we don't delete the whole tree
	delete oldroot;
	
	myint range=m_iRootmax-m_iRootmin;
	m_iRootmax=m_iRootmin+(range*m_Root->Hbnd())/Normalization();
	m_iRootmin+=(range*m_Root->Lbnd())/Normalization();
}


CDasherNode * CDasherModel::Get_node_under_crosshair()
{
	return m_Root->Get_node_under(Normalization(),m_iRootmin,m_iRootmax,m_isCanvasOX,m_isCanvasOY);
}


CDasherNode * CDasherModel::Get_node_under_mouse(myint miMousex,myint miMousey)
{
	return m_Root->Get_node_under(Normalization(),m_iRootmin,m_iRootmax,miMousex,miMousey);
}


void CDasherModel::Get_string_under_mouse(const myint miMousex,const myint miMousey, vector<symbol> &str)
{
	m_Root->Get_string_under(Normalization(),m_iRootmin,m_iRootmax,miMousex,miMousey,str);
	return;
}


void CDasherModel::Flush(const myint miMousex,const myint miMousey)
{
	vector<symbol> vtUnder;
	Get_string_under_mouse(m_isCanvasOX,m_isCanvasOY,vtUnder);
	unsigned int i;
	for (i=0;i<vtUnder.size();i++)
		m_editbox->flush(vtUnder[i]);
}


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


void CDasherModel::Start()
{
	m_iRootmin=0;
	m_iRootmax=m_isCanvasY;
	
	delete m_Root;
	CLanguageModel::CNodeContext* therootcontext=m_languagemodel->GetRootNodeContext();
	
	//Rootparent=new DasherNode(0,0,0,therootcontext,0,0,0,Normalization(),languagemodel);	
	if (m_editbox) {
		m_editbox->set_flushed(0);
		string ContextString;
		m_editbox->get_new_context(ContextString,5);
		m_languagemodel->EnterText(therootcontext, ContextString);
		m_languagemodel->ReleaseNodeContext(LearnContext);
		LearnContext = m_languagemodel->CloneNodeContext(therootcontext);
	}
	m_Root=new CDasherNode(0,0,0,0,Opts::Nodes1,0,Normalization(),m_languagemodel);
	m_Root->Push_Node(therootcontext);
	
	m_languagemodel->ReleaseNodeContext(therootcontext);
//	ppmmodel->dump();
//	dump();
	
}


void CDasherModel::Get_new_root_coords(myint miMousex,myint miMousey)
{
	int cappedrate=0;
	double dRx=1.0,dRxnew=1.0;
	double dRxnew2;

	int iSteps=m_fr.Steps();

	if (miMousex<m_isCanvasOX) {
	//	rx=1.0001*Ixmap[mx]/Ixmap[cx];
		if (miMousex<=0)
			miMousex=1;
		dRx=1.0*m_isCanvasOX/miMousex;
		dRxnew=pow(dRx,1.0/iSteps);  // or exp(log(rx)/steps) - i think the replacement is faster   
	
		dRxnew2=1+(dRx-1)/iSteps;
		//+(rx-1)*(rx-1)*(1.0/fr.steps()-1.0)/2/fr.steps();
		

		const double dRxmax=m_fr.Rxmax();
		if (dRxnew>dRxmax)
		 dRxnew=dRxmax;
		//		cappedrate=1;
	} else {
		if (miMousex==m_isCanvasOX)
			miMousex++;
	//		OutputDebugString(TEXT("zoom out\n"));
		dRx=1.0001*m_isCanvasOX/miMousex;
		dRxnew=exp(log(dRx)/iSteps);
	//	get_coords(root->lbnd,root->hbnd,&x1,&y1,&y2);
		//if (x1>0 || y1>0 || y2<CanvasY)
		//go_back_a_char();
		if (m_iRootmax<m_isCanvasY && m_iRootmin>0)
			return;
	} 
//	dchar debug[256];
//	_stprintf(debug,TEXT("rx %f rxnew %f approx %f\n"),rx,rxnew,rxnew2);
//	OutputDebugString(debug);
	//wsprintf(debug,TEXT("rx %f rxnew %f\n"),rx,rxnew);
	//OutputDebugString(debug);
	myint above=(miMousey-m_iRootmin);//*(1-rxnew)/(1-rx);
	myint below=(m_iRootmax-miMousey);//*(1-rxnew)/(1-rx);

//	wsprintf(debug,TEXT("above %I64d below %I64d \n"),above,below);
//	OutputDebugString(debug);

	myint miDistance=m_isCanvasY/2-miMousey;
	miDistance=myint(miDistance*(dRxnew-1)/(dRx-1));
	myint miNewrootzoom=miMousey+miDistance;

	myint newRootmax=miNewrootzoom+myint(below*dRxnew);
	myint newRootmin=miNewrootzoom-myint(above*dRxnew);
	if (newRootmin<m_isCanvasY/2 && newRootmax>m_isCanvasY/2 && newRootmax<INT_MAX && newRootmin>INT_MIN) {
		m_iRootmax=newRootmax;
		m_iRootmin=newRootmin;	
	}

}



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

void CDasherModel::Dump() const 
	// diagnostic dump
{
		// OutputDebugString(TEXT(" ptr   symbol   context Next  Child    pushme pushed cscheme   lbnd  hbnd \n"));
		m_Root->Dump_node();
}



////////////////////////////////////////////////////////////////////////////////////////////
//  Framerate
//
//  Keeps track of framerate
//  Also bitrate stuff
////////////////////////////////////////////////////////////////////////////////////////////



CDasherModel::CFramerate::CFramerate() {


 // maxbitrate should be user-defined and/or adaptive. Currently it is not.
#if defined(_WIN32_WCE)
	m_dMaxbitrate=5;
#else
	m_dMaxbitrate=5.5;
#endif

	m_dRXmax=2;  // only a transient effect
	m_iFrames=0;
	m_iSamples=1;

	// we dont know the framerate yet - play it safe by setting it high
	m_dFr=1<<5;

	// start off very slow until we have sampled framerate adequately
	m_iSteps=2000;
	m_iTime=0; // Hmmm, User must reset framerate before starting.
}

void CDasherModel::CFramerate::NewFrame(unsigned long Time) 
	// compute framerate if we have sampled enough frames
{
	
	m_iFrames++;

	if (m_iFrames==m_iSamples) {
		m_iTime2=Time;
		if (m_iTime2-m_iTime < 50) 
			m_iSamples++;      // increase sample size
		else if (m_iTime2-m_iTime > 80) {
			m_iSamples--;
			if (m_iSamples <2)
				m_iSamples=2;
		}
		if (m_iTime2-m_iTime) {
				m_dFr=m_iFrames*1000.0/(m_iTime2-m_iTime);
				m_iTime=m_iTime2;
				m_iFrames=0;

			}
		m_dRXmax=exp(m_dMaxbitrate*log(2)/m_dFr);
		m_iSteps=m_iSteps/2+(int)(-log(0.2)*m_dFr/log(2)/m_dMaxbitrate)/2;
	//	dchar debug[256];
	//		_stprintf(debug,TEXT("fr %f Steps %d samples %d time2 %d rxmax %f\n"),fr,Steps,samples,time2,RXmax);
	//	OutputDebugString(debug);
	
	}
}


void CDasherModel::CFramerate::Reset(unsigned long Time)
{
	m_iFrames=0;
	m_iTime=Time;
}


void CDasherModel::CFramerate::SetBitrate(double TargetRate)
{
	m_dMaxbitrate = TargetRate;
}


void CDasherModel::CFramerate::SetMaxBitrate(double MaxRate)
{
	m_dMaxbitrate = MaxRate;
}
