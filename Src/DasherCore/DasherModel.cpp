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

CDasherModel::CDasherModel(CDashEditbox* Editbox, CLanguageModel* LanguageModel, bool Dimensions, bool Eyetracker)
  : m_editbox(Editbox), m_languagemodel(LanguageModel), m_Root(0), m_Dimensions(Dimensions), m_Eyetracker(Eyetracker)
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


	// DJW 20031106 - hope to get it right this time
	
	if (oldroots.size()>0) { 
		delete oldroots[0];
		oldroots.clear();
		// At this point we have also deleted the root - so better NULL pointer
		m_Root=NULL;
	}
	
	delete m_Root;


	/*
	// This is yukky

	while (oldroots.size()>1) {
	  oldroots[0]->Delete_dead(oldroots[1]);
	  delete oldroots[0];
	  oldroots.pop_front();
	}

	// even more yukky
	if (oldroots.size()==1)
	{
		 oldroots[0]->Delete_dead(m_Root);
		delete oldroots[0];
		oldroots.clear();
	}

	delete m_Root;  // which will also delete all the whole structure
*/

}


void CDasherModel::Make_root(int whichchild)
 // find a new root node 
{
  /*	symbol t=m_Root->Symbol();

	if (m_Root->Control()==true) {
	  m_editbox->outputcontrol(m_Root->GetControlTree()->pointer,m_Root->GetControlTree()->data);
	} else {
	  if (t) {
	    m_editbox->output(t);
	    m_languagemodel->LearnNodeSymbol(LearnContext, t);
	  }
	}
  */

  symbol t=m_Root->Symbol();
  if (t) {
    m_languagemodel->LearnNodeSymbol(LearnContext, t);
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
	m_Rootmax=m_Rootmin+(range*m_Root->Hbnd())/Normalization();
	m_Rootmin+=(range*m_Root->Lbnd())/Normalization();
}

void CDasherModel::Reparent_root(int lower, int upper)
{

  /* Change the root node to the parent of the existing node
     We need to recalculate the coordinates for the "new" root as the 
     user may have moved around within the current root */

  if (oldroots.size()==0) // There is no node to reparent to
    return;

  /* Determine how zoomed in we are */
  double scalefactor=(m_Rootmax-m_Rootmin)/static_cast<double>(upper-lower);

  m_Root=oldroots.back();
  oldroots.pop_back();
  
  m_Rootmax=int(m_Rootmax+((Normalization()-upper)*scalefactor));
  m_Rootmin=int(m_Rootmin-(lower*scalefactor));

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
	
	m_Root=new CDasherNode(0,0,0,0,Opts::Nodes1,0,Normalization(),m_languagemodel, false, 7);
	CLanguageModel::CNodeContext* therootcontext=m_languagemodel->GetRootNodeContext();

	if (m_editbox) {
		string ContextString;
		m_editbox->get_new_context(ContextString,5);
		if (ContextString.size()==0) {
		  // If there is no root context, pretend that we've just
		  // finished a sentence
		  ContextString=". " + ContextString;
		}
		m_languagemodel->EnterText(therootcontext, ContextString);
		m_languagemodel->ReleaseNodeContext(LearnContext);
		LearnContext = m_languagemodel->CloneNodeContext(therootcontext);
	}

	m_Root->Push_Node(therootcontext);
	m_Root->Recursive_Push_Node(0);
	
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
		if (Mousex<=0)
			Mousex=1;
		dRx=1.0*m_DasherOX/Mousex;
		dRxnew=pow(dRx,1.0/iSteps);  // or exp(log(rx)/steps) - i think the replacement is faster   
	
		dRxnew2=1+(dRx-1)/iSteps;
		
		const double dRxmax=m_fr.Rxmax();
		if (dRxnew>dRxmax)
		 dRxnew=dRxmax;
	} else {
		if (Mousex==m_DasherOX)
			Mousex++;
		dRx=1.0001*m_DasherOX/Mousex;
		dRxnew=exp(log(dRx)/iSteps);
		if (m_Rootmax<m_DasherY && m_Rootmin>0) {
		  return;
		}
	} 
	myint above=(Mousey-m_Rootmin);//*(1-rxnew)/(1-rx);
	myint below=(m_Rootmax-Mousey);//*(1-rxnew)/(1-rx);

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
        // Find out the current node under the crosshair
        CDasherNode *old_under_cross=Get_node_under_crosshair();	
	
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
		CDasherNode* under_cross=Get_node_under_crosshair();
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

	symbol t=new_under_cross->Symbol();

	if (new_under_cross->Control()==true) {
	  //		m_editbox->outputcontrol(new_under_cross->GetControlTree()->pointer,new_under_cross->GetControlTree()->data,new_under_cross->GetControlTree()->type);
		OutputCharacters(new_under_cross);
		SetBitrate(m_dMaxRate/3);
	} else {
	  OutputCharacters(new_under_cross);
	  SetBitrate(m_dMaxRate);
	}
	//	m_Root->Recursive_Push_Node(0);
}

void CDasherModel::GoTo(myint miMousex,myint miMousey) 
	// work out the next viewpoint, opens some new nodes
{
        // Find out the current node under the crosshair
        CDasherNode *old_under_cross=Get_node_under_crosshair();	
	
	// works out next viewpoint
	Get_new_goto_coords(miMousex,miMousey);

	// push node under crosshair

	CDasherNode* new_under_cross = Get_node_under_crosshair();

	new_under_cross->Push_Node();

	// push node under goto point

	CDasherNode* node_under_goto = Get_node_under_mouse(miMousex, miMousey);

	node_under_goto->Push_Node();

	Update(m_Root,new_under_cross,0);

	if (new_under_cross!=old_under_cross) {
	  DeleteCharacters(new_under_cross,old_under_cross);
	}

	if (new_under_cross->isSeen()==true)
	  return;

	new_under_cross->Seen(true);

	symbol t=new_under_cross->Symbol();

	OutputCharacters(new_under_cross);
}

void CDasherModel::OutputCharacters(CDasherNode *node) {
  if (node->Parent()!=NULL && node->Parent()->isSeen()!=true) {
    node->Parent()->Seen(true);
    OutputCharacters(node->Parent());
  }
  symbol t=node->Symbol();
  if (t) {
    m_editbox->output(t);
  } else if (node->Control()==true) {
	  m_editbox->outputcontrol(node->GetControlTree()->pointer,node->GetControlTree()->data,node->GetControlTree()->type);
  }
}

bool CDasherModel::DeleteCharacters (CDasherNode *newnode, CDasherNode *oldnode) {
  if (newnode==NULL||oldnode==NULL) {
    return false;
  }
  // This deals with the trivial instance - we're reversing back over
  // text that we've seen already
  if (newnode->isSeen()==true) {
    if (oldnode->Parent()==newnode) {
      if (oldnode->Symbol()!=m_languagemodel->GetControlSymbol()&&oldnode->Control()==false&&oldnode->Symbol()!=0) {
	m_editbox->deletetext(oldnode->Symbol());
      }
      oldnode->Seen(false);
      return true;
    }
    if (DeleteCharacters(newnode,oldnode->Parent())==true) {
      if (oldnode->Symbol()!=m_languagemodel->GetControlSymbol()&&oldnode->Control()==false&&oldnode->Symbol()!=0) {
	m_editbox->deletetext(oldnode->Symbol());
      }
      oldnode->Seen(false);
      return true;
    }
  } else {
    // This one's more complicated - the user may have moved onto a new branch
    // Find the last seen node on the new branch
    CDasherNode *lastseen=newnode->Parent();
    while (lastseen!=NULL && lastseen->isSeen()==false) {      
      lastseen=lastseen->Parent();
    };
    // Delete back to last seen node
    while (oldnode!=lastseen) {
      oldnode->Seen(false);
      if (oldnode->Control()==true||oldnode->Symbol()==m_languagemodel->GetControlSymbol() || oldnode->Symbol()==0) {
	oldnode=oldnode->Parent();
	continue;
      }
      m_editbox->deletetext(oldnode->Symbol());
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



