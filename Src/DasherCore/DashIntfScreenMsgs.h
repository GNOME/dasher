// DashIntfScreenMsgs.h
//
// Created 2011 by Alan Lawrence
// Copyright (c) 2011 The Dasher Team
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

#ifndef __DASH_INTF_SCREEN_MSGS_H__
#define __DASH_INTF_SCREEN_MSGS_H__

#include "DashIntfSettings.h"

namespace Dasher {
///Implements the MessageDisplay part of CDasherInterfaceBase by rendering messages
/// directly onto the CDasherScreen (using MakeLabel wrapping to LP_MESSAGE_FONTSIZE).
///Also implements CreateGameModule to return a ScreenGameModule, which provides
/// text prompts in much the same fashion.
/// Note we subclass CDashIntfSettings as currently all platforms want to inherit from
/// the latter; if this changes, we could declare ScreenMsgs & Settings separately and
/// combine via multiple inheritance?? (from CSettingsUser, not DashIntfBase!)
class CDashIntfScreenMsgs : public CDashIntfSettings {
public:

  CDashIntfScreenMsgs(CSettingsStore *pSettingsStore, CFileUtils* fileUtils);

  /// Stores messages for Redraw to render onto the Screen on top of the view.
  /// For modal messages (bInterrupt=true), pauses Dasher, and keeps the message
  /// onscreen until the user starts moving again (via normal mechanisms);
  /// For non-modal or asynchronous messages (bInterrupt=false), we render
  /// the message over the canvas for LP_MESSAGE_TIME milliseconds without pausing.
  /// (This method merely stores the messages into m_dqAsyncMessages or m_dqModalMessages
  /// as appropriate; display, timeout, etc. is handled in Redraw.)
  /// \param strText text of message to display.
  /// \param bInterrupt whether to interrupt any text entry in progress.
  virtual void Message(const std::string &strText, bool bInterrupt);
  
  /// Override to render (on top of nodes+decorations) any messages, for
  /// LP_MESSAGE_TIME ms, before removing from queue.
  bool FinishRender(unsigned long ulTime);

  ///Override to re-MakeLabel any messages.
  void ChangeScreen(CDasherScreen *pNewScreen);
  
  ///Flush any modal messages that have been displayed before resuming.
  void onUnpause(unsigned long lTime);
  
  ///Implement to return a ScreenGameModule, i.e. rendering text prompts
  /// onto the Screen with Labels, much as we do for messages!
  CGameModule *CreateGameModule();
private:
  /// Asynchronous (non-modal) messages to be displayed to the user, longest-ago
  /// at the front, along with the timestamp of the frame at which each was first
  /// displayed to the user - 0 if not yet displayed.
  std::deque<pair<CDasherScreen::Label*, unsigned long> > m_dqAsyncMessages;
  
  /// Modal messages being or waiting to be displayed to the user, longest-ago
  /// at the front, along with the timestamp when each was first displayed to the
  /// user (0 if not yet displayed).
  std::deque<pair<CDasherScreen::Label*, unsigned long> > m_dqModalMessages;  
};

}
#endif
