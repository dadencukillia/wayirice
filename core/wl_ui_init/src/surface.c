/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#include "lib.h"

#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>
#include <stdlib.h>
#include "wlr-layer-shell-unstable-v1.h"
#include "xdg-shell.h"

#include "types.h"
#include "macroses.h"
#include "listeners.h"

wl_ui_surface* create_surface(wl_ui_application* app) {
  wl_ui_surface* surface = malloc(sizeof(wl_ui_surface));

  surface->app = app;
  surface->should_close = false;
  surface->frame_ready = false;
  surface->role_window.set = false;
  surface->role_bar.set = false;
  surface->egl_states.init = false;
  surface->egl_states.inited = false;

  return surface;
}

WL_UI_RESULT surface_init(wl_ui_surface* surface) {
  surface->wl_surface = wl_compositor_create_surface(surface->app->global_objects.wl_compositor);
  if (!surface->wl_surface) {
    DEBUG_LOG("failed to create a surface");
    return WL_UI_ERR;
  }

  DEBUG_LOG("surface created");
  return WL_UI_OK;
}

WL_UI_RESULT surface_role_window(wl_ui_surface* surface) {
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

WL_UI_RESULT surface_role_bar(wl_ui_surface* surface) {
  surface->role_bar.set = true;

  surface->role_bar.zwlr_layer_surface_v1 = zwlr_layer_shell_v1_get_layer_surface(
    surface->app->global_objects.zwlr_layer_shell_v1, 
    surface->wl_surface,
    NULL,
    ZWLR_LAYER_SHELL_V1_LAYER_TOP,
    "wayirice_bar"
  );

  zwlr_layer_surface_v1_set_anchor(surface->role_bar.zwlr_layer_surface_v1,
    ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
    ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
    ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT
  );
  zwlr_layer_surface_v1_set_size(surface->role_bar.zwlr_layer_surface_v1, 0, 32);
  zwlr_layer_surface_v1_set_exclusive_zone(surface->role_bar.zwlr_layer_surface_v1, 32);

  zwlr_layer_surface_v1_add_listener(surface->role_bar.zwlr_layer_surface_v1, &listener_zwlr_layer_surface_v1, surface);

  DEBUG_LOG("role to bar");
  return WL_UI_OK;
}

void surface_apply(wl_ui_surface* surface) {
  wl_surface_commit(surface->wl_surface);
}

WL_UI_BOOL surface_should_close(wl_ui_surface* surface) {
  return surface->should_close;
}

WL_UI_BOOL surface_can_update(wl_ui_surface* surface) {
  if (surface->frame_ready) {
    surface->frame_ready = false;
    struct wl_callback* wl_callback = wl_surface_frame(surface->wl_surface);
    wl_callback_add_listener(wl_callback, &listener_wl_callback, surface);

    return true;
  }

  return false;
}

void destroy_surface(wl_ui_surface* surface) {
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

  if (surface->role_bar.set) {
    zwlr_layer_surface_v1_destroy(surface->role_bar.zwlr_layer_surface_v1);
  }

  wl_surface_destroy(surface->wl_surface);
}
