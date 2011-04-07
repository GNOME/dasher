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

#include "DasherInterfaceBase.h"

namespace Dasher {
class CDashIntfScreenMsgs : public CDasherInterfaceBase {
public:
  /// Stores the message for Redraw to render onto the Screen on top of the view
  virtual void Message(const std::string &strText);
  
  /// Override to render (on top of nodes+decorations) any messages, for
  /// LP_MESSAGE_TIME ms, before removing from queue.
  bool FinishRender(unsigned long ulTime);

  ///Override to re-MakeLabel any messages.
  void ChangeScreen(CDasherScreen *pNewScreen);

private:
  ///Messages to be displayed to the user, longest-ago at the front,
  /// along with the timestamp of the frame at which each was first displayed
  /// to the user, or 0 if not yet displayed.
  std::deque<pair<CDasherScreen::Label*, unsigned long> > m_dqMessages;
  
};

}
#endif
