/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#ifndef WL_UI_INIT_WAYLAND_H
#define WL_UI_INIT_WAYLAND_H

#include "types.h"

struct wl_ui_application* create_app();
WL_UI_RESULT app_init(struct wl_ui_application* app);
void app_dispatch_events(struct wl_ui_application* app);

struct wl_ui_surface* create_surface(struct wl_ui_application* app);
WL_UI_RESULT surface_init(struct wl_ui_surface* surface);
WL_UI_RESULT surface_role_window(struct wl_ui_surface* surface);
void surface_show(struct wl_ui_surface* surface);
bool surface_should_close(struct wl_ui_surface* surface);
bool surface_can_update(struct wl_ui_surface* surface);

void destroy_app(struct wl_ui_application* app);
void destroy_surface(struct wl_ui_surface* surface);

#endif
