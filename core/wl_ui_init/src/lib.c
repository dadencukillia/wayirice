/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#include <EGL/eglplatform.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>
#include <EGL/egl.h>
#include "xdg-shell.h"

#if defined(NDEBUG) || defined(NDEBUG_WL_UI_INIT)
#define DEBUG_LOG(message)
#else
#define DEBUG_LOG(message) printf("[WL_UI_INIT] %s\n", message)
#endif

struct app_state {
  struct wl_display* wl_display;

  struct wl_compositor* wl_compositor;
  struct xdg_wm_base* xdg_wm_base;
  struct wl_seat* wl_seat;
  struct wl_shm* wl_shm;

  struct wl_pointer* wl_pointer;
  struct wl_keyboard* wl_keyboard;

  struct wl_cursor_theme* cursor_theme;
  struct wl_cursor_image* cursor_image;
  struct wl_buffer* cursor_buffer;
  struct wl_surface* cursor_surface;

  bool should_close;
};

static void empty_function() {}

#define DONT_HANDLE (void*) empty_function

static void handle_registry_listener_global(
  void* data,
  struct wl_registry* registry,
  uint32_t name,
  const char* interface,
  uint32_t version
) {
  struct app_state* state = data;

  if (strcmp(interface, "wl_compositor") == 0) {
    uint32_t supported_version = 4;
    state->wl_compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version < supported_version ? version : supported_version);
    DEBUG_LOG("wl_compositor binded");
  } else if (strcmp(interface, "xdg_wm_base") == 0) {
    uint32_t supported_version = 1;
    state->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, version < supported_version ? version : supported_version);
    DEBUG_LOG("xdg_wm_base binded");
  } else if (strcmp(interface, "wl_seat") == 0) {
    uint32_t supported_version = 8;
    state->wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, version < supported_version ? version : supported_version);
  } else if (strcmp(interface, "wl_shm") == 0) {
    uint32_t supported_version = 1;
    state->wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, version < supported_version ? version : supported_version);
  }
}

static void handle_xdg_wm_base_listener_ping(
  void* data,
  struct xdg_wm_base* xdg_wm_base,
  uint32_t serial
) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

static void handle_xdg_surface_configure(
  void* data,
  struct xdg_surface* xdg_surface,
  uint32_t serial
) {
  xdg_surface_ack_configure(xdg_surface, serial);
}

static void handle_xdg_toplevel_close(
  void* data,
  struct xdg_toplevel* xdg_toplevel
) {
  struct app_state* state = data;

  state->should_close = true;
}

static void handle_xdg_toplevel_configure(
  void* data,
  struct xdg_toplevel* xdg_toplevel,
  int32_t,
  int32_t,
  struct wl_array *
) {
}

static void handle_pointer_enter(
  void* data,
  struct wl_pointer* wl_pointer,
  uint32_t serial,
  struct wl_surface* surface,
  wl_fixed_t, wl_fixed_t
) {
  struct app_state* state = data;

  wl_pointer_set_cursor(wl_pointer, serial, state->cursor_surface, state->cursor_image->hotspot_x, state->cursor_image->hotspot_y);

  wl_surface_attach(state->cursor_surface, state->cursor_buffer, 0, 0);
  wl_surface_damage(state->cursor_surface, 0, 0, state->cursor_image->width, state->cursor_image->height);
  wl_surface_commit(state->cursor_surface);
}

static void handle_pointer_frame(
  void* data,
  struct wl_pointer* wl_pointer
) {
}

static void handle_seat_capabilities(
  void* data,
  struct wl_seat* wl_seat,
  uint32_t capability
) {
  struct app_state* state = data;

  if (capability & WL_SEAT_CAPABILITY_POINTER) {
    state->wl_pointer = wl_seat_get_pointer(state->wl_seat);
  }

  if (capability & WL_SEAT_CAPABILITY_KEYBOARD) {
    state->wl_keyboard = wl_seat_get_keyboard(state->wl_seat);
  }
}

void make_window(void) {
  struct app_state state = { 0 };

  state.wl_display = wl_display_connect(NULL);
  DEBUG_LOG("display connected");

  // Registering globals
  {
    struct wl_registry* registry = wl_display_get_registry(state.wl_display);
    const struct wl_registry_listener listener = {
      .global = handle_registry_listener_global
    };
    wl_registry_add_listener(registry, &listener, &state);
    wl_display_roundtrip(state.wl_display);
    wl_registry_destroy(registry);
  }

  // Creating surface
  struct wl_surface* surface = wl_compositor_create_surface(state.wl_compositor);
  DEBUG_LOG("surface created");

  // WM Base ping-pong
  struct xdg_wm_base_listener wm_base_listener = {
    .ping = handle_xdg_wm_base_listener_ping
  };
  xdg_wm_base_add_listener(state.xdg_wm_base, &wm_base_listener, &state);

  // Attaching a role to the surface
  struct xdg_surface* xdg_surface = xdg_wm_base_get_xdg_surface(state.xdg_wm_base, surface);
  xdg_surface_set_window_geometry(xdg_surface, 0, 0, 400, 300);
  struct xdg_surface_listener xdg_surface_listener = {
    .configure = handle_xdg_surface_configure
  };
  xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, &state);

  struct xdg_toplevel* xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
  struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = handle_xdg_toplevel_configure,
    .close = handle_xdg_toplevel_close
  };
  xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, &state);

  // WL Seat
  struct wl_seat_listener wl_seat_listener = {
    .capabilities = handle_seat_capabilities,
    .name = DONT_HANDLE
  };
  wl_seat_add_listener(state.wl_seat, &wl_seat_listener, &state);

  wl_display_roundtrip(state.wl_display);

  struct wl_pointer_listener wl_pointer_listener = {
    .enter = handle_pointer_enter,
    .leave = DONT_HANDLE,
    .axis_relative_direction = DONT_HANDLE,
    .motion = DONT_HANDLE,
    .axis = DONT_HANDLE,
    .axis_discrete = DONT_HANDLE,
    .axis_source = DONT_HANDLE,
    .axis_stop = DONT_HANDLE,
    .axis_value120 = DONT_HANDLE,
    .button = DONT_HANDLE,
    .frame = handle_pointer_frame
  };

  if (state.wl_pointer) {
    state.cursor_theme = wl_cursor_theme_load(NULL, 24, state.wl_shm);
    struct wl_cursor* default_cursor = wl_cursor_theme_get_cursor(state.cursor_theme, "left_ptr");
    state.cursor_image = default_cursor->images[0];
    state.cursor_buffer = wl_cursor_image_get_buffer(state.cursor_image);
    state.cursor_surface = wl_compositor_create_surface(state.wl_compositor);
    wl_pointer_add_listener(state.wl_pointer, &wl_pointer_listener, &state);
  }

  // EGL
  EGLDisplay egl_display = eglGetDisplay((EGLNativeDisplayType) state.wl_display);

  EGLint major, minor;
  if (!eglInitialize(egl_display, &major, &minor)) {
    DEBUG_LOG("failed to initialize EGL");
    return;
  }

  if (!eglBindAPI(EGL_OPENGL_API)) {
    DEBUG_LOG("failed to bind OpenGL API");
    return;
  }

  const EGLint attrib_list[] = {
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_NONE
  };

  const EGLint context_attribs[] = {
    EGL_CONTEXT_MAJOR_VERSION, 3,
    EGL_CONTEXT_MINOR_VERSION, 3,
    EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    EGL_NONE
  };

  EGLConfig egl_config;
  EGLint num_configs;

  if (!eglChooseConfig(egl_display, attrib_list, &egl_config, 1, &num_configs)) {
    DEBUG_LOG("eglConfig creation error");
    return;
  }

  if (num_configs == 0) {
    DEBUG_LOG("RGBA parameters are not supported");
    return;
  }

  struct wl_egl_window* wl_egl_window = wl_egl_window_create(surface, 400, 300);
  EGLSurface egl_surface = eglCreateWindowSurface(egl_display, egl_config, (EGLNativeWindowType) wl_egl_window, NULL);
  EGLContext egl_context = eglCreateContext(egl_display, egl_config, EGL_NO_CONTEXT, context_attribs);

  eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
  eglSwapBuffers(egl_display, egl_surface);

  wl_surface_commit(surface);

  // Surface loop
  while (!state.should_close) {
    if (wl_display_dispatch(state.wl_display) == -1) {
      break;
    }
  }

  // Destructors
  DEBUG_LOG("destroying...");
  eglDestroyContext(egl_display, egl_context);
  eglDestroySurface(egl_display, egl_surface);
  wl_egl_window_destroy(wl_egl_window);

  if (state.wl_pointer) {
    wl_cursor_theme_destroy(state.cursor_theme);
    wl_surface_destroy(state.cursor_surface);
    wl_pointer_destroy(state.wl_pointer);
  }

  if (state.wl_keyboard) {
    wl_keyboard_destroy(state.wl_keyboard);
  }

  xdg_toplevel_destroy(xdg_toplevel);
  xdg_surface_destroy(xdg_surface);
  wl_surface_destroy(surface);

  xdg_wm_base_destroy(state.xdg_wm_base);
  wl_seat_destroy(state.wl_seat);
  wl_shm_destroy(state.wl_shm);
  wl_compositor_destroy(state.wl_compositor);

  wl_display_disconnect(state.wl_display);
}
