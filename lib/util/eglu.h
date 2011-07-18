// Copyright (c) 2011 by WexWorks, LLC -- All Rights Reserved

#ifndef EGLU_H_
#define EGLU_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace ww {
namespace eglu {

EGLNativeDisplayType GetNativeDisplay();
bool InitDisplay(EGLNativeDisplayType native_display, EGLDisplay *display,
                 EGLConfig *config);
EGLNativeWindowType CreateNativeWindow(const char *title, int width,
                                       int height, EGLDisplay display,
                                       EGLConfig config);
bool CreateSurfaceContext(EGLNativeWindowType window, EGLDisplay display,
                          EGLConfig config, EGLContext *context,
                          EGLSurface *surface);
const char *ErrorString();
void PrintConfig(EGLDisplay display, EGLConfig config);
bool SwapBuffers(EGLDisplay display, EGLSurface surface);
bool Terminate(EGLDisplay display);

} // namespace eglu
} // namespace ww

#endif  // EGLU_H_
