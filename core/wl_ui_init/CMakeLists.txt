find_package(PkgConfig REQUIRED)

pkg_check_modules(WAYLAND REQUIRED 
  wayland-client 
  wayland-egl 
  wayland-cursor
  egl
)

add_library(wl_ui_init 
  ./src/lib.c

  ./protocols/xdg-shell.c
)
target_include_directories(wl_ui_init PUBLIC ./include PRIVATE ./protocols)

target_include_directories(wl_ui_init PRIVATE ${WAYLAND_INCLUDE_DIRS})
target_link_libraries(wl_ui_init PRIVATE ${WAYLAND_LIBRARIES})

# Test demo

add_executable(wl_ui_init_test ./test/main.c)
target_link_libraries(wl_ui_init_test wl_ui_init)
