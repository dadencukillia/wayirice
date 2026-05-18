/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#include <EGL/eglplatform.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <wayland-client.h>
#include <wayland-egl-backend.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
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

  bool should_close;
};

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
    DEBUG_LOG("wl_compositor registered");
  } else if (strcmp(interface, "xdg_wm_base") == 0) {
    uint32_t supported_version = 1;
    state->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, version < supported_version ? version : supported_version);
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

void make_window(void) {
  struct app_state state;
  state.should_close = false;

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
  wl_egl_window_destroy(wl_egl_window);
  xdg_toplevel_destroy(xdg_toplevel);
  xdg_surface_destroy(xdg_surface);
  wl_surface_destroy(surface);
  xdg_wm_base_destroy(state.xdg_wm_base);
  wl_compositor_destroy(state.wl_compositor);
  wl_display_disconnect(state.wl_display);
}
