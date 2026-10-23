#pragma once

#include "base/base_inc.h"
#include "draw/draw_core.h"
#include "ui/ui_context.h"
#include "ui/ui_slotmap.h"

#include "window_event.h"

typedef struct window_s window_t;

struct window_s {
    const char *title;
    i32 width;
    i32 height;
    f32 scale_factor;

    ui_handle_t root_view;

    bool is_open;
    bool should_close;
    bool is_dirty;
};

typedef struct window_params_s window_params_t;
struct window_params_s {
    const char *title;
    int width;
    int height;
};

window_t *window_create(ui_context_t *cx, window_params_t params);
void window_destroy(window_t *window);
bool window_is_open(const window_t *window);
ui_event_t wait_for_event(void);
void dispatch_event(ui_context_t *cx, window_t *window, const ui_event_t *event);

// void window_poll_events(window_t *window);
// void window_wait_events(window_t *window);

framebuffer_t window_framebuffer(window_t *window);
void window_present(window_t *window);

#if LINUX
#include "linux/wayland/linux_wayland.h" // IWYU pragma: keep
#endif
