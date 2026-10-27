#pragma once

#include "base/base_inc.h"
#include "draw/draw_core.h"
#include "ui/ui_app.h"
#include "ui/ui_slotmap.h"

#include "window_core.h"
#include "window_event.h"

typedef struct window_params_s window_params_t;
struct window_params_s {
    const char *title;
    int width;
    int height;
};

window_t *window_create(ui_app_t *app, window_params_t params);
void window_destroy(window_t *window);
bool window_is_open(const window_t *window);
void window_set_animating(window_t *window, bool animating);
ui_event_t wait_for_event(void);
bool poll_event(ui_event_t *event);
void dispatch_event(ui_app_t *app, window_t *window, const ui_event_t *event);

framebuffer_t window_framebuffer(window_t *window);
void window_damage(window_t *window, i32 x, i32 y, i32 w, i32 h);
void window_present(window_t *window);

#if LINUX
#include "linux/wayland/linux_wayland.h" // IWYU pragma: keep
#endif
