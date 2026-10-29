#pragma once

#include "base/base_inc.h"

typedef struct window_s window_t;
struct window_s {
    const char *title;
    i32 width;
    i32 height;
    f32 scale_factor;

    bool is_open;
    bool should_close;
    bool is_dirty;
    bool is_animating;
    bool needs_full_redraw;
    u32 full_redraw_buffers;

    void *hit_tests;
    void *gpu_draw_list;
};
