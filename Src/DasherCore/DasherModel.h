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
#include "ExpansionPolicy.h"
#include "SettingsStore.h"
#include "AlphabetManager.h"

namespace Dasher {
  class CDasherModel;
  class CDasherView;
}

/// \defgroup Model The Dasher model
/// @{

/// \brief Dasher 'world' data structures and dynamics.
///
/// The DasherModel implements arithmetic coding for Dasher.
/// It contains a tree of DasherNodes and the current viewpoint, 
/// and evolves the tree by expanding leaves (somewhat in response to DasherView) and
/// (eventually) deleting ancestors/parents.

///
/// DasherModel does not care what the nodes in the tree are or mean, tho it does handle
/// calling CDasherNode::Output() / Undo() on nodes falling under/leaving the crosshair
/// (However, determining which nodes are under the crosshair, is done by the CDasherView).
///
/// The class is Observable in that it broadcasts a pointer to a CDasherNode when the node's
/// children are created.
class Dasher::CDasherModel: public Observable<CDasherNode*>, private NoClones
{
 public:
  static const unsigned int NORMALIZATION = 1<<16;
  static const myint ORIGIN_X=2048, ORIGIN_Y=2048, MAX_Y=4096;

  /// Constructs a new CDasherModel. Note, must be followed by a call to
  /// SetNode() before the model can be used.
  CDasherModel();
  ~CDasherModel();

  /// @name Offset routines
  /// For "bouncing" the display up and down (dynamic button modes)
  /// @{


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
  /// @name Dynamics
  /// For controlling movement of the model
  /// @{

  ///
  /// Schedule a zoom over many frames, such that after the last frame,
  /// the given  range of Dasher coordinates (in the current view)
  /// will fill the Y-axis. (Used by click mode, button mode etc.)
  /// Note any safety margin, max-zoom, etc., are the responsibility
  /// of the caller; this method requires only that y2 > y1.
  /// \param y1,y2 - target range of y axis, i.e. to move to 0,MAXY
  /// \param nSteps number of steps to schedule to take us all the way there
  void ScheduleZoom(dasherint y1, dasherint y2, int nSteps);
  
  /// Schedule one frame of movement, with the property that
  /// <nsteps> calls with the same parameter, should bring
  /// the given range of Dasher Y-space to fill the axis.
  /// (Roughly - we use approximations! - but more accurate
  /// than the first step of a zoom).
  /// \param y1,y2 - target range of y axis, i.e. to move to 0,MAXY
  /// \param nSteps number of steps that would take us all the way there
  /// \param limX X coord at which max speed achieved (any X coord lower than
  /// this, will be slowed down to that speed).
  /// \param bExact whether to do "exact" calculations (slower, using floating-point
  /// pow), or approximate with integers (will move at not-ideal rate in some directions)
  void ScheduleOneStep(dasherint y1, dasherint y2, int nSteps, int limX, bool bExact);

  ///Cancel any steps previously scheduled (most likely by ScheduleZoom)
  void ClearScheduledSteps();

  ///
  /// Called by DasherInterfaceBase to update the bounds of the root node for
  /// the next step that has been scheduled (whether a multi-step zoom or a
  /// single step from ScheduleOneStep).
  /// \return True if this moves the model (by applying a previously-scheduled
  /// step); false if there were no scheduled steps (=> the model hasn't moved).
  ///
  bool NextScheduledStep();

  /// @}

  /// Returns the node that was under the crosshair in the
  /// last frame that was rendered. (I.e., this is the last
  /// node output.)
  CDasherNode *Get_node_under_crosshair();
  
  ///
  /// This is pretty horrible - a rethink of the start/reset mechanism
  /// is definitely in order. Used to prevent the root node from being
  /// too large in various modes before Dasher is started.
  ///

  void LimitRoot(int iMaxWidth);

  ///
  /// Rebuild the tree of nodes from a given root
  ///

  void SetNode(CDasherNode *pNewRoot);

  ///
  /// The current offset of the cursor/insertion point in the text buffer
  /// - measured in (unicode) characters, _not_ octets.
  ///

  int GetOffset();

  /// Create the children of a Dasher node
  void ExpandNode(CDasherNode * pNode);

 private:

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

  // Queue of steps scheduled, represented as pairs
  // of min/max coordinates for root node
  std::deque<std::pair<myint,myint> > m_deGotoQueue;

  /// TODO: Not sure what this actually does
  double m_dAddProb;

  // Model parameters... (cached from settings store)

  // Whether characters entered by alphabet manager are expected to
  // require conversion.
  // TODO: Need to rethink this at some point.
  bool m_bRequireConversion;

  // Information entered so far in this model
  double m_dTotalNats;

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
