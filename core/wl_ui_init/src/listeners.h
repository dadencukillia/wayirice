/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#ifndef WL_UI_INIT_LISTENERS_H
#define WL_UI_INIT_LISTENERS_H

#include "wlr-layer-shell-unstable-v1.h"
#include "xdg-shell.h"
#include <wayland-client.h>

extern const struct wl_registry_listener listener_wl_registry;
extern const struct wl_seat_listener listener_wl_seat;
extern const struct wl_pointer_listener listener_wl_pointer;
extern const struct xdg_wm_base_listener listener_xdg_wm_base;
extern const struct xdg_surface_listener listener_xdg_surface;
extern const struct xdg_toplevel_listener listener_xdg_toplevel;
extern const struct wl_callback_listener listener_wl_callback;
extern const struct zwlr_layer_surface_v1_listener listener_zwlr_layer_surface_v1;

#endif
