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
  ///Displays a message to the user - somehow. Two styles
  /// of message are supported: (1) modal messages, i.e. which interrupt text entry;
  /// these should be explicitly dismissed (somehow) before text entry resumes; and
  /// (2) non-modal or asynchronous messages, which should be displayed in the background
  /// but allow the user to continue text entry as normal.
  /// NOTE for subclasses: it is best not to popup any modal window here but rather to
  /// store all messages until the next frame is rendered and then combine them into one.
  /// \param strText text of message to display.
  /// \param bInterrupt if true, text entry should be interrupted; if false, user should
  /// be able to continue writing uninterrupted.
  virtual void Message(const std::string &strText, bool bInterrupt)=0;
  
  ///Utility method for common case of displaying a modal message with a format
  /// string containing a single %s.
  void FormatMessageWithString(const char* fmt, const char* str);

  ///Utility method for less-but-still-quite-common case of displaying a modal
  /// message with a format string containing two %s
  void FormatMessageWith2Strings(const char* fmt, const char* str1, const char* str2);

};

/// @}

#endif
