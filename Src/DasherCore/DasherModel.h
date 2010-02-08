// DasherModel.h
//
// Copyright (c) 2008 The Dasher Team
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

#ifdef HAVE_CONFIG_H
#include <config.h>
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
#include "ExpansionPolicy.h"

namespace Dasher {
  class CDasherModel;
  class CDasherInterfaceBase;
  class CDasherView;
  
  struct SLockData;
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
class Dasher::CDasherModel:public CFrameRate, private NoClones
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
  /// Update the root location with *one step* towards the specified
  /// co-ordinates - used by timer callbacks (for non-button modes)
  void OneStepTowards(myint, myint, unsigned long iTime, Dasher::VECTOR_SYMBOL_PROB* pAdded = NULL, int* pNumDeleted = NULL);  

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
  /// Make the 'target' root coordinates match those currently visible, so any
  /// Offset(int) currently in progress (i.e. being smoothed over several
  /// frames) stops (at whatever point it's currently reached). Appropriate for
  /// abrupt changes in behaviour (such as backing off in  button modes)
  void AbortOffset();

  /// @}

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
  /// Render the model to a given view. Return if any nodes were
  /// expanded, as if so we may want to render *another* frame to
  /// perform further expansion.
  ///

  bool RenderToView(CDasherView *pView, CExpansionPolicy &policy);

  /// @}

  /// 
  /// @name Scheduled operation
  /// E.g. response to button mode
  /// @{

  ///
  /// Schedule zoom to a given Dasher coordinate (used in click mode,
  /// button mode etc.)
  ///

  void ScheduleZoom(dasherint iDasherX, dasherint iDasherY, int iMaxZoom = 0);

  ///
  /// Update the bounds of the root node for the next step in any
  /// still-in-progress zoom scheduled by ScheduleZoom (does nothing
  /// if no steps remaining / no zoom scheduled).
  ///
  bool NextScheduledStep(unsigned long iTime, Dasher::VECTOR_SYMBOL_PROB* pAdded = NULL, int *pNumDeleted = NULL);

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
  /// Check whether a change of root node is needed, and perform the
  /// update if so
  /// TODO: Could be done in UpdateBounds?
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
  /// TODO: Figure out how all these "offset"s work / relate to each other - if they do! In particular,
  /// what do we need DasherModel's own m_iOffset (which measures in _bytes_, not unicode characters!) for?
  ///

  int GetOffset() {
    return m_pLastOutput->m_iOffset+1;
  };

  /// Create the children of a Dasher node
  void ExpandNode(CDasherNode * pNode); 
  
  void SetControlOffset(int iOffset);

 private:

  /// Common portion of OneStepTowards / NextScheduledStep, taking
  /// bounds for the root node in the next frame.
  void UpdateBounds(myint iNewMin, myint iNewMax, unsigned long iTime, Dasher::VECTOR_SYMBOL_PROB *pAdded, int *pNumDeleted);

  /// Struct representing intermediate stages in the goto queue
  ///
  struct SGotoItem {
    myint iN1;
    myint iN2;
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
  myint m_iDisplayOffset;

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


  // Time at which the model was started (ie last unpaused, used for gradual speed up)
  // TODO: Implementation is very hacky at the moment
  // TODO: Duplicates functionality previously implemented elsewhere...
  //   ...ACL 22/5/09 does it? There was an even hackier implementation, of resetting the
  //   framerate, used for control mode (ControlManager.cpp), but that's all I could find
  //   - and that seemed even worse, so I've removed it in favour of this here....?
  unsigned long m_iStartTime;
  
  // Offset into buffer of node currently under crosshair
  int m_iOffset;

  // Debug/performance information...

  // Information entered so far in this model
  double m_dTotalNats; 


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


  /// Should be public?
  void InitialiseAtOffset(int iOffset, CDasherView *pView);

  /// Called from InitialiseAtOffset
  void DeleteTree();

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
  /// Makes the node under the crosshair the root by deleting everything
  /// outside it, then rebuilds the nodes beneath it. (Thus, the node under
  /// the crosshair stays in the same place.) Used when control mode is turned
  /// on or off, or more generally, when the sizes of child nodes may have
  /// changed.
  ///

  void RebuildAroundCrosshair();

  ///
  /// Rebuild the parent of the current root - used during backing off
  ///

  void Reparent_root(int lower, int upper); 

  ///
  /// Return a pointer to the Dasher node which is currently under the
  /// crosshair. Used for output, and apparently needed for game mode.
  ///

  CDasherNode *Get_node_under_crosshair();    

  ///
  /// Output a node, which has not been seen (& first, any ancestors that haven't been seen either),
  /// but which _is_ a descendant of m_pLastOutput.
  ///
  
  void RecursiveOutput(CDasherNode *pNode, Dasher::VECTOR_SYMBOL_PROB* pAdded);
  
  ///
  /// Handle the output caused by a change in node over the crosshair. Specifically,
  /// deletes from m_pLastOutput back to closest ancestor of pNewNode,
  /// then outputs from that ancestor to the node now under the crosshair (inclusively)
  ///

  void HandleOutput(Dasher::VECTOR_SYMBOL_PROB* pAdded, int* pNumDeleted);


  ///
  /// Clear the queue of old roots - used when those nodes become
  /// invalid, eg during changes to conrol mode
  ///

  void ClearRootQueue();

};
/// @}

#endif /* #ifndef __DasherModel_h__ */
