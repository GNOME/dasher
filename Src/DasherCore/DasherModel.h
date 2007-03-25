// DasherModel.h
//
// Copyright (c) 2007 The Dasher Team
//
// This file is part of Dasher.
//
// Dasher is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Dasher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Dasher; if not, write to the Free Software 
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef __DasherModel_h__
#define __DasherModel_h__

#ifndef WIN32
#include "config.h"
#endif

#include <deque>
#include <cmath>
#include <vector>

#include "../Common/NoClones.h"
#include "DasherComponent.h"
#include "DasherNode.h"
#include "DasherTypes.h"
#include "FrameRate.h"
#include "NodeCreationManager.h"

namespace Dasher {
  class CDasherModel;
  class CDasherInterfaceBase;
  class CDasherView;
}

/// \defgroup Model The Dasher model
/// @{

/// \brief Dasher 'world' data structures and dynamics.
///
/// The DasherModel represents the current state of Dasher
/// It contains a tree of DasherNodes
///             knows the current viewpoint
///             knows how to evolve the viewpoint
///
class Dasher::CDasherModel:public Dasher::CDasherComponent, private NoClones
{
 public:

  CDasherModel(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CNodeCreationManager *pNCManager, CDasherInterfaceBase *pDashIface, CDasherView *pView, int iOffset, bool bGameMode = false, const std::string &strGameModeText = "");
  ~CDasherModel();

  ///
  /// Event handler
  ///

  void HandleEvent(Dasher::CEvent * pEvent);

  /// @name Dymanic evolution
  /// Routines detailing the timer dependent evolution of the model
  /// @{

  ///
  /// Update the root location - called in response to regular timer
  /// callbacks
  ///
  bool UpdatePosition(myint, myint, unsigned long iTime, Dasher::VECTOR_SYMBOL_PROB* pAdded = NULL, int* pNumDeleted = NULL);  

  ///
  /// Notify the framerate class that a new frame has occurred
  ///
  void NewFrame(unsigned long Time);


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

  /// @}

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
  /// @name Rendering
  /// Methods to do with rendering the model to a view
  /// @{

  /// 
  /// Render the model to a given view
  ///

  bool RenderToView(CDasherView *pView, bool bRedrawDisplay);

  /// @}

  /// 
  /// @name Scheduled operation
  /// E.g. response to button mode
  /// @{

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

  /// @}

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

  ///
  /// Notify of a change of cursor position within the attached
  /// buffer. Resulting action should be appropriate - ie don't
  /// completely rebuild the model if an existing node covers this
  /// point
  ///

  void SetOffset(int iLocation, CDasherView *pView);

  ///
  /// TODO: Implement this
  ///

  int GetOffset() {
    return 0;
  };

  void SetControlOffset(int iOffset);
 protected:
  int m_iRenderCount;


 private:

  CDasherInterfaceBase * m_pDasherInterface;

  /////////////////////////////////////////////////////////////////////////////

  // Interfaces

  //  CLanguageModel *m_pLanguageModel;     // pointer to the language model

  CLanguageModel::Context LearnContext; // Used to add data to model as it is entered

  /////////////////////////////////////////////////////////////////////////////

  CDasherNode *m_Root;
  CDasherNode *m_pLastOutput;

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

  ///
  /// CDasherModel::Get_new_root_coords( myint Mousex,myint Mousey )
  /// 
  /// Calculate the new co-ordinates for the root node after a single
  /// update step. For further information, see Doc/geometry.tex.
  /// 
  /// \param mousex x mouse co-ordinate measured right to left.
  /// \param mousey y mouse co-ordinate measured top to bottom.
  /// \param iNewMin New root min
  /// \param iNewMax New root max
  /// \param iTime Current timestamp
  ///
  void Get_new_root_coords(myint mousex, myint mousey, myint &iNewMin, myint &iNewMax, unsigned long iTime);

  double CorrectionFactor(int dasherx, int dashery);

  void DeleteTree();

  void InitialiseAtOffset(int iOffset, CDasherView *pView);

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
  /// Make a child of the root into a new root
  ///

  void Make_root(CDasherNode *pNewRoot); 

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

  bool RecursiveCheckRoot(CDasherNode *pNode, CDasherNode **pNewNode, bool &bFound);

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

  int m_iOffset;

};
/// @}

#endif /* #ifndef __DasherModel_h__ */
