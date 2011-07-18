#include "util/debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef ANDROID
#include <android/log.h>
#define LOG_TAG "nvidia"
#endif

#ifdef WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#define VC_EXTRALEAN 1
#define NOMINMAX
#include "windows.h"
#endif
#endif	// WINDOWS

namespace ww {


void
Print(const char *format, ...)
{
    static const int len = 4096;
    char buf[len];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, len, format, args);
    va_end (args);
#if defined ANDROID
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", buf);
#elif defined WINDOWS
    OutputDebugString(buf);
#else
    printf(buf);
#endif
}



void
Warning(const char *format, ...)
{
    static const int len = 4096;
    char buf[len];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, len, format, args);
    va_end (args);
    char msg[len];
    sprintf(msg, "Warning: %s", buf);
#if defined ANDROID
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", msg);
#elif defined WINDOWS
    OutputDebugString(msg);
#else
    printf(msg);
#endif
}



void
Error(const char *format, ...)
{
    static const int len = 4096;
    char buf[len];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, len, format, args);
    va_end (args);
    char msg[len];
    sprintf(msg, "Error: %s", buf);
#if defined ANDROID
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", buf);
#elif defined WINDOWS
    OutputDebugString(msg);
#else
    printf(msg);
#endif
}



void
Abort(const char *format, ...)
{
    static const int len = 4096;
    char buf[len];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, len, format, args);
    va_end (args);
    char msg[len];
    sprintf(msg, "Abort: %s", buf);
#if defined ANDROID
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "%s", buf);
#elif defined WINDOWS
    OutputDebugString(msg);
#else
    printf(msg);
#endif
    abort();
}


} // namespace ww
