
// Simple millisecond accurate timer.
//
// Copyright 2004 by Keith Vertanen

#ifndef __SIMPLE_TIMER_H__
#define __SIMPLE_TIMER_H__

#include <sys/timeb.h>
#include <time.h>

class CSimpleTimer
{
public:
    CSimpleTimer();
    ~CSimpleTimer();

    double GetElapsed();

private:
    int     m_startSecond;
    int     m_startMs;

};

#endif

