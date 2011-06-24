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
/// It does not care what the nodes in the tree are or mean, tho the model does handle
/// calling CDasherNode::Output() / Undo() on nodes falling under/leaving the crosshair
/// (However, determining which nodes are under the crosshair, is done by the CDasherView).
///
/// The class is Observable in that it broadcasts a pointer to a CDasherNode when the node's
/// children are created.
class Dasher::CDasherModel:public Dasher::CFrameRate, public Observable<CDasherNode*>, private NoClones
{
 public:
  static const unsigned int NORMALIZATION = 1<<16;
  static const myint ORIGIN_X=2048, ORIGIN_Y=2048, MAX_Y=4096;

  /// Constructs a new CDasherModel. Note, must be followed by a call to
  /// SetOffset() before the model can be used.
  CDasherModel(CSettingsUser *pCreateFrom, CDasherInterfaceBase *pDashIface);
  ~CDasherModel();

  ///
  /// Event handler
  ///

  void HandleEvent(int iParameter);

  /// @name Dymanic evolution
  /// Routines detailing the timer dependent evolution of the model
  /// @{

  ///
  /// Update the root location with *one step* towards the specified
  /// co-ordinates - used by timer callbacks (for non-button modes)
  void OneStepTowards(myint, myint, unsigned long iTime);

  ///
  /// Notify the framerate class that a new frame has occurred
  /// Called from CDasherInterfaceBase::NewFrame
  ///
  void RecordFrame(unsigned long Time);

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
  /// Render the model to a given view, and cause output to happen.
  /// Note, enqueues nodes onto the Expansion Policy, but does not apply it.
  ///
  void RenderToView(CDasherView *pView, CExpansionPolicy &policy);

  /// @}

  ///
  /// @name Scheduled operation
  /// E.g. response to button mode
  /// @{

  ///
  /// Schedule a zoom such that the given range of Dasher coordinates
  /// will fill the Y-axis. (used in click mode, button mode etc.)
  /// Note that safety margin, max-zoom, etc., as desired, are the responsibility
  /// of the caller; this method requires only that y2 > y1.
  /// \param y1 Minimum Y-coordinate (will be moved to dasher-y of 0)
  /// \param y2 Maximum Y-coordinate (will be moved to dasher-y of 4096)
  ///
  void ScheduleZoom(long time, dasherint y1, dasherint y2);

  void ClearScheduledSteps();

  ///
  /// Update the bounds of the root node for the next step in any
  /// still-in-progress zoom scheduled by ScheduleZoom (does nothing
  /// if no steps remaining / no zoom scheduled).
  ///
  bool NextScheduledStep(unsigned long iTime);

  /// @}

  ///
  /// This is pretty horrible - a rethink of the start/reset mechanism
  /// is definitely in order. Used to prevent the root node from being
  /// too large in various modes before Dasher is started.
  ///

  void LimitRoot(int iMaxWidth);

  ///
  /// Rebuild the tree of nodes (may reuse the existing ones if !bForce).
  /// @param iLocation offset (cursor position) in attached buffer from which to obtain context
  /// @param pMgr Manager to use to create nodes
  /// @param bForce if true, model should be completely rebuilt (even for
  /// same offset) - characters at old offsets may have changed, or we have
  /// a new AlphabetManager. If false, assume buffer and alphabet unchanged,
  /// so no need to rebuild the model if an existing node covers this point.
  ///

  void SetOffset(int iLocation, CAlphabetManager *pMgr, CDasherView *pView, bool bForce);

  ///
  /// The current offset of the cursor/insertion point in the text buffer
  /// - measured in (unicode) characters, _not_ octets.
  ///

  int GetOffset();

  /// Create the children of a Dasher node
  void ExpandNode(CDasherNode * pNode);

 private:

  /// Common portion of OneStepTowards / NextScheduledStep, taking
  /// bounds for the root node in the next frame.
  void UpdateBounds(myint iNewMin, myint iNewMax, unsigned long iTime);

  /// Struct representing intermediate stages in the goto queue
  ///
  struct SGotoItem {
    myint iN1;
    myint iN2;
  };

  // Pointers to various auxilliary objects
  CDasherInterfaceBase *m_pDasherInterface;

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

  // Whether characters entered by alphabet manager are expected to
  // require conversion.
  // TODO: Need to rethink this at some point.
  bool m_bRequireConversion;
  
  /**
   * The string a user must type if game mode is active.
   */
  std::string m_strGameTarget;

  // Model status...


  // Time at which the model was started (ie last unpaused, used for gradual speed up)
  // TODO: Implementation is very hacky at the moment
  // TODO: Duplicates functionality previously implemented elsewhere...
  //   ...ACL 22/5/09 does it? There was an even hackier implementation, of resetting the
  //   framerate, used for control mode (ControlManager.cpp), but that's all I could find
  //   - and that seemed even worse, so I've removed it in favour of this here....?
  unsigned long m_iStartTime;

  // Debug/performance information...

  // Information entered so far in this model
  double m_dTotalNats;

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

  ///
  /// Make a child of the root into a new root
  ///

  void Make_root(CDasherNode *pNewRoot);

  ///
  /// Make the parent of the current root into the new root (rebuilding if necessary) - used during backing off
  /// Return true if successful, false if couldn't.
  ///
  bool Reparent_root();

  /// Handle the output caused by a change in node over the crosshair. Specifically,
  /// deletes from m_pLastOutput back to closest ancestor of pNewNode,
  /// then outputs from that ancestor to that node
  /// @param pNewNode innermost node now covering the crosshair
  void OutputTo(CDasherNode *pNewNode);


  ///
  /// Clear the queue of old roots - used when those nodes become
  /// invalid, eg during changes to conrol mode
  ///

  void ClearRootQueue();

};
/// @}

#endif /* #ifndef __DasherModel_h__ */
