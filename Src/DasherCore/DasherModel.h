// DasherModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2002 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DasherModel_h__
#define __DasherModel_h__

#include "../Common/MSVC_Unannoy.h"
#include "DashEdit.h"
#include "DasherNode.h"
#include "LanguageModel.h"
#include "../Common/NoClones.h"
#include <math.h>
#include "DasherTypes.h"


// The CDasherModel represents the current state of Dasher
// It contains a pointer to a structure of DasherNodes
// The knows the current viewpoint
// It also knows how to evolve the viewpoint

namespace Dasher {class CDasherModel;}
class Dasher::CDasherModel : private NoClones
{
public:
	
	CDasherModel(CDashEditbox* Editbox, CLanguageModel* LanguageModel);
	~CDasherModel();
	
	// framerate functions
	void NewFrame(unsigned long Time) {m_fr.NewFrame(Time);}    // called everytime we render a new frame
	const double Framerate () {return m_fr.Framerate();}        // return the framerate
	
	// User control of speed
	void SetBitrate(double TargetRate) {m_fr.SetBitrate(TargetRate);} // Use or start at this bitrate
	void SetMaxBitrate(double MaxRate) {m_fr.SetMaxBitrate(MaxRate);} // Cap any adaption at this rate
	
	// functions returning private data (read only access)
	const myint Rootmin() const {return m_Rootmin;}
	const myint Rootmax() const {return m_Rootmax;}
	const myint DasherOX() const {return m_DasherOX;}
	CDasherNode* Root() const {return m_Root;}
	int Normalization() const {return m_languagemodel->normalization();}
	myint DasherY() const {return m_DasherY;}

	void Dump() const;                                              // diagnostics
	void Flush(const myint smousex,const myint smousey);            // flush to the edit control
	//void Learn_symbol(symbol Symbol) {m_languagemodel->learn_symbol(Symbol);} // feed character to language model
	
	void Tap_on_display(myint,myint, unsigned long Time);           // evolves the current viewpoint
	void Start();                                                   // initializes the data structure
	void Make_root(int whichchild);                                 // find a new root node
	void Reset_framerate(unsigned long Time) {m_fr.Reset(Time);}
	
private:

	// keeps track of framerate
		// computes the Steps parameter
		// computes RXmax - which controls the maximum rate of zooming in
	
	class CFramerate {
	public:
		CFramerate() ;
		~CFramerate() {};
		const double Rxmax() const {return m_dRXmax;}
		const int Steps() const {return m_iSteps;}
		const double Framerate() const {return m_dFr;}
		void Reset(unsigned long Time);
		void NewFrame(unsigned long Time);
		
		// TODO: These two shouldn't be the same thing:
		void SetBitrate(double TargetRate);
		void SetMaxBitrate(double MaxRate);
	private:
		double m_dFr;
		double m_dMaxbitrate;    // the maximum rate of entering information
		double m_dRXmax;         // the maximum zoomin per frame
		int m_iFrames,m_iTime,m_iTime2,m_iSamples;
		int m_iSteps;            // the 'Steps' parameter. See djw thesis.
	};
	



	// Rootmin and Rootmax specify the position of the root node in Dasher coords
	myint m_Rootmin,m_Rootmax;

	// Size of Dasher's arithmetic coding interval - it defines the Dasher coordinate system
	myint m_DasherY;

	// x position of crosshair in Dasher coords - distance from RHS is square Dasher
	myint m_DasherOX;    

	// y position of crosshair in Dasher coords - distance from top in square Dasher
	myint m_DasherOY;     
	
	CDashEditbox* m_editbox;           // pointer to the editbox
	CLanguageModel* m_languagemodel;   // pointer to the language model
	CLanguageModel::CNodeContext* LearnContext;        // Used to add data to model as it is entered
	CAlphabet* m_alphabet;             // pointer to the alphabet
	CFramerate m_fr;                   // keep track of framerate
	
	// TODO - move somewhere
	// the probability that gets added to every symbol
	double m_dAddProb;             
	
	CDasherNode* Get_node_under_mouse(myint smousex,myint smousey);
	CDasherNode* Get_node_under_crosshair();
	CDasherNode* m_Root;
	void Get_new_root_coords(myint mousex,myint mousey);
	void Get_string_under_mouse(const myint smousex,const myint smousey,std::vector<symbol> &str);
	void Update(CDasherNode* node,CDasherNode* under,int safe);





};


#endif /* #ifndef __DasherModel_h__ */
