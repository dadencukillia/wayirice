find_path(
  WAYLAND_CLIENT_INCLUDE_DIR
  NAMES wayland-client.h
  REQUIRED
)

find_library(
  WAYLAND_CLIENT_LIBRARY
  NAMES wayland-client libwayland-client
  REQUIRED
)

add_library(wl_ui_init 
  ./src/lib.c

  ./protocols/xdg-shell.c
)
target_include_directories(wl_ui_init PUBLIC ./include PRIVATE ./protocols)

target_include_directories(wl_ui_init PRIVATE ${WAYLAND_CLIENT_INCLUDE_DIR})
target_link_libraries(wl_ui_init ${WAYLAND_CLIENT_LIBRARY})

# Test demo

add_executable(wl_ui_init_test ./test/main.c)
target_link_libraries(wl_ui_init_test wl_ui_init)
