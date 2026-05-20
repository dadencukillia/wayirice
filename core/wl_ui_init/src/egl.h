/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#ifndef WL_UI_INIT_EGL_H
#define WL_UI_INIT_EGL_H

#include "lib.h"

WL_UI_RESULT app_init_egl(wl_ui_application* app);
WL_UI_RESULT surface_init_egl(wl_ui_surface *surface, int width, int height);
WL_UI_RESULT surface_configure_egl(wl_ui_surface *surface);

#endif
