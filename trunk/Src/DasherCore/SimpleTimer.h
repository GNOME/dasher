
// Simple millisecond accurate timer.
//
// Copyright 2004 by Keith Vertanen

#ifndef __SIMPLE_TIMER_H__
#define __SIMPLE_TIMER_H__

#include <time.h>

/// \ingroup Logging
/// \{
class CSimpleTimer
{
public:
  CSimpleTimer();
  ~CSimpleTimer();

  double GetElapsed();

private:
  int     m_iStartSecond;
  int     m_iStartMs;

};
/// \}

#endif

