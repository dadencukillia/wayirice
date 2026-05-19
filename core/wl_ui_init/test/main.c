#include "wl_ui_init.h"

int main(void) {
  wl_ui_application* app = create_app();
  app_init(app);
  app_init_egl(app);

  wl_ui_surface* surface = create_surface(app);
  surface_init(surface);
  surface_role_window(surface);
  surface_init_egl(surface);
  surface_show(surface);

  while (!surface_should_close(surface)) {
    app_wait_for_events(app, 1'000 / 60);

    if (surface_can_update(surface)) {
      surface_activate(surface);
      surface_swap_buffers(surface);
    }
  }

  destroy_surface(surface);
  destroy_app(app);

  return 0;
}
