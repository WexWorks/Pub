// Copyright (c) 2011 by WexWorks, LLC -- All Rights Reserved

#ifndef TIMER_H_
#define TIMER_H_

#ifdef WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1
#define NOMINMAX
#include "windows.h"
#endif
typedef LARGE_INTEGER TimerVal;
#endif

#if defined LINUX || defined ANDROID
#include <sys/time.h>
typedef struct timeval TimerVal;
#endif

namespace ww {

class Timer {
public:
  Timer();
  ~Timer() {}

  double Elapsed() const;
  void Restart();

  static const char *String(double seconds);
  const char *String() const {
    return String(Elapsed());
  }

private:
  TimerVal start_time_;
};

} // namespace ww

#endif   // TIMER_H_
