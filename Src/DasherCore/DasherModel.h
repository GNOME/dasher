// DasherModel.h
//
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2005 David Ward
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __DasherModel_h__
#define __DasherModel_h__

#include "../Common/NoClones.h"

#include "LanguageModelling/LanguageModel.h"

#include "DashEdit.h"
#include "DasherNode.h"
#include "Alphabet.h"
#include <math.h>
#include "DasherTypes.h"
#include "FrameRate.h"
#include <vector>
#include <deque>

// The DasherModel represents the current state of Dasher
// It contains a tree of DasherNodes
//		knows the current viewpoint
//		knows how to evolve the viewpoint

namespace Dasher {class CDasherModel;}

class Dasher::CDasherModel : private NoClones
{
public:

	class CTrainer
	{
	public:
		CTrainer(CDasherModel& DasherModel);

		void Train(const std::vector<symbol>& vSymbols);

		~CTrainer();

	private:
		CLanguageModel::Context m_Context;
		CDasherModel& m_DasherModel;


	};

	typedef enum
	{
		idPPM =0,
		idBigram =1
	} LanguageModelID;


	CDasherModel(const CAlphabet* pAlphabet, CDashEditbox* Editbox, LanguageModelID idLM, bool Dimensions, bool Eyetracker, bool Paused);
	~CDasherModel();
	
	CTrainer* GetTrainer();

	// framerate functions
	void NewFrame(unsigned long Time) {m_fr.NewFrame(Time);}    // called everytime we render a new frame
	double Framerate () const {return m_fr.Framerate();}        // return the framerate
	void Reset_framerate(unsigned long Time) {m_fr.Reset(Time);}

	void Halt() {m_fr.Initialise();}
	
	// User control of speed
	void SetBitrate(double TargetRate) {m_fr.SetBitrate(TargetRate);} // Use or start at this bitrate
	void SetMaxBitrate(double MaxRate) {m_dMaxRate=MaxRate;m_fr.SetMaxBitrate(MaxRate);} // Cap any adaption at this rate
	
	// Whether control mode is active - if true, DasherModel will create control nodes
	//	- existing control modes will not be deleted, but will be inactive 
	void SetControlMode(bool b);

	std::string GroupLabel(int group) const {return m_pcAlphabet->GetGroupLabel(group);}
	int GroupColour(int group) const {return m_pcAlphabet->GetGroupColour(group);}


	// functions returning private data (read only access)
	myint Rootmin() const {return m_Rootmin;}
	myint Rootmax() const {return m_Rootmax;}
	myint DasherOX() const {return m_DasherOX;}
	myint DasherOY() const {return m_DasherOY;}
	
	CDasherNode* Root() const {return m_Root;}
	int Normalization() const {return m_iNormalization;}
	myint DasherY() const {return m_DasherY;}
	
	bool Dimensions() const {return m_Dimensions;}
	bool Eyetracker() const {return m_Eyetracker;}
    bool Paused() const {return m_Paused; }


	void OutputCharacters(CDasherNode *node);
	bool DeleteCharacters(CDasherNode *newnode, CDasherNode *oldnode);
	void Trace() const;                                              // diagnostics
	//void Learn_symbol(symbol Symbol) {m_languagemodel->learn_symbol(Symbol);} // feed character to language model

    void Set_dimensions(bool dimensions) {m_Dimensions=dimensions;}
    void Set_eyetracker(bool eyetracker) {m_Eyetracker=eyetracker;}
    void Set_paused(bool paused)         {m_Paused=paused;}
	
	void Tap_on_display(myint,myint, unsigned long Time);           // evolves the current viewpoint
	void GoTo(double,myint);                                         // jumps to a new viewpoint
	void Start();                                                   // initializes the data structure
	void Make_root(int whichchild);                                 // find a new root node
	void Reparent_root(int lower, int upper);                                 // change back to the previous root
	
	void ResetNats() 
	{
	  total_nats = 0;
	}
	
	double GetNats() 
	{
	  return total_nats;
	}
	
	
	void SetUniform( int _uniform );

	myint PlotGoTo(myint MouseX, myint MouseY);

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

	void EnterText(CLanguageModel::Context Context, std::string TheText) const;
	void LearnText(CLanguageModel::Context Context, std::string* TheText, bool IsMore);

	CLanguageModel::Context CreateEmptyContext() const;

	// Alphabet pass-through functions for widely needed information
	symbol GetSpaceSymbol() const {return m_pcAlphabet->GetSpaceSymbol();}
	symbol GetControlSymbol() const {return m_pcAlphabet->GetControlSymbol();}
	const std::string& GetDisplayText(int iSymbol) const {return m_pcAlphabet->GetDisplayText(iSymbol);}

private:

	/////////////////////////////////////////////////////////////////////////////

	// Interfaces
	
	CDashEditbox* m_pEditbox;           // pointer to the editbox
	CLanguageModel* m_pLanguageModel;   // pointer to the language model
	
	const CAlphabet* m_pcAlphabet;             // pointer to the alphabet
	
	CLanguageModel::Context LearnContext;        // Used to add data to model as it is entered
	
	/////////////////////////////////////////////////////////////////////////////


	CDasherNode* m_Root;
	
	// Old root notes
	std::deque<CDasherNode*> oldroots;

	// Rootmin and Rootmax specify the position of the root node in Dasher coords
	myint m_Rootmin,m_Rootmax;


	myint m_Rootmin_min, m_Rootmax_max;

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

    // Fraction to allocate to uniform dist. (*1000)
	int m_uniform;
	
	CFrameRate m_fr;                   // keep track of framerate

	double total_nats; // Information entered so far
	
	// the probability that gets added to every symbol
	double m_dAddProb;             

	double m_dMaxRate;
	
	CDasherNode* Get_node_under_mouse(myint smousex,myint smousey);
	CDasherNode* Get_node_under_crosshair();
	double Get_new_root_coords(myint mousex,myint mousey);
	void Get_new_goto_coords(double zoomfactor,myint mousey);
	void Get_string_under_mouse(const myint smousex,const myint smousey,std::vector<symbol> &str);

	void GetProbs(CLanguageModel::Context context, std::vector<symbol> &NewSymbols,
		std::vector<unsigned int> &Groups, std::vector<unsigned int> &Probs, int iNorm) const;

	void Push_Node(CDasherNode* pNode);      // give birth to children
	void Recursive_Push_Node(CDasherNode* pNode, int depth);


	int GetColour(symbol s) const;

	int m_iNormalization; // The arithmetic interval for child nodes

	ControlTree* m_pControltree;

	bool m_bControlMode;

	friend class CDasherNode;
};

/////////////////////////////////////////////////////////////////////////////

inline CLanguageModel::Context CDasherModel::CreateEmptyContext() const
{
	return m_pLanguageModel->CreateEmptyContext();
}

/////////////////////////////////////////////////////////////////////////////

inline int CDasherModel::GetColour(symbol s) const
{
	return m_pcAlphabet->GetColour(s);
}



#endif /* #ifndef __DasherModel_h__ */
