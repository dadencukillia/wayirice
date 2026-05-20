/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#include "lib.h"

#include <EGL/egl.h>
#include <wayland-egl.h>

#include "macroses.h"
#include "types.h"

WL_UI_RESULT app_init_egl(wl_ui_application* app) {
  app->egl_states.display = eglGetDisplay((EGLNativeDisplayType) app->wl_display);
  if (!app->egl_states.display) {
    DEBUG_LOG("failed to get EGL display");
    return WL_UI_ERR;
  }

  if (!eglInitialize(app->egl_states.display, &app->egl_states.major, &app->egl_states.minor)) {
    DEBUG_LOG("failed to initialize EGL");
    return WL_UI_ERR;
  }

  if (!eglBindAPI(EGL_OPENGL_API)) {
    DEBUG_LOG("failed to bind OpenGL API");
    return WL_UI_ERR;
  }


  DEBUG_LOG("app egl inited");
  return WL_UI_OK;
}

WL_UI_RESULT surface_init_egl(wl_ui_surface *surface, int width, int height) {
  const EGLint attrib_list[] = {
    EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 24,
    EGL_NONE
  };

  const EGLint context_attribs[] = {
    EGL_CONTEXT_MAJOR_VERSION, 3,
    EGL_CONTEXT_MINOR_VERSION, 3,
    EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    EGL_NONE
  };

  EGLConfig egl_config;
  EGLint num_configs;

  surface->egl_states.egl_display = surface->app->egl_states.display;

  if (!eglChooseConfig(surface->egl_states.egl_display, attrib_list, &egl_config, 1, &num_configs)) {
    DEBUG_LOG("eglConfig creation error");
    return WL_UI_ERR;
  }

  if (num_configs == 0) {
    DEBUG_LOG("RGBA parameters are not supported");
    return WL_UI_ERR;
  }

  surface->egl_states.wl_egl_window = wl_egl_window_create(surface->wl_surface, width, height);
  surface->egl_states.egl_surface = eglCreateWindowSurface(surface->egl_states.egl_display, egl_config, (EGLNativeWindowType) surface->egl_states.wl_egl_window, NULL);
  surface->egl_states.egl_context = eglCreateContext(surface->egl_states.egl_display, egl_config, EGL_NO_CONTEXT, context_attribs);

  DEBUG_LOG("surface egl inited");
  return WL_UI_OK;
}

WL_UI_RESULT surface_configure_egl(wl_ui_surface *surface) {
  if (surface->egl_states.inited) {
    wl_egl_window_resize(surface->egl_states.wl_egl_window, surface->info.width, surface->info.height, 0, 0);
  } else {
    if (surface_init_egl(surface, surface->info.width, surface->info.height) == WL_UI_ERR) {
      return WL_UI_ERR;
    }
    surface->egl_states.inited = true;
  }

  return WL_UI_OK;
}

void surface_activate(wl_ui_surface *surface) {
  eglMakeCurrent(surface->egl_states.egl_display, surface->egl_states.egl_surface, surface->egl_states.egl_surface, surface->egl_states.egl_context);
}

void surface_swap_buffers(wl_ui_surface* surface) {
  eglSwapBuffers(surface->egl_states.egl_display, surface->egl_states.egl_surface);
}
