

// DasherModel.h
//
// Copyright (c) 2001-2005 David Ward

#ifndef __DasherModel_h__
#define __DasherModel_h__

#ifndef WIN32
#include "config.h"
#endif

#include "../Common/NoClones.h"

#include "LanguageModelling/LanguageModel.h"
#include "DasherNode.h"
#include "DasherComponent.h"
#include "Alphabet/Alphabet.h"
#include "AlphabetManagerFactory.h"
#include "ControlManagerFactory.h"
#include "ConversionManagerFactory.h"
#include "NodeCreationManager.h"

#include <math.h>
#include "DasherTypes.h"
#include "FrameRate.h"
#include <vector>
#include <deque>

namespace Dasher {
  class CDasherModel;
  class CDasherInterfaceBase;
  class CDasherView;
}

///
/// \brief Dasher 'world' data structures and dynamics.
///
/// The DasherModel represents the current state of Dasher
/// It contains a tree of DasherNodes
///             knows the current viewpoint
///             knows how to evolve the viewpoint

class Dasher::CDasherModel:public Dasher::CDasherComponent, private NoClones
{
 public:

  /// 
  /// Member class used to train the language model
  ///

  class CTrainer {
  public:
    CTrainer(CDasherModel & DasherModel);

    void Train(const std::vector < symbol > &vSymbols);

    ~CTrainer();

  private:
    CLanguageModel::Context m_Context;
    CDasherModel & m_DasherModel;

  };

  typedef enum {
    idPPM = 0,
    idBigram = 1,
    idWord = 2,
    idMixture = 3,
    idJapanese = 4
  } LanguageModelID;

  CDasherModel(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CNodeCreationManager *pNCManager, CDasherInterfaceBase * pDashIface, bool bGameMode = false, const std::string &strGameModeText = "");
  ~CDasherModel();

  ///
  /// Prototype binary dump of language model data
  ///

  bool WriteLMToFile(const std::string &strFilename) {
    if(m_pLanguageModel)
      return m_pLanguageModel->WriteToFile(strFilename);
    else
      return false;
  }

  ///
  /// Prototype binary read of language model data
  ///

  bool ReadLMFromFile(const std::string &strFilename) {
    if(m_pLanguageModel)
      return m_pLanguageModel->ReadFromFile(strFilename);
    else
      return false;
  }

  ///
  /// Evet handler
  ///

  void HandleEvent(Dasher::CEvent * pEvent);

  ///
  /// Return a trainer object for the language model
  ///

  CTrainer *GetTrainer();
  
  ///
  /// Notify the framerate class that a new frame has occurred
  ///

  void NewFrame(unsigned long Time) {
    m_fr.NewFrame(Time);
  }

  ///
  /// Get the current framerate
  ///

  double Framerate() const {
    return m_fr.Framerate();
  }

  ///
  /// Reset the framerate class
  ///

  void Reset_framerate(unsigned long Time) {
    m_fr.Reset(Time);
  }

  ///
  /// Initialise the framerate class - presumably called whenever
  /// dasher is stoppe, but the actual semantics here need to be
  /// verified
  ///

  void Halt() {
    m_fr.Initialise();
  }

  ///
  /// Set the target bitrate - probably shouldn't be called externally
  /// - could implement through the event subsystem instead
  ///
  
  void SetBitrate(double TargetRate) {
    m_fr.SetBitrate(TargetRate);
  } 

  ///
  /// Set the context in which predictions are made - need to check
  /// the semantics here. Probably a sensible thing to do here is to
  /// not allow context changes within an existing model, but to force
  /// a rebuild. This will only work once the language model has been
  /// decoupled, as retraining every time would be somewhat
  /// inconvenient.
  ///

  void SetContext(std::string & sNewContext);

/*   /// */
/*   /// What does this do? */
/*   /// */

/*   void Trace() const;  */

  ///
  /// Update the root location - called in response to regular timer
  /// callbacks
  ///

  bool UpdatePosition(myint, myint, unsigned long iTime, Dasher::VECTOR_SYMBOL_PROB* pAdded = NULL, int* pNumDeleted = NULL);  
  
  ///
  /// Check semantics here
  ///

  void Start();                 // initializes the data structure

  ///
  /// Clear the queue of old roots - used when those nodes become
  /// invalid, eg during changes to conrol mode
  ///

  void ClearRootQueue();

  ///
  /// Reset counter of total nats entered
  ///
 
  void ResetNats() {
    m_dTotalNats = 0.0;
  }

  ///
  /// Return the total nats entered
  ///

  double GetNats() {
    return m_dTotalNats;
  }

  ///
  /// Update a language model context to include an additional character
  ///

  void EnterText(CLanguageModel::Context Context, std::string TheText) const;

  ///
  /// Update a language model context to include an additional
  /// character, also learning the new data.

  void LearnText(CLanguageModel::Context Context, std::string * TheText, bool IsMore);

  ///
  /// Create an empty language model context
  ///

  CLanguageModel::Context CreateEmptyContext() const;

  ///
  /// Get the symbol ID representing space
  ///

  symbol GetSpaceSymbol() const {
    return m_pNodeCreationManager->GetSpaceSymbol();
  }

  ///
  /// Get the symbol ID representing the control node 
  ///

  symbol GetControlSymbol() const {
    return m_pNodeCreationManager->GetControlSymbol();
  }

  ///
  /// Get the symbol ID representing the conversion pseudo-character
  ///

  symbol GetStartConversionSymbol() const {
    return m_pNodeCreationManager->GetStartConversionSymbol();
  }

  ///
  /// Convert a given symbol ID to display text
  ///

  const std::string & GetDisplayText(int iSymbol) const {
    return m_pNodeCreationManager->GetDisplayText(iSymbol);
  }

  ///
  /// Get a root node of a given type
  ///

  CDasherNode *GetRoot( int iType, CDasherNode *pParent, int iLower, int iUpper, void *pUserData ) {
    return m_pNodeCreationManager->GetRoot(iType, pParent, iLower, iUpper, pUserData);
  };
  
  ///
  /// Get the node creation manager (this is temporary - eventually
  /// the node creation manager will be owned outside of this class)
  ///

/* /\*   CNodeCreationManager *GetNodeCreationManager() { *\/ */
/* /\*     return m_pNodeCreationManager; *\/ */
/*   }; */

  // TODO - only public temporarily - sort this out
  void GetProbs(CLanguageModel::Context context, std::vector < symbol > &NewSymbols, std::vector < unsigned int >&Probs, int iNorm) const;
  int GetColour(symbol s) const;
  

  ///
  /// TODO: Move these elsewhere
  ///

  //  bool m_bContextSensitive;
  std::string m_strContextBuffer;

  /// 
  /// Render the model to a given view
  ///

  bool RenderToView(CDasherView *pView, bool bRedrawDisplay);

  ///
  /// Schedule zoom to a given Dasher coordinate (used in click mode,
  /// button mode etc.)
  ///

  void ScheduleZoom(int dasherx, int dashery);

  ///
  /// Return the number of remaining zoom steps
  ///

  int ScheduledSteps() {
    return m_deGotoQueue.size();
  }

  ///
  /// Apply an offset to the 'target' coordinates - implements the jumps in
  /// two button dynamic mode.
  ///

  void Offset(int iOffset);
 
  ///
  /// Reset the 'target' root coordinates to match those currently visible. 
  /// Appropriate for abrubt changes in behaviour (such as backing off in 
  /// button modes)
  /// 

  void MatchTarget(bool bReverse);

  ///
  /// This is pretty horrible - a rethink of the start/reset mechanism
  /// is definitely in order. Used to prevent the root node from being
  /// too large in various modes before Dasher is started.
  ///

  void LimitRoot(int iMaxWidth);

  ///
  /// Cause Dasher to temporarily slow down (eg as part of automatic
  /// speed control in n-button dynamic mode).
  ///

  void TriggerSlowdown() {
    m_iStartTime = 0;
  };

  ///
  /// Return whether Dasher is currently being temporarily slowed or
  /// not.
  ///

  bool IsSlowdown(unsigned long iTime) {
    return ((iTime - m_iStartTime) < static_cast<unsigned long>(GetLongParameter(LP_SLOW_START_TIME)));
  };

  ///
  /// Check whether a change of root node is needed, and perform the
  /// update if so
  ///

  bool CheckForNewRoot(CDasherView *pView);

 protected:
  int m_iRenderCount;


 private:

  CDasherInterfaceBase * m_pDasherInterface;

  /////////////////////////////////////////////////////////////////////////////

  // Interfaces

  CLanguageModel *m_pLanguageModel;     // pointer to the language model

  CLanguageModel::Context LearnContext; // Used to add data to model as it is entered

  /////////////////////////////////////////////////////////////////////////////

  CDasherNode *m_Root;

  // Old root notes
  std::deque < CDasherNode * >oldroots;

  // Rootmin and Rootmax specify the position of the root node in Dasher coords
  myint m_Rootmin, m_Rootmax;
  
  myint m_iTargetMin;
  myint m_iTargetMax;

  myint m_iTargetOffset; // Displayed rootmin/max - actual rootmin/rootmax

  myint m_Rootmin_min, m_Rootmax_max;

  // The active interval over which Dasher nodes are maintained - this is most likely bigger than (0,DasherY)
  //  CRange m_Active;

  CFrameRate m_fr;              // keep track of framerate

  double m_dTotalNats;            // Information entered so far

  // the probability that gets added to every symbol
  double m_dAddProb;

  double m_dMaxRate;

  int m_Stepnum;

  CDasherNode *Get_node_under_mouse(myint smousex, myint smousey);

  void Get_new_root_coords(myint mousex, myint mousey, myint &iNewMin, myint &iNewMax, unsigned long iTime);

 // void Get_new_goto_coords(double zoomfactor, myint mousey);

  void Get_string_under_mouse(const myint smousex, const myint smousey, std::vector < symbol > &str);

  double CorrectionFactor(int dasherx, int dashery);

  void Recursive_Push_Node(CDasherNode * pNode, int depth);

  ///
  /// Perform output on a node - recurse up the tree outputting any
  /// symbols which have not been output so far. Neeed to check
  /// behaviour with respect to deletion
  ///

  void RecursiveOutput(CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded);


  ///
  /// Go directly to a given coordinate - check semantics
  ///
  
  void NewGoTo(myint n1, myint n2, Dasher::VECTOR_SYMBOL_PROB* pAdded, int* pNumDeleted);

  ///
  /// Check semantics here
  ///

  void OutputCharacters(CDasherNode * node);
  bool DeleteCharacters(CDasherNode * newnode, CDasherNode * oldnode, int* pNumDeleted = NULL);

  ///
  /// Create the children of a Dasher node
  ///

  void Push_Node(CDasherNode * pNode); 

  /// 
  /// Old style drilling down of nodes - optionally can still be
  /// called
  ///

  void OldPush(myint iMousex, myint iMousey);

  ///
  /// Schedule smooth transition to a given coordinate
  ///

  double Plan_new_goto_coords(int iRxnew, myint mousey, int *iSteps, myint *o1, myint *o2 , myint *n1, myint *n2); 

  /// 
  /// Make a child of the root into a new root
  ///

  void Make_root(CDasherNode *whichchild); 

  ///
  /// A version of Make_root which is suitable for arbitrary
  /// descendents of the root, not just immediate children.
  ///

  void RecursiveMakeRoot(CDasherNode *pNewRoot);

  ///
  /// Rebuild the data structure such that a given node is guaranteed
  /// to be in the same place on the screen. This would usually be the
  /// node under the crosshair
  ///

  void RebuildAroundNode(CDasherNode *pNode);

  ///
  /// Rebuild the parent of the current root - used during backing off
  ///

  void Reparent_root(int lower, int upper); 

  ///
  /// Return a pointer to the Dasher node which is currently under the
  /// crosshair. Apparently this is needed for game mode.
  ///

  CDasherNode *Get_node_under_crosshair();    

  ///
  /// Handle the output caused by a change in node being over the
  /// crosshair
  ///

  void HandleOutput(CDasherNode *pNewNode, CDasherNode *pOldNode, Dasher::VECTOR_SYMBOL_PROB* pAdded, int* pNumDeleted);

  CNodeCreationManager *m_pNodeCreationManager;
  bool m_bGameMode;


  unsigned long m_iStartTime;

  struct SGotoItem {
    myint iN1;
    myint iN2;
    int iStyle;
  };

  std::deque<SGotoItem> m_deGotoQueue;
  
  // Whether characters entered by alphabet manager are expected to
  // require conversion.
  // TODO: Need to rethink this at some point.
  bool m_bRequireConversion;

  friend class CTrainer;
  friend class CDasherNode;

};

/////////////////////////////////////////////////////////////////////////////

inline CLanguageModel::Context CDasherModel::CreateEmptyContext() const {
  return m_pLanguageModel->CreateEmptyContext();
}

inline int CDasherModel::GetColour(symbol s) const { 
  return m_pNodeCreationManager->GetColour(s); 
}

#endif /* #ifndef __DasherModel_h__ */
