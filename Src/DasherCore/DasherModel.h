// DasherModel.h
//
// Copyright (c) 2001-2005 David Ward

#ifndef __DasherModel_h__
#define __DasherModel_h__

#include "../Common/NoClones.h"

#include "LanguageModelling/LanguageModel.h"
#include "DashEdit.h"
#include "DasherNode.h"
#include "DasherComponent.h"
#include "Alphabet/Alphabet.h"
#include "Alphabet/AlphIO.h"
#include "AlphabetManagerFactory.h"
#include "ControlManagerFactory.h"
#include <math.h>
#include "DasherTypes.h"
#include "FrameRate.h"
#include <vector>
#include <deque>
#include "DasherGameMode.h"


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

  CDasherModel(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CDasherInterfaceBase * pDashIface, CAlphIO *pAlphIO);
  ~CDasherModel();

  bool WriteLMToFile(const std::string &strFilename) {
    if(m_pLanguageModel)
      return m_pLanguageModel->WriteToFile(strFilename);
    else
      return false;
  }

  bool ReadLMFromFile(const std::string &strFilename) {
    if(m_pLanguageModel)
      return m_pLanguageModel->ReadFromFile(strFilename);
    else
      return false;
  }

  void HandleEvent(Dasher::CEvent * pEvent);

  CTrainer *GetTrainer();

  // framerate functions
  void NewFrame(unsigned long Time) {
    m_fr.NewFrame(Time);
  }

  // called everytime we render a new frame
  double Framerate() const {
    return m_fr.Framerate();
  }

  void Reset_framerate(unsigned long Time) {
    m_fr.Reset(Time);
  }

  void Halt() {
    m_fr.Initialise();
  }

  void RecursiveOutput(CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded);

  // User control of speed
  void SetBitrate(double TargetRate) {
    m_fr.SetBitrate(TargetRate);
  }                             // Use or start at this bitrate
  //void SetMaxBitrate(double MaxRate) {m_dMaxRate=MaxRate;m_fr.SetMaxBitrate(MaxRate);} // Cap any adaption at this rate

  std::string GroupLabel(int group) const {
    return m_pcAlphabet->GetGroupLabel(group);
  }
  int GroupColour(int group) const {
    return m_pcAlphabet->GetGroupColour(group);
  }
  void SetContext(std::string & sNewContext);

  // functions returning private data (read only access)
  myint Rootmin() const {
    return m_Rootmin;
  }
  myint Rootmax() const {
    return m_Rootmax;
  }
  CDasherNode *Root() const {
    return m_Root;
  }

  void OutputCharacters(CDasherNode * node);
  bool DeleteCharacters(CDasherNode * newnode, CDasherNode * oldnode, int* pNumDeleted = NULL);

  void Trace() const;           // diagnostics
  //void Learn_symbol(symbol Symbol) {m_languagemodel->learn_symbol(Symbol);} // feed character to language model

  void Tap_on_display(myint, myint, unsigned long Time, Dasher::VECTOR_SYMBOL_PROB* pAdded = NULL, int* pNumDeleted = NULL);  // evolves the current viewpoint

/*   void GoTo(double, myint);     // jumps to a new viewpoint */
   void NewGoTo(myint n1, myint n2, int style);
   double Plan_new_goto_coords(int iRxnew, myint mousey, int *iSteps, myint *o1, myint *o2 , myint *n1, myint *n2); 

  void Start();                 // initializes the data structure

  /// 
  /// Make a child of the root into a new root
  ///

  void Make_root(CDasherNode *whichchild); 

  ///
  /// Clear the queue of old roots - used when those nodes become
  /// invalid, eg during changes to conrol mode
  ///

  void ClearRootQueue();

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

  void Reparent_root(int lower, int upper);     // change back to the previous root

 
  void ResetNats() {
    total_nats = 0;
  }

  double GetNats() {
    return total_nats;
  }

  myint PlotGoTo(myint MouseX, myint MouseY);

  void EnterText(CLanguageModel::Context Context, std::string TheText) const;
  void LearnText(CLanguageModel::Context Context, std::string * TheText, bool IsMore);

  CLanguageModel::Context CreateEmptyContext()const;

  // Alphabet pass-through functions for widely needed information
  symbol GetSpaceSymbol() const {
    return m_pcAlphabet->GetSpaceSymbol();
  }

  symbol GetControlSymbol() const {
    return m_pcAlphabet->GetControlSymbol();
  }

  const std::string & GetDisplayText(int iSymbol) const {
    return m_pcAlphabet->GetDisplayText(iSymbol);
  }

  const CAlphabet & GetAlphabet() const {
    return *m_pcAlphabet;
  }

  CAlphabet *GetAlphabetNew() const {
    return m_pcAlphabet;
  }

  CDasherNode *Get_node_under_crosshair();    // Needed for Game Mode
  
  myint GetGameModePointerLoc() {
    return m_pGameMode->GetDasherCoordOfTarget();
  }
  
  CDasherNode *GetRoot( int iType, CDasherNode *pParent, int iLower, int iUpper, void *pUserData ) {
    if( iType == 0 )
      return m_pAlphabetManagerFactory->GetRoot(pParent, iLower, iUpper, pUserData);
    else
      return m_pControlManagerFactory->GetRoot(pParent, iLower, iUpper, pUserData);
  };
  
  // FIXME - only public temporarily
  void GetProbs(CLanguageModel::Context context, std::vector < symbol > &NewSymbols, std::vector < unsigned int >&Probs, int iNorm) const;
  int GetColour(symbol s) const;

  
  // Control mode stuff

  void RegisterNode( int iID, const std::string &strLabel, int iColour ) {
    m_pControlManagerFactory->RegisterNode(iID, strLabel, iColour);
  }
  
  void ConnectNode(int iChild, int iParent, int iAfter) {
     m_pControlManagerFactory->ConnectNode(iChild, iParent, iAfter);
  }

  void DisconnectNode(int iChild, int iParent) {
     m_pControlManagerFactory->DisconnectNode(iChild, iParent);
  }

  void Push_Node(CDasherNode * pNode);  // give birth to children

  bool m_bContextSensitive;

  std::string m_strContextBuffer;

  void RenderToView(CDasherView *pView);
  bool RenderToView(CDasherView *pView, int iMouseX, int iMouseY, bool bRedrawDisplay);

  bool CheckForNewRoot(CDasherView *pView);

  void ScheduleZoom(int dasherx, int dashery);
 
 private:

  CDasherInterfaceBase * m_pDasherInterface;

  /////////////////////////////////////////////////////////////////////////////

  // Interfaces

  CLanguageModel *m_pLanguageModel;     // pointer to the language model

  CAlphabet *m_pcAlphabet;        // pointer to the alphabet

  CLanguageModel::Context LearnContext; // Used to add data to model as it is entered

  /////////////////////////////////////////////////////////////////////////////

  CDasherGameMode *m_pGameMode;

  CDasherNode *m_Root;

  // Old root notes
  std::deque < CDasherNode * >oldroots;

  // Rootmin and Rootmax specify the position of the root node in Dasher coords
  myint m_Rootmin, m_Rootmax;

  myint m_Rootmin_min, m_Rootmax_max;

  // The active interval over which Dasher nodes are maintained - this is most likely bigger than (0,DasherY)
  //  CRange m_Active;

  CFrameRate m_fr;              // keep track of framerate

  double total_nats;            // Information entered so far

  // the probability that gets added to every symbol
  double m_dAddProb;

  double m_dMaxRate;

  int m_Stepnum;

  CDasherNode *Get_node_under_mouse(myint smousex, myint smousey);

  double Get_new_root_coords(myint mousex, myint mousey);

  void DoZoom(myint iTargetMin, myint iTargetMax);

  void Get_new_goto_coords(double zoomfactor, myint mousey);

  void Get_string_under_mouse(const myint smousex, const myint smousey, std::vector < symbol > &str);

  


  void Recursive_Push_Node(CDasherNode * pNode, int depth);

  //  ControlTree *m_pControltree;

  CAlphabetManagerFactory *m_pAlphabetManagerFactory;
  CControlManagerFactory *m_pControlManagerFactory;

  struct SGotoItem {
    myint iN1;
    myint iN2;
    int iStyle;
  };

  std::deque<SGotoItem> m_deGotoQueue;

  friend class CDasherGameMode;
  friend class CTrainer;
  friend class CDasherNode;

};

/////////////////////////////////////////////////////////////////////////////

inline CLanguageModel::Context CDasherModel::CreateEmptyContext() const {
  return m_pLanguageModel->CreateEmptyContext();
}

inline int CDasherModel::GetColour(symbol s) const {
  return m_pcAlphabet->GetColour(s);
}

#endif /* #ifndef __DasherModel_h__ */
