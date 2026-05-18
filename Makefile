GENERATOR=Ninja

.PHONY: configure_core_debug configure_core_release build_core_debug build_core_release run_core core_debug core_release
.DEFAULT_GOAL := test_wl_ui_init

export CC := clang
export CXX := clang++

configure_core_debug: core
	cmake -Score -B=build -G=${GENERATOR} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=${CXX} -DCMAKE_C_COMPILER=${CC}

configure_core_release: core
	cmake -Score -B=build -G=${GENERATOR} -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=${CXX} -DCMAKE_C_COMPILER=${CC}

build_core_debug: build | configure_core_debug
	cmake --build build -j 8 --config Debug

build_core_release: build | configure_core_release
	cmake --build build -j 8 --config Debug

run_core:
	./build/engine/wayirice

core_debug: configure_core_debug build_core_debug run_core
core_release: configure_core_release build_core_release run_core

test_wl_ui_init: configure_core_debug build_core_debug
	./build/wl_ui_init/wl_ui_init_test
