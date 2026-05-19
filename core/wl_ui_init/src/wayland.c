/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#include "wayland.h"

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <stdlib.h>
#include <wayland-egl.h>
#include "xdg-shell.h"

#include "types.h"
#include "macroses.h"
#include "listeners.h"

struct wl_ui_application* create_app() {
  struct wl_ui_application* app = malloc(sizeof(struct wl_ui_application));

  app->global_objects.wl_compositor = NULL;
  app->global_objects.xdg_wm_base = NULL;
  app->global_objects.wl_seat = NULL;
  app->global_objects.wl_shm = NULL;

  app->input_devices.wl_pointer = NULL;
  app->input_devices.wl_keyboard = NULL;

  app->egl_states.inited = false;

  return app;
}

WL_UI_RESULT app_init(struct wl_ui_application* app) {
  app->wl_display = wl_display_connect(NULL);
  if (!app->wl_display) {
    DEBUG_LOG("failed to connect a display");
    return WL_UI_ERR;
  }

  DEBUG_LOG("display connected");

  // Registering globals
  {
    struct wl_registry* registry = wl_display_get_registry(app->wl_display);
    wl_registry_add_listener(registry, &listener_wl_registry, app);
    wl_display_roundtrip(app->wl_display);
    wl_registry_destroy(registry);
  }

  if (
    app->global_objects.wl_compositor == NULL ||
    app->global_objects.xdg_wm_base == NULL ||
    app->global_objects.wl_seat == NULL ||
    app->global_objects.wl_shm == NULL
  ) {
    DEBUG_LOG("missing a global object");
    return WL_UI_ERR;
  }

  // Input devices
  wl_seat_add_listener(app->global_objects.wl_seat, &listener_wl_seat, app);
  wl_display_roundtrip(app->wl_display);

  if (app->input_devices.wl_pointer) {
    app->input_devices.cursor_theme = wl_cursor_theme_load(NULL, 24, app->global_objects.wl_shm);
    struct wl_cursor* default_cursor = wl_cursor_theme_get_cursor(app->input_devices.cursor_theme, "left_ptr");
    app->input_devices.cursor_image = default_cursor->images[0];
    app->input_devices.cursor_buffer = wl_cursor_image_get_buffer(app->input_devices.cursor_image);
    app->input_devices.cursor_surface = wl_compositor_create_surface(app->global_objects.wl_compositor);
    wl_pointer_add_listener(app->input_devices.wl_pointer, &listener_wl_pointer, app);
  }

  // Listeners
  xdg_wm_base_add_listener(app->global_objects.xdg_wm_base, &listener_xdg_wm_base, app);

  return WL_UI_OK;
}

void app_dispatch_events(struct wl_ui_application* app) {
  wl_display_dispatch(app->wl_display);
}

struct wl_ui_surface* create_surface(struct wl_ui_application* app) {
  struct wl_ui_surface* surface = malloc(sizeof(struct wl_ui_surface));

  surface->app = app;
  surface->should_close = false;
  surface->frame_ready = true;
  surface->role_window.set = false;
  surface->egl_states.inited = false;

  return surface;
}

WL_UI_RESULT surface_init(struct wl_ui_surface* surface) {
  surface->wl_surface = wl_compositor_create_surface(surface->app->global_objects.wl_compositor);
  if (!surface->wl_surface) {
    DEBUG_LOG("failed to create a surface");
    return WL_UI_ERR;
  }

  DEBUG_LOG("surface created");
  return WL_UI_OK;
}

WL_UI_RESULT surface_role_window(struct wl_ui_surface* surface) {
  surface->role_window.set = true;

  surface->role_window.xdg_surface = xdg_wm_base_get_xdg_surface(surface->app->global_objects.xdg_wm_base, surface->wl_surface);
  if (!surface->role_window.xdg_surface) {
    DEBUG_LOG("failed to create xdg_surface");
    return WL_UI_ERR;
  }

  xdg_surface_add_listener(surface->role_window.xdg_surface, &listener_xdg_surface, surface);

  surface->role_window.xdg_toplevel = xdg_surface_get_toplevel(surface->role_window.xdg_surface);
  if (!surface->role_window.xdg_toplevel) {
    DEBUG_LOG("failed to create xdg_toplevel");
    return WL_UI_ERR;
  }

  xdg_toplevel_add_listener(surface->role_window.xdg_toplevel, &listener_xdg_toplevel, surface);

  DEBUG_LOG("role to window");

  return WL_UI_OK;
}

void surface_show(struct wl_ui_surface* surface) {
  wl_surface_commit(surface->wl_surface);
}

bool surface_should_close(struct wl_ui_surface* surface) {
  return surface->should_close;
}

bool surface_can_update(struct wl_ui_surface* surface) {
  if (surface->frame_ready) {
    surface->frame_ready = false;
    struct wl_callback* wl_callback = wl_surface_frame(surface->wl_surface);
    wl_callback_add_listener(wl_callback, &listener_wl_callback, surface);

    return true;
  }

  return false;
}

void destroy_app(struct wl_ui_application* app) {
  DEBUG_LOG("destroying application...");

  if (app->input_devices.wl_pointer) {
    wl_cursor_theme_destroy(app->input_devices.cursor_theme);
    wl_surface_destroy(app->input_devices.cursor_surface);
    wl_pointer_destroy(app->input_devices.wl_pointer);
  }

  if (app->input_devices.wl_keyboard) {
    wl_keyboard_destroy(app->input_devices.wl_keyboard);
  }

  xdg_wm_base_destroy(app->global_objects.xdg_wm_base);
  wl_seat_destroy(app->global_objects.wl_seat);
  wl_shm_destroy(app->global_objects.wl_shm);
  wl_compositor_destroy(app->global_objects.wl_compositor);

  wl_display_disconnect(app->wl_display);
}

void destroy_surface(struct wl_ui_surface* surface) {
  DEBUG_LOG("destroying surface...");

  if (surface->egl_states.inited) {
    eglDestroyContext(surface->egl_states.egl_display, surface->egl_states.egl_context);
    eglDestroySurface(surface->egl_states.egl_display, surface->egl_states.egl_surface);
    wl_egl_window_destroy(surface->egl_states.wl_egl_window);
  }

  if (surface->role_window.set) {
    xdg_toplevel_destroy(surface->role_window.xdg_toplevel);
    xdg_surface_destroy(surface->role_window.xdg_surface);
  }

  wl_surface_destroy(surface->wl_surface);
}
