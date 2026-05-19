/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#ifndef WL_UI_INIT_LIB_H
#define WL_UI_INIT_LIB_H

enum WL_UI_RESULT : int {
  WL_UI_OK,
  WL_UI_ERR
};

typedef struct wl_ui_application wl_ui_application;
typedef struct wl_ui_surface wl_ui_surface;
typedef enum WL_UI_RESULT WL_UI_RESULT;

// egl.c

WL_UI_RESULT app_init_egl(struct wl_ui_application* app);
WL_UI_RESULT surface_init_egl(struct wl_ui_surface* surface);
void surface_activate(struct wl_ui_surface* surface);
void surface_swap_buffers(struct wl_ui_surface* surface);

// app.c

wl_ui_application* create_app();
WL_UI_RESULT app_init(struct wl_ui_application* app);
void app_dispatch_events(struct wl_ui_application* app);
void destroy_app(struct wl_ui_application* app);

// surface.c

wl_ui_surface* create_surface(wl_ui_application* app);
WL_UI_RESULT surface_init(wl_ui_surface* surface);
WL_UI_RESULT surface_role_window(wl_ui_surface* surface);
void surface_show(wl_ui_surface* surface);
bool surface_can_update(wl_ui_surface* surface);
void destroy_surface(wl_ui_surface* surface);
bool surface_should_close(wl_ui_surface* surface);

#endif
