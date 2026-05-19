/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#ifndef WL_UI_INIT_TYPES_H
#define WL_UI_INIT_TYPES_H

#include "EGL/egl.h"

enum WL_UI_RESULT : int {
  WL_UI_OK,
  WL_UI_ERR
};


// app

struct wl_ui_global_objects {
  struct wl_compositor* wl_compositor;
  struct xdg_wm_base* xdg_wm_base;
  struct wl_seat* wl_seat;
  struct wl_shm* wl_shm;
};

struct wl_ui_app_egl_states {
  bool inited;

  EGLint major;
  EGLint minor;

  EGLDisplay display;
};

struct wl_ui_input_devices {
  struct wl_pointer* wl_pointer;
  struct wl_keyboard* wl_keyboard;

  struct wl_cursor_theme* cursor_theme;
  struct wl_cursor_image* cursor_image;
  struct wl_buffer* cursor_buffer;
  struct wl_surface* cursor_surface;
};

struct wl_ui_application {
  struct wl_display* wl_display;
  struct wl_ui_global_objects global_objects;
  struct wl_ui_app_egl_states egl_states;
  struct wl_ui_input_devices input_devices;
};

// surface

struct wl_ui_surface_egl_states {
  bool inited;

  EGLDisplay egl_display;
  EGLSurface egl_surface;
  EGLContext egl_context;
  struct wl_egl_window* wl_egl_window;
};

struct wl_ui_role_window {
  bool set;

  struct xdg_surface* xdg_surface;
  struct xdg_toplevel* xdg_toplevel;
};

struct wl_ui_surface {
  struct wl_ui_application* app;
  struct wl_surface* wl_surface;
  struct wl_ui_surface_egl_states egl_states;

  struct wl_ui_role_window role_window;

  bool should_close;
  bool frame_ready;
};

#endif
