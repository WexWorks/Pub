// Copyright (c) 2011 by WexWorks, LLC -- All Rights Reserved

#include "util/app.h"
#include <stdio.h>

using namespace ww;

App::App()
  : done_(false)
#if USE_EGL
    , native_display_(0), native_window_(0)
#endif
{

}

App::~App() {

}

#ifdef WINDOWS
#include <WindowsX.h>

void Win32Command(HWND hWnd, int id, HWND hWndctl, UINT codenotify) {
  FORWARD_WM_COMMAND(hWnd, id, hWndctl, codenotify, DefWindowProc);
}

void Win32Destroy(HWND hWnd) {
  hWnd;
  PostQuitMessage(0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam) {
  PAINTSTRUCT ps;
  HDC hdc;
  const int HIGH_BIT = 0x8000;  // high bit mask for a short

  // Retrieve the Window for this thread
  App *app = (App *)GetWindowLong(hWnd, GWL_USERDATA);

  switch (message) {
  case WM_CREATE:
    return 0;

  case WM_CLOSE:
    if (app)
      app->Quit();
    break;

  case WM_COMMAND:
    return HANDLE_WM_COMMAND(hWnd, wparam, lparam, Win32Command);

  case WM_DESTROY:
    return HANDLE_WM_DESTROY(hWnd, wparam, lparam, Win32Destroy);

  case WM_QUIT:
    if (app)
      app->Quit();
    break;
#if 0
  case WM_KEYDOWN:
    event.type = Event::KeyDown;
    // High-order bit indicates key is pressed, low-order is toggle
    event.shift = bool(GetKeyState(VK_SHIFT) & HIGH_BIT);
    event.control = bool(GetKeyState(VK_CONTROL) & HIGH_BIT);
    event.alt = bool(GetKeyState(VK_MENU) & HIGH_BIT);

    switch (wparam) {
    case 0x0A:      // linefeed
    case VK_INSERT:
    case VK_DELETE:
    case VK_F2:
      break;
    case 0x08:      event.key = Event::BackspaceKey;    break;
    case 0x1B:      event.key = Event::EscapeKey;       break;
    case 0x0D:      event.key = Event::EnterKey;        break;
    case VK_LEFT:   event.key = Event::LeftArrowKey;    break;
    case VK_RIGHT:  event.key = Event::RightArrowKey;   break;
    case VK_DOWN:   event.key = Event::DownArrowKey;    break;
    case VK_UP:     event.key = Event::UpArrowKey;      break;
    case VK_END:
    case VK_HOME:   event.key = Event::HomeKey;         break;
    case VK_PRIOR:  event.key = Event::PageUpKey;       break;
    case VK_NEXT:   event.key = Event::PageDownKey;     break;
    case 0xBB:      event.key = '=';                    break;
    case 0xBD:      event.key = '-';                    break;
    default:
      event.key = char(tolower((TCHAR)wparam));
    }

    break;
#endif
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_MOUSEWHEEL: {
    float xy[2] = { LOWORD(lparam), HIWORD(lparam)};
    if (app)
      app->Touch(1, xy);
#if 0
    bool shift = bool(wparam & MK_SHIFT);
    bool control = bool(wparam & MK_CONTROL);
    switch (message) {
    case WM_LBUTTONDOWN : event.button = Event::LeftMouse; break;
    case WM_MBUTTONDOWN : event.button = Event::MiddleMouse; break;
    case WM_RBUTTONDOWN : event.button = Event::RightMouse; break;
    case WM_MOUSEWHEEL:
      event.key = char(short(HIWORD(wparam)) / WHEEL_DELTA);
      event.button = event.key > 0 ? 
        Event::WheelUpMouse : Event::WheelDownMouse;
      break;
    }
#endif
    break;
                      }
  case WM_SETFOCUS:
    SetFocus(hWnd);
    return 0;

#if 0
  case WM_MOUSEMOVE:
    event.type = Event::MouseDrag;
    event.x = LOWORD(lparam);
    event.y = HIWORD(lparam);
    event.shift = bool(wparam & MK_SHIFT);
    event.control = bool(wparam & MK_CONTROL);
    if (wparam & MK_LBUTTON)
      event.button = Event::LeftMouse;
    else if (wparam & MK_MBUTTON)
      event.button = Event::MiddleMouse;
    else if (wparam & MK_RBUTTON)
      event.button = Event::RightMouse;
    else
      event.button = Event::NoMouse;
    break;

  case WM_SIZE:
    event.type = Event::WinResize;
    event.x = LOWORD(lparam);
    event.y = HIWORD(lparam);
    break;;

  case WM_MOVE:
    event.type = Event::WinMove;
    event.x = LOWORD(lparam);
    event.y = HIWORD(lparam);
    break;
#endif

  case WM_PAINT:
    hdc = BeginPaint(hWnd, &ps);
    if (app)
      app->Repaint();
    EndPaint(hWnd, &ps);
    break;

  case WM_SETCURSOR:
//    SetCursor(g_arrowCursor);
    return 0;

  default:
    return DefWindowProc(hWnd, message, wparam, lparam);
    break;
  }

  return 0;
}
#endif  // WINDOWS

bool App::Init(int width, int height) {
#ifdef WINDOWS
  native_display_ = GetDC(NULL);
  WNDCLASSEX wcex;
  const char *className = "Win32WindowClass"; // must match eglu.cpp
  HINSTANCE hInstance = GetModuleHandle(NULL);
  if (!GetClassInfoEx(hInstance, className, &wcex)) {
    wcex.cbSize         = sizeof(WNDCLASSEX); 
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = className;
    wcex.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);
    if (!RegisterClassEx(&wcex))
      return false;
  }
#endif
#ifdef LINUX
  if (!native_display_)
    native_display_ = XOpenDisplay(NULL);
#endif
  return true;
}

bool App::Run() {
#ifdef WINDOWS
  MSG msg = {0};
  while (!done_) {
    if (GetMessage(&msg, native_window_, 0, 0) == -1)
      return false;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
#endif

#ifdef LINUX
  // Blocks the insta-kill of the close window icon, which causes reboot!
  // Should deliver a ClientMessage, but does not seem to...
  // Atom wm_protocols = XInternAtom(native_display_, "WM_PROTOCOLS", false);
  Atom wm_delete = XInternAtom(native_display_, "WM_DELETE_WINDOW", false);
  if (!XSetWMProtocols(native_display_, native_window_, &wm_delete, 1))
    return false;

  XSelectInput(native_display_, native_window_,
               KeyPressMask | ExposureMask | EnterWindowMask | LeaveWindowMask |
               PointerMotionMask | VisibilityChangeMask | ButtonPressMask |
               ButtonReleaseMask | StructureNotifyMask);

  while (!done_) {
    while (XPending(native_display_) > 0) {
      XEvent event;
      XNextEvent(native_display_, &event);

      switch (event.type) {
      case Expose:
        if (event.xexpose.window == native_window_)
          Repaint();
        break;
      case ButtonPress: {
        float xy[2] = { event.xbutton.x, event.xbutton.y};
        Touch(1, xy);
        break;
      }
      case ButtonRelease:
        break;
      case KeyPress: {
        char key;
        KeySym keysym;
        XLookupString(&event.xkey, &key, 1, &keysym, NULL);
        switch (keysym) {
          case 'r': {
            XWindowAttributes attr;
            XGetWindowAttributes(native_display_, native_window_, &attr);
            XResizeWindow(native_display_, native_window_, attr.height, attr.width);
            break;
          }
          case XK_Escape:
            Quit();
            break;
        }
        break;
      }
      case KeyRelease:
        break;
      case DestroyNotify:
        Quit();
        break;
      case ClientMessage:
        if ((Atom)event.xclient.data.l[0] == wm_delete)
          Quit();
        break;
      case UnmapNotify:
        Quit();
        break;
      case MapNotify:
        break;
      case MapRequest:
        break;
      case ConfigureNotify:
        Init(event.xconfigure.width, event.xconfigure.height);
        break;
      default:
        break;
      }
    }
    Repaint();
  }
#endif // LINUX

  return true;
}
