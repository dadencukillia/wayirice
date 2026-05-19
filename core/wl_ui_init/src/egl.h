/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#ifndef WL_UI_INIT_EGL_H
#define WL_UI_INIT_EGL_H

#include "types.h"

WL_UI_RESULT app_init_egl(struct wl_ui_application* app);
WL_UI_RESULT surface_init_egl(struct wl_ui_surface* surface);
void surface_activate(struct wl_ui_surface* surface);
void surface_swap_buffers(struct wl_ui_surface* surface);

#endif
