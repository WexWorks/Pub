// Copyright (c) 2011 by WexWorks, LLC -- All Rights Reserved

#include "util/eglu.h"
#include "util/debug.h"
#include <stdlib.h>

namespace ww {
namespace eglu {

EGLNativeDisplayType GetNativeDisplay() {
#ifdef WINDOWS
  return GetDC(NULL);
#endif
#ifdef LINUX
  return XOpenDisplay(NULL);
#endif
  return NULL;
}

bool InitDisplay(EGLNativeDisplayType *native_display, EGLDisplay *display,
                 EGLConfig *config) {
  if (!*native_display) {
   *native_display = GetNativeDisplay();
  }
  *display = eglGetDisplay(*native_display);
  if (*display == EGL_NO_DISPLAY)
    *display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (*display == EGL_NO_DISPLAY) {
    Error("Cannot get EGL display.\n");
    return false;
  }

  EGLint major = -1, minor = -1;
  if (!eglInitialize(*display, &major, &minor)) {
    Error("Cannot initialize EGL display.\n");
    return false;
  }

  EGLint max_config = 0;
  if (!eglGetConfigs(*display, NULL, 0, &max_config)) {
    Error("Cannot get EGL configuration count.\n");
    return false;
  }

  EGLConfig *config_list = (EGLConfig *) malloc(max_config * sizeof(EGLConfig));
  if (config_list == NULL) {
    Error("Cannot allocate %d EGL configurations.\n", max_config);
    return false;
  }

  EGLint config_count = 0;
  if (!eglGetConfigs(*display, config_list, max_config, &config_count)) {
    Error("Cannot get %d EGL configurations.\n", max_config);
    return false;
  }
  *config = config_list[0];

  return true;
}

#ifdef LINUX
Bool WaitForMap(Display *d, XEvent *e, char *win_ptr) {
  return (e->type == MapNotify && e->xmap.window == (*((Window*) win_ptr)));
}
#endif

EGLNativeWindowType CreateNativeWindow(const char *title, int width,
                                       int height, EGLDisplay display,
                                       EGLConfig config) {
#ifdef LINUX
  int vid = -1;
  if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &vid)) {
    Error("Cannot get config visual id attribute.\n");
    return NULL;
  }
  XVisualInfo xvi;
  xvi.visualid = vid;
  EGLNativeDisplayType native_display = GetNativeDisplay();
  int n;
  XVisualInfo *visual = XGetVisualInfo(native_display, VisualIDMask, &xvi, &n);
  if (!visual) {
    Error("Could not get X visual info.\n");
    return NULL;
  }
  long screen = DefaultScreen(native_display);
  Colormap colormap = XCreateColormap(native_display,
                                      RootWindow(native_display, screen),
                                      visual->visual, AllocNone);
  XSetWindowAttributes wa;
  wa.colormap = colormap;
  wa.background_pixel = 0xFFFFFFFF;
  wa.border_pixel = 0;
  wa.event_mask = StructureNotifyMask | ExposureMask;

  unsigned long mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

  Window window = XCreateWindow(native_display,
                                RootWindow(native_display, screen), 0, 0,
                                width, height, 0, visual->depth, InputOutput,
                                visual->visual, mask, &wa);

  XSizeHints sh;
  sh.flags = USPosition;
  sh.x = 10;
  sh.y = 10;

  XSetStandardProperties(native_display, window, title, title, None, 0, 0, &sh);
  XMapWindow(native_display, window);
  XEvent event;
  XIfEvent(native_display, &event, WaitForMap, (char*) &window);
  XSetWMColormapWindows(native_display, window, &window, 1);
  XFlush(native_display);

  // LEAK: colormap and visual are leaked!

  return window;
#endif  // LINUX

#ifdef WINDOWS
  RECT rect;
  SetRect(&rect, 0, 0, 1, 1);
  HMONITOR hMonitor;
  hMonitor = MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST);
  MONITORINFOEX mi;
  mi.cbSize = sizeof(mi);
  GetMonitorInfo(hMonitor, &mi);
  rect = mi.rcMonitor;
  int x = 0, y = 0;
  if (x < 0)
    x = rect.left;
  if (y < 0)
    y = rect.top;
  if (width < 0)
    width = rect.right - rect.left;
  if (height < 0)
    height = rect.bottom - rect.top;
  SetRect(&rect, x, y, x+width, y+height);

  // Register class
  WNDCLASSEX wcex;
  const char *className = "Win32WindowClass"; // must match app.cpp
  HINSTANCE hInstance = GetModuleHandle(NULL);
  if (!GetClassInfoEx(hInstance, className, &wcex))
    return NULL;

  // Create window
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
  width = rect.right - rect.left;     // adjust size for window borders
  height = rect.bottom - rect.top;
  HWND hWnd = CreateWindow(className, "", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

  if (hWnd)
    ShowWindow(hWnd, SW_SHOW);

  return hWnd;
#endif  // WINDOWS
}

bool CreateSurfaceContext(EGLNativeWindowType window, EGLDisplay display,
                          EGLConfig config, EGLContext *context,
                          EGLSurface *surface) {

  *surface = eglCreateWindowSurface(display, config, window, NULL);
  if (*surface == EGL_NO_SURFACE) {
    Error("Cannot create EGL window surface.\n");
    return false;
  }

  if (!eglBindAPI(EGL_OPENGL_ES_API)) {
    Error("Cannot bind OpenGL ES API.\n");
    return false;
  }

  const EGLint contextAttrib[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  *context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttrib);
  if (*context == EGL_NO_CONTEXT) {
    Error("Cannot create EGL context.\n");
    return false;
  }

  if (!eglMakeCurrent(display, *surface, *surface, *context)) {
    Error("Cannot make EGL surface current.\n");
    return false;
  }

  return true;
}

const char *ErrorString() {
  EGLint num_err = eglGetError();
  switch (num_err) {
  case EGL_SUCCESS:
    return "EGL_SUCCESS";
  case EGL_BAD_DISPLAY:
    return "EGL_BAD_DISPLAY";
  case EGL_NOT_INITIALIZED:
    return "EGL_NOT_INITIALIZED";
  case EGL_BAD_ACCESS:
    return "EGL_BAD_ACCESS";
  case EGL_BAD_ALLOC:
    return "EGL_BAD_ALLOC";
  case EGL_BAD_ATTRIBUTE:
    return "EGL_BAD_ATTRIBUTE";
  case EGL_BAD_CONFIG:
    return "EGL_BAD_CONFIG";
  case EGL_BAD_CONTEXT:
    return "EGL_BAD_CONTEXT";
  case EGL_BAD_CURRENT_SURFACE:
    return "EGL_BAD_CURRENT_SURFACE";
  case EGL_BAD_MATCH:
    return "EGL_BAD_MATCH";
  case EGL_BAD_NATIVE_PIXMAP:
    return "EGL_BAD_NATIVE_PIXMAP";
  case EGL_BAD_NATIVE_WINDOW:
    return "EGL_BAD_NATIVE_WINDOW";
  case EGL_BAD_PARAMETER:
    return "EGL_BAD_PARAMETER";
  case EGL_BAD_SURFACE:
    return "EGL_BAD_SURFACE";
  default:
    return "unknown";
  }

  return "EGL unused";
}

void PrintConfig(EGLDisplay display, EGLConfig config) {
  const int attr_count = 13;
  EGLint attr_id[attr_count] = { EGL_ALPHA_SIZE, EGL_BLUE_SIZE, EGL_RED_SIZE,
                                 EGL_GREEN_SIZE, EGL_DEPTH_SIZE,
                                 EGL_STENCIL_SIZE, EGL_BUFFER_SIZE,
                                 EGL_MAX_PBUFFER_WIDTH, EGL_MAX_PBUFFER_HEIGHT,
                                 EGL_MAX_PBUFFER_PIXELS, EGL_NATIVE_RENDERABLE,
                                 EGL_RENDERABLE_TYPE, EGL_SAMPLES };
  const char *attr_name[attr_count] = { "alpha size", "blue size", "red size",
                                        "green size", "depth size",
                                        "stencil size", "buffer size",
                                        "pbuffer width", "pbuffer height",
                                        "pbuffer pixels", "native renderable",
                                        "renderable type", "samples" };
  for (int i = 0; i < attr_count; ++i) {
    EGLint v;
    if (!eglGetConfigAttrib(display, config, attr_id[i], &v)) {
      Error("Cannot get EGL config attribute \"%s\".\n", attr_name[i]);
      return;
    }
    Print("%s = %d\n", attr_name[i], v);
  }
}

bool SwapBuffers(EGLDisplay dpy, EGLSurface srf) {
  if (!eglSwapBuffers(dpy, srf)) {
    Error("Cannot swap EGL buffer: %s\n", ErrorString());
    return false;
  }
  return true;
}

bool Terminate(EGLDisplay dpy) {
  if (dpy != EGL_NO_DISPLAY) {
    if (!eglTerminate(dpy)) {
      Error("Cannot terminate EGL session.\n");
      return false;
    }
  }
  return true;
}

} // namespace eglu
} // namespace ww

