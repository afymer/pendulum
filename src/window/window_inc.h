#pragma once

#include "base/base_arena.h"
#include "draw/draw_core.h"

typedef struct window_params_s window_params_t;

struct window_params_s {
    const char *title;
    int width;
    int height;
};

typedef struct window_s window_t;

struct window_s {
    const char *title;
    int width;
    int height;
    bool should_close;
    bool needs_redraw;
};

window_t *window_create(arena_t *arena, window_params_t params);
void window_destroy(window_t *window);
void window_poll_events(window_t *window); // non-blocking
void window_wait_events(window_t *window); // blocks until at least one event is handled

framebuffer_t window_framebuffer(window_t *window);
void window_present(window_t *window);

#if LINUX
#include "linux/wayland/linux_wayland.h" // IWYU pragma: keep
#endif
