add_library(wl_ui_init 
  ./src/lib.c
)
target_include_directories(wl_ui_init PUBLIC ./include)

# Test demo

add_executable(wl_ui_init_test ./test/main.c)
target_link_libraries(wl_ui_init_test wl_ui_init)
