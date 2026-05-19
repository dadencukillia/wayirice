/*
 * SPDX-FileCopyrightText: 2026 Illia Diadenchuk
 * SPDX-License-Identifier: Zlib license
 */

#ifndef WL_UI_INIT_MACROSES_H
#define WL_UI_INIT_MACROSES_H

#include <stdio.h>

#if defined(NDEBUG) || defined(NDEBUG_WL_UI_INIT)
#define DEBUG_LOG(message)
#else
#define DEBUG_LOG(message) printf("[WL_UI_INIT] %s\n", message)
#endif

void empty_function();

#define DONT_HANDLE (void*) empty_function

#endif
