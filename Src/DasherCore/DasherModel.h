// DasherModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2004 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DasherModel_h__
#define __DasherModel_h__

#include "../Common/MSVC_Unannoy.h"
#include "DashEdit.h"
#include "DasherNode.h"
#include "DasherScreen.h"
#include "LanguageModel.h"
#include "../Common/NoClones.h"
#include <math.h>
#include "DasherTypes.h"
#include "FrameRate.h"
#include <vector>
#include <deque>

// The CDasherModel represents the current state of Dasher
// It contains a pointer to a structure of DasherNodes
// The knows the current viewpoint
// It also knows how to evolve the viewpoint

namespace Dasher {class CDasherModel;}
class Dasher::CDasherModel : private NoClones
{
public:
	
	CDasherModel(CDashEditbox* Editbox, CLanguageModel* LanguageModel, bool Dimensions, bool Eyetracker, bool Paused);
	~CDasherModel();
	
	// framerate functions
	void NewFrame(unsigned long Time) {m_fr.NewFrame(Time);}    // called everytime we render a new frame
	const double Framerate () {return m_fr.Framerate();}        // return the framerate

	void Halt() {m_fr.Initialise();}
	
	// User control of speed
	void SetBitrate(double TargetRate) {m_fr.SetBitrate(TargetRate);} // Use or start at this bitrate
	void SetMaxBitrate(double MaxRate) {m_dMaxRate=MaxRate;m_fr.SetMaxBitrate(MaxRate);} // Cap any adaption at this rate
	
	// functions returning private data (read only access)
	const myint Rootmin() const {return m_Rootmin;}
	const myint Rootmax() const {return m_Rootmax;}
	const myint DasherOX() const {return m_DasherOX;}
	const myint DasherOY() const {return m_DasherOY;}
	CDasherNode* Root() const {return m_Root;}
	int Normalization() const {return m_iNormalization;}
	myint DasherY() const {return m_DasherY;}
	bool Dimensions() const {return m_Dimensions;}
	bool Eyetracker() const {return m_Eyetracker;}
    bool Paused() const {return m_Paused; }


	void OutputCharacters(CDasherNode *node);
	bool DeleteCharacters(CDasherNode *newnode, CDasherNode *oldnode);
	void Dump() const;                                              // diagnostics
	//void Learn_symbol(symbol Symbol) {m_languagemodel->learn_symbol(Symbol);} // feed character to language model

       void Set_dimensions(bool dimensions) {m_Dimensions=dimensions;}
       void Set_eyetracker(bool eyetracker) {m_Eyetracker=eyetracker;}
       void Set_paused(bool paused)         {m_Paused=paused;}
	void Tap_on_display(myint,myint, unsigned long Time);           // evolves the current viewpoint
	void GoTo(float,myint);                                         // jumps to a new viewpoint
	void Start();                                                   // initializes the data structure
	void Make_root(int whichchild);                                 // find a new root node
	void Reparent_root(int lower, int upper);                                 // change back to the previous root
	void Reset_framerate(unsigned long Time) {m_fr.Reset(Time);}

	CAlphabet* m_alphabet;             // pointer to the alphabet
	
	CAlphabet* GetAlphabet() { return m_alphabet; }

	myint CDasherModel::PlotGoTo(myint MouseX, myint MouseY);

	void NewControlTree(ControlTree *tree) { m_pControltree=tree; }
	ControlTree* GetControlTree() const { return m_pControltree; }

	
	struct CRange
	{
		CRange(myint _iMin, myint _iMax) : iMin(_iMin), iMax(_iMax) {}
		CRange() {}
		myint iMin;
		myint iMax;
	};

	void SetActive(const CRange& range) { m_Active = range;}

private:

	// Old root notes
	std::deque<CDasherNode*> oldroots;

	// Rootmin and Rootmax specify the position of the root node in Dasher coords
	myint m_Rootmin,m_Rootmax;

	// Size of Dasher's arithmetic coding interval - it defines the Dasher coordinate system
	myint m_DasherY;

	// x position of crosshair in Dasher coords - distance from RHS is square Dasher
	myint m_DasherOX;    

	// y position of crosshair in Dasher coords - distance from top in square Dasher
	myint m_DasherOY;     

	// The active interval over which Dasher nodes are maintained - this is most likely bigger than (0,DasherY)
	CRange m_Active;

	// Number of input dimensions
	bool m_Dimensions;

	// Eyetracker mode
	bool m_Eyetracker;

	bool m_Paused;

	CDashEditbox* m_editbox;           // pointer to the editbox
	CLanguageModel* m_languagemodel;   // pointer to the language model
	CContext* LearnContext;        // Used to add data to model as it is entered
	CFrameRate m_fr;                   // keep track of framerate
	
	// TODO - move somewhere
	// the probability that gets added to every symbol
	double m_dAddProb;             

	double m_dMaxRate;
	
	CDasherNode* Get_node_under_mouse(myint smousex,myint smousey);
	CDasherNode* Get_node_under_crosshair();
	CDasherNode* m_Root;
	void Get_new_root_coords(myint mousex,myint mousey);
	void Get_new_goto_coords(float zoomfactor,myint mousey);
	void Get_string_under_mouse(const myint smousex,const myint smousey,std::vector<symbol> &str);
	void Update(CDasherNode* node,CDasherNode* under,int safe);


	int m_iNormalization; // The arithmetic interval for child nodes

	ControlTree* m_pControltree;


};

#endif /* #ifndef __DasherModel_h__ */
