
// Object that keeps track of a time span.  
// Span starts when the object is created, and
// ends when someone asks it for its XML representation.
// User can also call stop to cause timer to stop and
// call for XML representation later.
//
// Copyright 2005 by Keith Vertanen

#ifndef __TIME_SPAN_H__
#define __TIME_SPAN_H__

#include "FileLogger.h"
#include <string>
#include "SimpleTimer.h"
#include <sys/timeb.h>
#include <time.h>
#include <vector>

#ifdef USER_LOG_TOOL
#include "XMLUtil.h"
#endif

using namespace std;

extern CFileLogger* g_pLogger;

class CTimeSpan;

typedef vector<CTimeSpan>    VECTOR_TIME_SPAN;
typedef vector<CTimeSpan*>   VECTOR_TIME_SPAN_PTR;

class CTimeSpan
{
public:
  CTimeSpan(const string& strName, bool bAddDate);

#ifdef USER_LOG_TOOL
  CTimeSpan(const string& strName, const string& strXML);
#endif

  ~CTimeSpan();

  void                Stop();
  string              GetXML(const string& strPrefix = "", bool bSinglePointInTime = false);

  void                Continue();
  bool                IsStopped();
  double              GetElapsed();

  static string       GetTimeStamp();
  static string       GetDateStamp();

private:
  string              m_strName;
  string              m_strStartTime;
  string              m_strEndTime;
  double              m_dElapsed;
  CSimpleTimer*       m_pTimer;
  string              m_strStartDate;

  void                InitMemberVars();

};

#endif
