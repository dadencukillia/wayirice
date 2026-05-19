/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#ifndef WL_UI_INIT_APP_H
#define WL_UI_INIT_APP_H

#include "types.h"

struct wl_ui_application* create_app();
enum WL_UI_RESULT app_init(struct wl_ui_application* app);
void app_dispatch_events(struct wl_ui_application* app);
void destroy_app(struct wl_ui_application* app);

#endif
