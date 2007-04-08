#include "../Common/Common.h"

#include "Timer.h"
#include "DasherControl.h"

#include <sys/time.h>

gint timer_callback(gpointer data) {
  return static_cast < CDasherControl * >(data)->TimerEvent();
}

gint long_timer_callback(gpointer data) {
  return static_cast < CDasherControl * >(data)->LongTimerEvent();
}

long get_time() {
  // We need to provide a monotonic time source that ticks every millisecond
  long s_now;
  long ms_now;

  struct timeval tv;
  struct timezone tz;

  gettimeofday(&tv, &tz);

  s_now = tv.tv_sec;
  ms_now = tv.tv_usec / 1000;

  return (s_now * 1000 + ms_now);
}
