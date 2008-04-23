#include "../Common/Common.h"

#include "Timer.h"
#include "DasherControl.h"

#ifndef DASHER_WIN32
#include <sys/time.h>
#endif

// FIXME - figure out what calls this
gint timer_callback(gpointer data) {
  return static_cast < CDasherControl * >(data)->TimerEvent();
}

gint long_timer_callback(gpointer data) {
  return static_cast < CDasherControl * >(data)->LongTimerEvent();
}

long get_time() {
#ifdef DASHER_WIN32
  return 0;
#else
  // We need to provide a monotonic time source that ticks every millisecond
  long s_now;
  long ms_now;

  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  s_now = tv.tv_sec;
  ms_now = tv.tv_usec / 1000;

  return (s_now * 1000 + ms_now);
#endif
}
