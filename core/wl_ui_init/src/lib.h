/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#ifndef WL_UI_INIT_LIB_H
#define WL_UI_INIT_LIB_H

enum WL_UI_RESULT {
  WL_UI_OK,
  WL_UI_ERR
};

typedef struct wl_ui_application wl_ui_application;
typedef struct wl_ui_surface wl_ui_surface;
typedef enum WL_UI_RESULT WL_UI_RESULT;
typedef unsigned char WL_UI_BOOL;

// egl.c

void surface_activate(wl_ui_surface* surface);
void surface_swap_buffers(wl_ui_surface* surface);

// app.c

wl_ui_application* create_app();
WL_UI_RESULT app_init(wl_ui_application* app);
void app_wait_for_events(wl_ui_application* app, int timeout_ms);
void destroy_app(wl_ui_application* app);

// surface.c

wl_ui_surface* create_surface(wl_ui_application* app);
WL_UI_RESULT surface_init(wl_ui_surface* surface);
WL_UI_RESULT surface_role_window(wl_ui_surface* surface);
WL_UI_RESULT surface_role_bar(wl_ui_surface* surface);
void surface_apply(wl_ui_surface* surface);
WL_UI_BOOL surface_can_update(wl_ui_surface* surface);
void destroy_surface(wl_ui_surface* surface);
WL_UI_BOOL surface_should_close(wl_ui_surface* surface);

#endif
