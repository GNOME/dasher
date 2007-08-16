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

#include <climits>
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

  CDasherModel(CEventHandler * pEventHandler, CSettingsStore * pSettingsStore, CNodeCreationManager *pNCManager, CDasherInterfaceBase *pDashIface, CDasherView *pView, int iOffset);
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
  /// callbacks, mainly from the tier events of various button
  /// handlers
  /// TODO: Make this a bit more central in the button handler hierarchy
  ///
  bool UpdatePosition(myint, myint, unsigned long iTime, Dasher::VECTOR_SYMBOL_PROB* pAdded = NULL, int* pNumDeleted = NULL);  

  ///
  /// Notify the framerate class that a new frame has occurred
  /// Called from CDasherInterfaceBase::NewFrame
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
  /// TODO: Need to check semantics here
  /// Called from CDasherInterfaceBase::UnPause;
  ///

  void Reset_framerate(unsigned long Time) {
    m_fr.Reset(Time);
  }

  ///
  /// Initialise the framerate class - presumably called whenever
  /// dasher is stoppe, but the actual semantics here need to be
  /// verified
  /// Called from CDasherInterfacebase::Halt 
  ///

  void Halt() {
    m_fr.Initialise();
  }

  ///
  /// Set the target bitrate - probably shouldn't be called externally
  /// - could implement through the event subsystem instead
  ///
  
/*   void SetBitrate(double TargetRate) { */
/*     m_fr.SetBitrate(TargetRate); */
/*   }  */

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
  /// TODO: Could be done in UpdatePosition?
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

 private:

  /// Struct representing intermediate stages in the goto queue
  ///
  struct SGotoItem {
    myint iN1;
    myint iN2;
    int iStyle;
  };
  
  // Pointers to various auxilliary objects
  CDasherInterfaceBase *m_pDasherInterface;
  CNodeCreationManager *m_pNodeCreationManager;

  // The root of the Dasher tree
  CDasherNode *m_Root;

  // Old root notes
  // TODO: This should probably be rethought at some point - it doesn't really make a lot of sense
  std::deque < CDasherNode * >oldroots;

  // Rootmin and Rootmax specify the position of the root node in Dasher coords
  myint m_Rootmin;
  myint m_Rootmax;

  // Permitted range for root node - model cannot zoom beyond this
  // point without falling back to a new root node.
  myint m_Rootmin_min;
  myint m_Rootmax_max;

  // TODO: Does this need to be brought back? Make it relative to visible region?
  // The active interval over which Dasher nodes are maintained - this is most likely bigger than (0,DasherY)
  // CRange m_Active;

  // Offset used when presenting the model to the user, specified as
  // Displayed rootmin/max - actual rootmin/rootmax
  myint m_iTargetOffset; 

  CDasherNode *m_pLastOutput;

  // Queue of goto locations (eg for button mode)
  std::deque<SGotoItem> m_deGotoQueue;

  /// TODO: Not sure what this actually does
  double m_dAddProb;

  // Model parameters... (cached from settings store)

  // Current maximum bitrate (ie zoom at far rhs).
  double m_dMaxRate;

  // Whether game mode is active
  // TODO: This isn't very functional at the moment
  bool m_bGameMode;

  // Whether characters entered by alphabet manager are expected to
  // require conversion.
  // TODO: Need to rethink this at some point.
  bool m_bRequireConversion;

  // Model status...

  // Helper class to estimate frame rate
  CFrameRate m_fr;  

  // Time at which the model was started (ie last unpaused, used for gradual speed up)
  // TODO: Implementation is very hacky at the moment
  // TODO: Duplicates functionality previously implemented elsewhere
  unsigned long m_iStartTime;
  
  // Offset into buffer of node currently under crosshair
  int m_iOffset;

  // Debug/performance information...

  // Information entered so far in this model
  double m_dTotalNats; 

  // Number of nodes rendered
  int m_iRenderCount;



  CDasherNode *Get_node_under_mouse(myint smousex, myint smousey);

  ///
  /// Go directly to a given coordinate - check semantics
  ///
  void NewGoTo(myint n1, myint n2, Dasher::VECTOR_SYMBOL_PROB* pAdded, int* pNumDeleted);

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


  /// Called from ScheduleZoom
  double CorrectionFactor(int dasherx, int dashery);

  /// Should be public?
  void InitialiseAtOffset(int iOffset, CDasherView *pView);

  /// Called from InitialiseAtOffset
  void DeleteTree();

  /// Push nodes recursively (d'oh!) - seems to be a little broken at the moment, need to check where this gets called
  /// TODO: Need to sort out, need to check that this isn't generating too many nodes
  void Recursive_Push_Node(CDasherNode * pNode, int depth);

  /// Create the children of a Dasher node
  void Push_Node(CDasherNode * pNode); 


  ///
  /// Perform output on a node - recurse up the tree outputting any
  /// symbols which have not been output so far. Neeed to check
  /// behaviour with respect to deletion
  ///

  void RecursiveOutput(CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded);


  ///
  /// Check semantics here
  ///

  void OutputCharacters(CDasherNode * node);
  bool DeleteCharacters(CDasherNode * newnode, CDasherNode * oldnode, int* pNumDeleted = NULL);

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


  ///
  /// Clear the queue of old roots - used when those nodes become
  /// invalid, eg during changes to conrol mode
  ///

  void ClearRootQueue();

};
/// @}

#endif /* #ifndef __DasherModel_h__ */
