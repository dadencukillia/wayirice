/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#include "lib.h"

#include <sys/poll.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>
#include <stdlib.h>
#include <poll.h>
#include "xdg-shell.h"

#include "types.h"
#include "macroses.h"
#include "listeners.h"

wl_ui_application* create_app() {
  wl_ui_application* app = malloc(sizeof(wl_ui_application));

  app->global_objects.wl_compositor = NULL;
  app->global_objects.xdg_wm_base = NULL;
  app->global_objects.wl_seat = NULL;
  app->global_objects.wl_shm = NULL;

  app->input_devices.wl_pointer = NULL;
  app->input_devices.wl_keyboard = NULL;

  app->egl_states.inited = false;

  return app;
}

WL_UI_RESULT app_init(wl_ui_application* app) {
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

void app_dispatch_events(wl_ui_application* app) {
  struct wl_display* wl_display = app->wl_display;

  wl_display_flush(wl_display);

  if (wl_display_prepare_read(wl_display) < 0) {
    wl_display_dispatch_pending(wl_display);
    return;
  }

  int fd_descriptor = wl_display_get_fd(wl_display);

  struct pollfd fd = { fd_descriptor, POLLIN, 0 };
  int ready_sockets_amount = poll(&fd, 1, 0);

  if (ready_sockets_amount > 0 && (fd.revents & POLLIN)) {
    wl_display_read_events(wl_display);
    wl_display_dispatch_pending(wl_display);
    return;
  }

  wl_display_cancel_read(wl_display);
}

void destroy_app(wl_ui_application* app) {
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
