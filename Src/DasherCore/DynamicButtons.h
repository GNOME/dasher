// DynamicFilter.h
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

#ifndef __DynamicButtons_h__
#define __DynamicButtons_h__

#include "DynamicFilter.h"

/// \ingroup InputFilter
/// @{
namespace Dasher {
///filter with three states: paused, reversing, running. Button 1 is dedicated reverse
/// button (subclasses may also call reverse());  when reversing, any key pauses,
/// then any key restarts.
class CDynamicButtons : public CDynamicFilter {
 public:
  CDynamicButtons(CSettingsUser *pCreator, CDasherInterfaceBase *pInterface, CFrameRate *pFramerate, ModuleID_t iID, const char *szName);

  ///when reversing, backs off; when paused, does nothing; when running, delegates to TimerImpl
  virtual void Timer(unsigned long Time, CDasherView *pView, CDasherInput *pInput, CDasherModel *m_pDasherModel, CExpansionPolicy **pol);

  virtual void KeyDown(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);
  virtual void KeyUp(unsigned long iTime, int iId, CDasherView *pView, CDasherInput *pInput, CDasherModel *pModel);

 protected:
  ///Called when a key event is detected - could be a single press (a la KeyDown/KeyUp),
  /// but is also called with explicit indication of "long" or other types of press,
  /// avoiding need for subclasses to try to detect these manually. In the default implementation:
  /// if paused, any key restarts; if reversing, any key pauses; if running, short presses
  /// of button 0 or 100 act as a dedicated reverse button, and button 1 pauses; any other
  /// press type or button is passed onto ActionButton.
  /// \param iType 0=normal press, 1=long press; see also CButtonMultiPress.
  virtual void ButtonEvent(unsigned long iTime, int iButton, int iType, CDasherModel *pModel);

  ///Called to handle key events when the Filter is running forwards normally.
  /// Short presses of buttons 0, 100 and 1 have been handled already, but all
  /// other buttons/press-types will be passed here.
  /// \param iType 0=normal press, 1=long press; see also CButtonMultiPress.
  virtual void ActionButton(unsigned long iTime, int iButton, int iType, CDasherModel *pModel) = 0;

  ///Whether a key (any that we might respond to) is held down.
  /// If so, m_iHeldId identifies the key in question. We need this
  /// not just for detecting long-presses etc. (in subclasses), and
  /// ignoring presses of other keys while the first is down, but also
  /// simply to filter out key-repeat events (=multiple keydown without a keyup)
  bool m_bKeyDown;

  ///if m_bKeyDown is true, identifies the key that was first pressed
  /// that is currently still held down.
  int m_iHeldId;

  bool m_bDecorationChanged;
  bool isReversing() {return !isPaused() && !m_bForwards;}
  bool isRunning() {return !isPaused() && m_bForwards;}
  virtual void pause();
  virtual void reverse(unsigned long iTime);
  virtual void run(unsigned long iTime);

  virtual void TimerImpl(unsigned long Time, CDasherView *m_pDasherView, CDasherModel *m_pDasherModel, CExpansionPolicy **pol) = 0;

  ///Subclasses should all this (rather than pModel->Offset()) to offset the model
  /// (it also stores the model, to abort the offset upon pause if necessary)
  void ApplyOffset(CDasherModel *pModel, int iOffset);

private:
  bool m_bForwards;
  unsigned long m_uSpeedControlTime;
  CDasherModel *m_pModel;
};
}
#endif
