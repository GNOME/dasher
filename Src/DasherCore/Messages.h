// Messages.h
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

#ifndef __MESSAGES_H__
#define __MESSAGES_H__

///Abstract superclass = interface for displaying messages to the user.
///Each platform must implement: see CDasherInterfaceBase, CDashIntfScreenMsgs

#include <string>

/// \ingroup Core
/// @{

class CMessageDisplay {
public:
  ///Displays a message to the user - somehow. This should not interrupt the user:
  /// if Dasher is in motion, it should continue just as if no message were displayed.
  /// \param strText text of message to display. TODO, do we want an enum for seriousness?
  virtual void Message(const std::string &strText)=0;
};

/// @}

#endif
