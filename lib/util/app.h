// Copyright (c) 2011 by WexWorks, LLC -- All Rights Reserved

#ifndef APP_H_
#define APP_H_

#if WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1
#define NOMINMAX
#include <windows.h>
#endif
typedef HDC AppDisplay;
typedef HWND AppWindow;
#endif

#if LINUX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
typedef Display *AppDisplay;
typedef Window AppWindow;
#endif // LINUX


namespace ww {

class App {
public:
  App();
  virtual ~App();
  virtual bool Init(int width, int height);
  virtual bool Run();

  virtual bool Resume() { return true; }
  virtual bool Repaint() { return true; }
  virtual bool Step(float dt) { return true; }
  virtual void Touch(int count, float *xy) {}
  virtual bool Pause() { return true; }
  virtual bool Quit() { done_ = true; return true; }

protected:
  App(const App&);              // disallow copy ctor
  void operator=(const App&);   // disallow assignment
  bool done_;                   // true when we are quitting
#if WINDOWS || LINUX
  AppDisplay native_display_;   // XDisplay, HDC, ...
  AppWindow native_window_;     // HWND, XWindow, ...
#endif
}; // class App


} // namespace ww

#endif  // APP_H_
