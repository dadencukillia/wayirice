/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#include "types.h"
#include "app.h"
#include "surface.h"
#include "egl.h"

void make_window(void) {
  struct wl_ui_application* app = create_app();
  app_init(app);
  app_init_egl(app);

  struct wl_ui_surface* surface = create_surface(app);
  surface_init(surface);
  surface_init_egl(surface);
  surface_role_window(surface);

  surface_show(surface);

  while (!surface_should_close(surface)) {
    app_dispatch_events(app);

    if (surface_can_update(surface)) {
      surface_activate(surface);
      surface_swap_buffers(surface);
    }
  }

  destroy_surface(surface);
  destroy_app(app);
}
