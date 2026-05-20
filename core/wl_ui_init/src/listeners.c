/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#include "listeners.h"

#include <string.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl-core.h>
#include "wlr-layer-shell-unstable-v1.h"
#include "xdg-shell.h"

#include "egl.h"
#include "types.h"
#include "macroses.h"

// wl_registry

static void handle_wl_registry_listener_global(
  void* data,
  struct wl_registry* registry,
  uint32_t name,
  const char* interface,
  uint32_t version
) {
  struct wl_ui_application* app = data;

  if (strcmp(interface, "wl_compositor") == 0) {
    uint32_t supported_version = 4;
    app->global_objects.wl_compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version < supported_version ? version : supported_version);
    DEBUG_LOG("wl_compositor binded");
  } else if (strcmp(interface, "xdg_wm_base") == 0) {
    uint32_t supported_version = 1;
    app->global_objects.xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, version < supported_version ? version : supported_version);
    DEBUG_LOG("xdg_wm_base binded");
  } else if (strcmp(interface, "wl_seat") == 0) {
    uint32_t supported_version = 8;
    app->global_objects.wl_seat = wl_registry_bind(registry, name, &wl_seat_interface, version < supported_version ? version : supported_version);
    DEBUG_LOG("wl_seat binded");
  } else if (strcmp(interface, "wl_shm") == 0) {
    uint32_t supported_version = 1;
    app->global_objects.wl_shm = wl_registry_bind(registry, name, &wl_shm_interface, version < supported_version ? version : supported_version);
    DEBUG_LOG("wl_shm binded");
  } else if (strcmp(interface, "zwlr_layer_shell_v1") == 0) {
    uint32_t supported_version = 3;
    app->global_objects.zwlr_layer_shell_v1 = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, version < supported_version ? version : supported_version);
    DEBUG_LOG("zwlr_layer_shell_v1 binded");
  }
}

const struct wl_registry_listener listener_wl_registry = {
  .global = handle_wl_registry_listener_global,
  .global_remove = DONT_HANDLE
};

// wl_seat

static void handle_wl_seat_capabilities(
  void* data,
  struct wl_seat* wl_seat,
  uint32_t capability
) {
  struct wl_ui_application* app = data;

  if (capability & WL_SEAT_CAPABILITY_POINTER) {
    app->input_devices.wl_pointer = wl_seat_get_pointer(app->global_objects.wl_seat);
  }

  if (capability & WL_SEAT_CAPABILITY_KEYBOARD) {
    app->input_devices.wl_keyboard = wl_seat_get_keyboard(app->global_objects.wl_seat);
  }
}

const struct wl_seat_listener listener_wl_seat = {
  .capabilities = handle_wl_seat_capabilities,
  .name = DONT_HANDLE
};

// wl_pointer

static void handle_wl_pointer_enter(
  void* data,
  struct wl_pointer* wl_pointer,
  uint32_t serial,
  struct wl_surface* surface,
  wl_fixed_t, wl_fixed_t
) {
  struct wl_ui_application* app = data;

  wl_pointer_set_cursor(wl_pointer, serial, app->input_devices.cursor_surface, app->input_devices.cursor_image->hotspot_x, app->input_devices.cursor_image->hotspot_y);

  wl_surface_attach(app->input_devices.cursor_surface, app->input_devices.cursor_buffer, 0, 0);
  wl_surface_damage(app->input_devices.cursor_surface, 0, 0, app->input_devices.cursor_image->width, app->input_devices.cursor_image->height);
  wl_surface_commit(app->input_devices.cursor_surface);
}

static void handle_wl_pointer_frame(
  void* data,
  struct wl_pointer* wl_pointer
) {
}

const struct wl_pointer_listener listener_wl_pointer = {
  .axis = DONT_HANDLE,
  .axis_discrete = DONT_HANDLE,
  .axis_relative_direction = DONT_HANDLE,
  .axis_source = DONT_HANDLE,
  .axis_stop = DONT_HANDLE,
  .axis_value120 = DONT_HANDLE,

  .button = DONT_HANDLE,
  .leave = DONT_HANDLE,
  .motion = DONT_HANDLE,

  .enter = handle_wl_pointer_enter,
  .frame = handle_wl_pointer_frame
};

// xdg_wm_base

static void handle_xdg_wm_base_listener_ping(
  void* data,
  struct xdg_wm_base* xdg_wm_base,
  uint32_t serial
) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

const struct xdg_wm_base_listener listener_xdg_wm_base = {
  .ping = handle_xdg_wm_base_listener_ping
};

// xdg_surface

static void handle_xdg_surface_configure(
  void* data,
  struct xdg_surface* xdg_surface,
  uint32_t serial
) {
  struct wl_ui_surface* surface = data;

  xdg_surface_ack_configure(xdg_surface, serial);

  surface->info = surface->buf_info;
  surface->frame_ready = true;

  if (surface->egl_states.init && !surface->egl_states.inited) {
    _m_surface_init_egl(surface, surface->info.width, surface->info.height);
    wl_surface_commit(surface->wl_surface);
  } else if (surface->egl_states.inited) {
    wl_egl_window_resize(surface->egl_states.wl_egl_window, surface->info.width, surface->info.height, 0, 0);
  }
}

const struct xdg_surface_listener listener_xdg_surface = {
  .configure = handle_xdg_surface_configure,
};

// xdg_toplevel

static void handle_xdg_toplevel_close(
  void* data,
  struct xdg_toplevel* xdg_toplevel
) {
  struct wl_ui_surface* surface = data;

  surface->should_close = true;
}

static void handle_xdg_toplevel_configure(
  void* data,
  struct xdg_toplevel* xdg_toplevel,
  int32_t width,
  int32_t height,
  struct wl_array*
) {
  struct wl_ui_surface* surface = data;

  surface->buf_info.width = width;
  surface->buf_info.height = height;
}

const struct xdg_toplevel_listener listener_xdg_toplevel = {
  .configure = handle_xdg_toplevel_configure,
  .close = handle_xdg_toplevel_close
};

// wl_callback

static void handle_wl_callback_done(
  void* data,
  struct wl_callback* wl_callback,
  uint32_t callback_data
) {
  struct wl_ui_surface* surface = data;

  surface->frame_ready = true;

  wl_callback_destroy(wl_callback);
}

const struct wl_callback_listener listener_wl_callback = {
  .done = handle_wl_callback_done
};

// zwlr_layer_surface_v1

static void handle_zwlr_layer_surface_v1_configure(
  void* data,
  struct zwlr_layer_surface_v1* zwlr_layer_surface_v1,
  uint32_t serial, uint32_t width, uint32_t height
) {
  struct wl_ui_surface* surface = data;

  zwlr_layer_surface_v1_ack_configure(zwlr_layer_surface_v1, serial);

  surface->buf_info.width = width;
  surface->buf_info.height = height;
  surface->info = surface->buf_info;
  surface->frame_ready = true;

  if (surface->egl_states.init && !surface->egl_states.inited) {
    _m_surface_init_egl(surface, surface->info.width, surface->info.height);
    wl_surface_commit(surface->wl_surface);
  } else if (surface->egl_states.inited) {
    wl_egl_window_resize(surface->egl_states.wl_egl_window, width, height, 0, 0);
  }
}

static void handle_zwlr_layer_surface_v1_closed(
  void* data,
  struct zwlr_layer_surface_v1* zwlr_layer_surface_v1
) {
  struct wl_ui_surface* surface = data;
  surface->should_close = true;
}

const struct zwlr_layer_surface_v1_listener listener_zwlr_layer_surface_v1 = {
  .configure = handle_zwlr_layer_surface_v1_configure,
  .closed = handle_zwlr_layer_surface_v1_closed
};

